// USB Mass Storage Device — exposes SD card to host via USB
// Targets CH32X035 USBFS peripheral (similar to STM32F103 USB)

#include "usb_msd.h"
#include "storage.h"
#include <ch32x035.h>
#include <Arduino.h>
#include <string.h>

// --- Packet buffer layout ---
// CH32X035 USBFS has a 1KB dedicated packet buffer in SRAM.
// Buffer descriptor table occupies first 64 bytes (8 endpoints × 8 bytes).
// Data buffers follow.

#define USB_PBUF_BASE  0x20000400
#define USB_BTABLE      ((volatile uint32_t *)USB_PBUF_BASE)
#define USB_PBUF_DATA   (USB_PBUF_BASE + 64)

// Buffer descriptor table offsets (16-bit half-words from pbuf base)
#define BTABLE_TX_ADDR(ep)  (USB_BTABLE[(ep) * 4 + 0])
#define BTABLE_TX_COUNT(ep) (USB_BTABLE[(ep) * 4 + 1])
#define BTABLE_RX_ADDR(ep)  (USB_BTABLE[(ep) * 4 + 2])
#define BTABLE_RX_COUNT(ep) (USB_BTABLE[(ep) * 4 + 3])

// Endpoint assignments
#define EP0     0   // Control
#define EP1_IN  1   // Bulk IN (device → host)
#define EP2_OUT 2   // Bulk OUT (host → device)

// Packet buffer offsets for each endpoint
#define EP0_TX_OFF  0
#define EP0_RX_OFF  64
#define EP1_TX_OFF  128
#define EP2_RX_OFF  192

// Max packet sizes
#define EP0_SIZE    64
#define EP1_SIZE    64  // Bulk IN
#define EP2_SIZE    64  // Bulk OUT

// --- USB standard request codes ---
#define REQ_GET_DESCRIPTOR      0x06
#define REQ_SET_ADDRESS         0x05
#define REQ_SET_CONFIGURATION   0x09
#define REQ_GET_CONFIGURATION   0x08
#define REQ_GET_STATUS          0x00
#define REQ_SET_FEATURE         0x03
#define REQ_CLEAR_FEATURE       0x01

#define DESC_DEVICE             0x01
#define DESC_CONFIG             0x02
#define DESC_STRING             0x03

// --- BOT (Bulk-Only Transport) ---
#define BOT_CBW_SIGNATURE   0x43425355  // "USBC"
#define BOT_CSW_SIGNATURE   0x53425355  // "USBS"
#define BOT_CBW_LEN         31
#define BOT_CSW_LEN         13
#define BOT_DIR_IN          0x80
#define BOT_DIR_OUT         0x00

// --- SCSI commands ---
#define SCSI_TEST_UNIT_READY    0x00
#define SCSI_REQUEST_SENSE      0x03
#define SCSI_INQUIRY            0x12
#define SCSI_MODE_SENSE_6       0x1A
#define SCSI_START_STOP_UNIT    0x1B
#define SCSI_PREVENT_ALLOW      0x1E
#define SCSI_READ_CAPACITY_10   0x25
#define SCSI_READ_10            0x28
#define SCSI_WRITE_10           0x2A

// --- Global state ---
static uint8_t  usbAddr       = 0;
static uint8_t  usbConfigured = 0;
static bool     hostPresent   = false;

// MSD state
static uint8_t  msdBuf[512];        // shared sector buffer
static uint32_t msdBlockCount  = 0;
static uint16_t msdBlockSize   = 512;

// CBW/CSW
static struct {
    uint32_t dCBWSignature;
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t  bmCBWFlags;
    uint8_t  bCBWLUN;
    uint8_t  bCBWCBLength;
    uint8_t  CBWCB[16];
} cbw;

static struct {
    uint32_t dCSWSignature;
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t  bCSWStatus;
} csw;

// Request sense data
static uint8_t senseKey = 0;
static uint8_t senseASC = 0;

// --- USB Descriptors ---

static const PROGMEM uint8_t devDesc[] = {
    0x12,       // bLength
    0x01,       // bDescriptorType (Device)
    0x00, 0x02, // bcdUSB 2.00
    0x00,       // bDeviceClass (use interface)
    0x00,       // bDeviceSubClass
    0x00,       // bDeviceProtocol
    EP0_SIZE,   // bMaxPacketSize0
    0x86, 0x1A, // idVendor (0x1A86 = WCH)
    0x10, 0x57, // idProduct (0x5710)
    0x00, 0x01, // bcdDevice 1.00
    1, 2, 3,    // iManufacturer, iProduct, iSerialNumber
    0x01,       // bNumConfigurations
};

static const PROGMEM uint8_t cfgDesc[] = {
    // Configuration descriptor
    0x09,       // bLength
    0x02,       // bDescriptorType (Configuration)
    0x20, 0x00, // wTotalLength (32 bytes)
    0x01,       // bNumInterfaces
    0x01,       // bConfigurationValue
    0x00,       // iConfiguration
    0xC0,       // bmAttributes (self-powered)
    0x32,       // bMaxPower (100 mA)

    // Interface descriptor
    0x09,       // bLength
    0x04,       // bDescriptorType (Interface)
    0x00,       // bInterfaceNumber
    0x00,       // bAlternateSetting
    0x02,       // bNumEndpoints (2: Bulk IN + Bulk OUT)
    0x08,       // bInterfaceClass (Mass Storage)
    0x06,       // bInterfaceSubClass (SCSI transparent)
    0x50,       // bInterfaceProtocol (Bulk-Only)
    0x00,       // iInterface

    // Endpoint 1 IN
    0x07,       // bLength
    0x05,       // bDescriptorType (Endpoint)
    0x81,       // bEndpointAddress (EP1 IN)
    0x02,       // bmAttributes (Bulk)
    EP1_SIZE, 0x00, // wMaxPacketSize
    0x00,       // bInterval

    // Endpoint 2 OUT
    0x07,
    0x05,
    0x02,       // bEndpointAddress (EP2 OUT)
    0x02,       // bmAttributes (Bulk)
    EP2_SIZE, 0x00,
    0x00,
};

// String descriptors
static const PROGMEM uint8_t langID[] = {
    0x04, 0x03, 0x09, 0x04  // English (US)
};

static const PROGMEM uint8_t strManufacturer[] = {
    18, 0x03,
    'P',0,'H',0,'S',0,'I',0,'2',0,'4',0,'5',0
};

static const PROGMEM uint8_t strProduct[] = {
    18, 0x03,
    'U',0,'S',0,'B',0,' ',0,'M',0,'S',0,'D',0
};

static const PROGMEM uint8_t strSerial[] = {
    8, 0x03,
    '0',0,'0',0,'1',0
};

static const PROGMEM uint8_t *strDescs[] = {
    langID, strManufacturer, strProduct, strSerial
};

// --- Packet buffer helpers ---

static void pbuf_write(uint16_t offset, const void *data, uint16_t len)
{
    volatile uint8_t *p = (volatile uint8_t *)(USB_PBUF_BASE + offset);
    const uint8_t *src = (const uint8_t *)data;
    while (len--) *p++ = *src++;
}

static void pbuf_read(uint16_t offset, void *data, uint16_t len)
{
    volatile uint8_t *p = (volatile uint8_t *)(USB_PBUF_BASE + offset);
    uint8_t *dst = (uint8_t *)data;
    while (len--) *dst++ = *p++;
}

// --- USB endpoint setup ---

static void epSetTxAddr(uint8_t ep, uint16_t offset)
{
    BTABLE_TX_ADDR(ep) = offset;
}

static void epSetRxAddr(uint8_t ep, uint16_t offset)
{
    BTABLE_RX_ADDR(ep) = offset;
}

static void epSetTxCount(uint8_t ep, uint16_t len)
{
    BTABLE_TX_COUNT(ep) = len;
}

static void epSetRxCount(uint8_t ep, uint16_t len)
{
    // Bit 15 enables the endpoint, bits 9:0 are the count
    uint16_t val = len;
    if (len > 0) val |= 0x8000;  // enable + block size
    else val = 0x8000;
    BTABLE_RX_COUNT(ep) = val;
}

static void epClearCTRx(uint8_t ep)
{
    // Clear correct transfer RX flag
    if (ep == 0)
        USBFSD->UEP0_CTRL &= ~0x8000;
    else if (ep == 1)
        USBFSD->UEP1_CTRL &= ~0x8000;
    else if (ep == 2)
        USBFSD->UEP2_CTRL &= ~0x8000;
    else if (ep == 3)
        USBFSD->UEP3_CTRL &= ~0x8000;
}

static void epClearCTTx(uint8_t ep)
{
    if (ep == 0)
        USBFSD->UEP0_CTRL &= ~0x0040;
    else if (ep == 1)
        USBFSD->UEP1_CTRL &= ~0x0040;
    else if (ep == 2)
        USBFSD->UEP2_CTRL &= ~0x0040;
    else if (ep == 3)
        USBFSD->UEP3_CTRL &= ~0x0040;
}

static void epSetStatus(uint8_t ep, uint16_t stat)
{
    volatile uint32_t *ctrl = 0;
    if (ep == 0) ctrl = &USBFSD->UEP0_CTRL;
    else if (ep == 1) ctrl = &USBFSD->UEP1_CTRL;
    else if (ep == 2) ctrl = &USBFSD->UEP2_CTRL;
    else if (ep == 3) ctrl = &USBFSD->UEP3_CTRL;
    else return;

    *ctrl = (*ctrl & ~0x0030) | (stat & 0x0030);
}

static void epToggle(uint8_t ep)
{
    volatile uint32_t *ctrl = 0;
    if (ep == 0) ctrl = &USBFSD->UEP0_CTRL;
    else if (ep == 1) ctrl = &USBFSD->UEP1_CTRL;
    else if (ep == 2) ctrl = &USBFSD->UEP2_CTRL;
    else if (ep == 3) ctrl = &USBFSD->UEP3_CTRL;
    else return;

    *ctrl ^= 0x0020;
}

// Send data on EP1 IN (bulk)
static void ep1Send(const void *data, uint16_t len)
{
    if (len == 0) {
        epSetTxCount(EP1_IN, 0);
        return;
    }
    pbuf_write(EP1_TX_OFF, data, len);
    epSetTxCount(EP1_IN, len);
    epSetStatus(EP1_IN, 0x0010);  // STAT_TX = VALID
}

// Prepare EP2 OUT for receiving
static void ep2Recv()
{
    epSetRxCount(EP2_OUT, EP2_SIZE);
    epSetStatus(EP2_OUT, 0x0030);  // STAT_RX = VALID
}

// --- USB control transfer handling ---

static void handleSetupPacket()
{
    // Read 8-byte setup packet from EP0 RX buffer
    uint8_t setup[8];
    pbuf_read(EP0_RX_OFF, setup, 8);

    uint8_t  bmRequestType = setup[0];
    uint8_t  bRequest      = setup[1];
    uint16_t wValue        = setup[2] | ((uint16_t)setup[3] << 8);
    uint16_t wIndex        = setup[4] | ((uint16_t)setup[5] << 8);
    uint16_t wLength       = setup[6] | ((uint16_t)setup[7] << 8);

    // Clear CTR after reading setup
    USBFSD->UEP0_CTRL &= ~0x8000;  // clear RX CTR
    USBFSD->UEP0_CTRL &= ~0x0040;  // clear TX CTR

    // Set endpoint status for data stage or status stage
    // Default: STALL if request not handled
    bool handled = false;

    if ((bmRequestType & 0x60) == 0x00) {  // Standard Device request
        switch (bRequest) {

        case REQ_SET_ADDRESS:
            usbAddr = wValue;
            // Status stage: send zero-length packet
            epSetTxCount(EP0, 0);
            epSetStatus(EP0, 0x0010);  // TX valid
            epSetRxCount(EP0, 0);
            epSetStatus(EP0, 0x0030);  // RX valid
            handled = true;
            break;

        case REQ_SET_CONFIGURATION:
            usbConfigured = wValue;
            if (usbConfigured) {
                // Configure endpoints
                USBFSD->UEP4_1_MOD = 0x80;  // EP1 TX (IN)
                USBFSD->UEP2_3_MOD = 0x00;  // EP2 RX (OUT)
                epSetTxAddr(EP1_IN, EP1_TX_OFF);
                epSetRxAddr(EP2_OUT, EP2_RX_OFF);
                epSetTxCount(EP1_IN, 0);
                epSetStatus(EP1_IN, 0x0000);  // NAK initially
                epSetStatus(EP2_OUT, 0x0000); // NAK initially
            }
            // Status stage
            epSetTxCount(EP0, 0);
            epSetStatus(EP0, 0x0010);
            epSetRxCount(EP0, 0);
            epSetStatus(EP0, 0x0030);
            handled = true;
            break;

        case REQ_GET_CONFIGURATION:
            {
                uint8_t cfg = usbConfigured ? 1 : 0;
                pbuf_write(EP0_TX_OFF, &cfg, 1);
                epSetTxCount(EP0, 1);
                epSetStatus(EP0, 0x0010);  // TX valid
                epSetRxCount(EP0, 0);
                epSetStatus(EP0, 0x0030);  // RX valid
                handled = true;
            }
            break;

        case REQ_GET_STATUS:
            {
                uint16_t status = 0;  // self-powered = 1, remote wakeup = 0
                if (wLength > 2) wLength = 2;
                pbuf_write(EP0_TX_OFF, &status, wLength);
                epSetTxCount(EP0, wLength);
                epSetStatus(EP0, 0x0010);
                epSetRxCount(EP0, 0);
                epSetStatus(EP0, 0x0030);
                handled = true;
            }
            break;

        case REQ_CLEAR_FEATURE:
        case REQ_SET_FEATURE:
            // No features to set/clear — accept
            epSetTxCount(EP0, 0);
            epSetStatus(EP0, 0x0010);
            epSetRxCount(EP0, 0);
            epSetStatus(EP0, 0x0030);
            handled = true;
            break;
        }
    }

    if ((bmRequestType & 0x60) == 0x80) {  // Standard Device → Host
        if (bRequest == REQ_GET_DESCRIPTOR) {
            uint8_t descType = wValue >> 8;
            uint8_t descIdx  = wValue & 0xFF;

            if (descType == DESC_DEVICE) {
                if (wLength > sizeof(devDesc)) wLength = sizeof(devDesc);
                pbuf_write(EP0_TX_OFF, devDesc, wLength);
                epSetTxCount(EP0, wLength);
                epSetStatus(EP0, 0x0010);
                epSetRxCount(EP0, 0);
                epSetStatus(EP0, 0x0030);
                handled = true;
            } else if (descType == DESC_CONFIG) {
                if (wLength > sizeof(cfgDesc)) wLength = sizeof(cfgDesc);
                pbuf_write(EP0_TX_OFF, cfgDesc, wLength);
                epSetTxCount(EP0, wLength);
                epSetStatus(EP0, 0x0010);
                epSetRxCount(EP0, 0);
                epSetStatus(EP0, 0x0030);
                handled = true;
            } else if (descType == DESC_STRING && descIdx < 4) {
                const uint8_t *sd = strDescs[descIdx];
                uint8_t len = pgm_read_byte(sd);
                if (wLength > len) wLength = len;
                pbuf_write(EP0_TX_OFF, sd, wLength);
                epSetTxCount(EP0, wLength);
                epSetStatus(EP0, 0x0010);
                epSetRxCount(EP0, 0);
                epSetStatus(EP0, 0x0030);
                handled = true;
            }
        }
    }

    if (!handled) {
        // STALL
        epSetStatus(EP0, 0x0020);
        epSetRxCount(EP0, 0);
        epSetStatus(EP0, 0x0020);
    }
}

// --- USB interrupt handler ---

static void handleEP0Out()
{
    // EP0 OUT transfer complete (data stage of control write)
    USBFSD->UEP0_CTRL &= ~0x8000;  // clear RX CTR
}

static void handleEP0In()
{
    // EP0 IN transfer complete
    USBFSD->UEP0_CTRL &= ~0x0040;  // clear TX CTR
}

// --- SCSI command processing ---

static void scsiTestUnitReady()
{
    // Always ready if SD is available
    if (storage::sdAvailable()) {
        senseKey = 0;
        csw.bCSWStatus = 0;  // command passed
    } else {
        senseKey = 2;  // NOT READY
        senseASC = 0x3A; // MEDIUM NOT PRESENT
        csw.bCSWStatus = 1;  // command failed
    }
    csw.dCSWDataResidue = 0;
}

static void scsiRequestSense()
{
    uint8_t sense[18];
    memset(sense, 0, 18);
    sense[0] = 0x70;    // response code
    sense[2] = senseKey;
    sense[7] = 10;      // additional sense length
    sense[12] = senseASC;

    uint8_t len = cbw.CBWCB[4];
    if (len > 18) len = 18;
    ep1Send(sense, len);
    csw.dCSWDataResidue = cbw.dCBWDataTransferLength - len;
    csw.bCSWStatus = 0;

    senseKey = 0;
    senseASC = 0;
}

static void scsiInquiry()
{
    uint8_t inquiry[36];
    memset(inquiry, 0, 36);
    inquiry[0] = 0x00;         // SBC direct-access device
    inquiry[1] = 0x80;         // removable
    inquiry[2] = 0x04;         // SPC-2
    inquiry[3] = 0x02;         // response data format
    inquiry[4] = 31;           // additional length
    memcpy(inquiry + 8,  "PHSI245 ", 8);
    memcpy(inquiry + 16, "SD Card    ", 12);
    memcpy(inquiry + 28, "1.00", 4);

    uint8_t len = cbw.CBWCB[4];
    if (len > 36) len = 36;
    ep1Send(inquiry, len);
    csw.dCSWDataResidue = cbw.dCBWDataTransferLength - len;
    csw.bCSWStatus = 0;
}

static void scsiModeSense6()
{
    uint8_t mode[4] = {3, 0, 0, 0};  // mode parameter header
    ep1Send(mode, 4);
    csw.dCSWDataResidue = cbw.dCBWDataTransferLength - 4;
    csw.bCSWStatus = 0;
}

static void scsiReadCapacity10()
{
    uint32_t lastBlock = sd::blockCount() - 1;
    if (lastBlock > 0xFFFFFFFF) lastBlock = 0xFFFFFFFF;

    uint8_t cap[8];
    cap[0] = (uint8_t)(lastBlock >> 24);
    cap[1] = (uint8_t)(lastBlock >> 16);
    cap[2] = (uint8_t)(lastBlock >> 8);
    cap[3] = (uint8_t)(lastBlock);
    cap[4] = (uint8_t)(msdBlockSize >> 24);
    cap[5] = (uint8_t)(msdBlockSize >> 16);
    cap[6] = (uint8_t)(msdBlockSize >> 8);
    cap[7] = (uint8_t)(msdBlockSize);

    ep1Send(cap, 8);
    csw.dCSWDataResidue = cbw.dCBWDataTransferLength - 8;
    csw.bCSWStatus = 0;
}

static bool scsiRead10()
{
    uint32_t lba = ((uint32_t)cbw.CBWCB[2] << 24) | ((uint32_t)cbw.CBWCB[3] << 16)
                 | ((uint32_t)cbw.CBWCB[4] << 8)  | cbw.CBWCB[5];
    uint16_t count = ((uint16_t)cbw.CBWCB[7] << 8) | cbw.CBWCB[8];
    if (count == 0) count = 256;

    if (lba + count > sd::blockCount()) {
        csw.dCSWDataResidue = cbw.dCBWDataTransferLength;
        csw.bCSWStatus = 1;
        senseKey = 5;  // ILLEGAL REQUEST
        senseASC = 0x21; // LBA OUT OF RANGE
        return false;
    }

    // Read and send one block per call to avoid hogging the CPU
    static uint32_t readLBA    = 0;
    static uint16_t readRemain = 0;
    readLBA    = lba;
    readRemain = count;
    (void)readLBA; (void)readRemain;  // used across calls

    // Read first block and send
    if (!sd::readBlock(lba, msdBuf)) {
        csw.dCSWDataResidue = cbw.dCBWDataTransferLength;
        csw.bCSWStatus = 1;
        senseKey = 4;  // HARDWARE ERROR
        return false;
    }
    ep1Send(msdBuf, 512);
    csw.dCSWDataResidue = 0; // handled by caller
    csw.bCSWStatus = 0;
    return true;
}

static void scsiWrite10()
{
    uint32_t lba = ((uint32_t)cbw.CBWCB[2] << 24) | ((uint32_t)cbw.CBWCB[3] << 16)
                 | ((uint32_t)cbw.CBWCB[4] << 8)  | cbw.CBWCB[5];
    uint16_t count = ((uint16_t)cbw.CBWCB[7] << 8) | cbw.CBWCB[8];
    if (count == 0) count = 256;

    if (lba + count > sd::blockCount()) {
        csw.dCSWDataResidue = cbw.dCBWDataTransferLength;
        csw.bCSWStatus = 1;
        senseKey = 5;
        senseASC = 0x21;
        // Need to receive and discard data
        return;
    }

    // For writes, we expect data on EP2 OUT. The data is in the RX buffer.
    // Read from EP2 buffer and write to SD.
    uint16_t rxCount = USBFSD->RX_LEN & 0x3FF;
    if (rxCount >= 512) rxCount = 512;
    pbuf_read(EP2_RX_OFF, msdBuf, rxCount);

    ep2Recv();  // prepare for next packet

    if (!sd::writeBlock(lba, msdBuf)) {
        csw.dCSWDataResidue = cbw.dCBWDataTransferLength;
        csw.bCSWStatus = 1;
        senseKey = 4;
        return;
    }

    csw.dCSWDataResidue = cbw.dCBWDataTransferLength - 512;
    csw.bCSWStatus = 0;
}

static void scsiPreventAllow()
{
    csw.dCSWDataResidue = 0;
    csw.bCSWStatus = 0;
}

static void scsiStopStart()
{
    csw.dCSWDataResidue = 0;
    csw.bCSWStatus = 0;
}

// --- BOT state machine ---
enum BotState {
    BOT_IDLE,
    BOT_CBW_READY,     // CBW received by ISR, pending processing
    BOT_DATA_OUT,      // waiting for write data from host
    BOT_DATA_IN,       // sending read data to host
    BOT_CSW,           // sending CSW
};

static volatile BotState botState = BOT_IDLE;
static uint8_t  cbwBuf[32];
static volatile uint8_t  cbwPos = 0;
static volatile bool     cbwReady = false;

// Write state tracking
static uint32_t writeLBA;
static uint16_t writeRemain;
static uint16_t writeBufPos;  // bytes received into msdBuf

static void processCBW()
{
    memcpy(&cbw, cbwBuf, BOT_CBW_LEN);
    csw.dCSWSignature  = BOT_CSW_SIGNATURE;
    csw.dCSWTag         = cbw.dCBWTag;
    csw.dCSWDataResidue = 0;
    csw.bCSWStatus      = 0;

    uint8_t op = cbw.CBWCB[0];

    switch (op) {
    case SCSI_TEST_UNIT_READY:
        scsiTestUnitReady();
        botState = BOT_CSW;
        cbwReady = false;
        break;
    case SCSI_REQUEST_SENSE:
        scsiRequestSense();
        botState = BOT_DATA_IN;
        cbwReady = false;
        break;
    case SCSI_INQUIRY:
        scsiInquiry();
        botState = BOT_DATA_IN;
        cbwReady = false;
        break;
    case SCSI_MODE_SENSE_6:
        scsiModeSense6();
        botState = BOT_DATA_IN;
        cbwReady = false;
        break;
    case SCSI_READ_CAPACITY_10:
        scsiReadCapacity10();
        botState = BOT_DATA_IN;
        cbwReady = false;
        break;
    case SCSI_READ_10:
        if (scsiRead10())
            botState = BOT_DATA_IN;
        else
            botState = BOT_CSW;
        cbwReady = false;
        break;
    case SCSI_WRITE_10:
        writeLBA = ((uint32_t)cbw.CBWCB[2] << 24) | ((uint32_t)cbw.CBWCB[3] << 16)
                 | ((uint32_t)cbw.CBWCB[4] << 8)  | cbw.CBWCB[5];
        writeRemain = ((uint16_t)cbw.CBWCB[7] << 8) | cbw.CBWCB[8];
        if (writeRemain == 0) writeRemain = 256;
        writeBufPos = 0;
        if (cbw.dCBWDataTransferLength > 0) {
            botState = BOT_DATA_OUT;
            ep2Recv();  // start receiving write data
        } else {
            scsiWrite10();
            botState = BOT_CSW;
        }
        cbwReady = false;
        break;
    case SCSI_PREVENT_ALLOW:
        scsiPreventAllow();
        botState = BOT_CSW;
        cbwReady = false;
        break;
    case SCSI_START_STOP_UNIT:
        scsiStopStart();
        botState = BOT_CSW;
        cbwReady = false;
        break;
    default:
        senseKey = 5;
        senseASC = 0x20;
        csw.bCSWStatus = 1;
        csw.dCSWDataResidue = cbw.dCBWDataTransferLength;
        botState = BOT_CSW;
        cbwReady = false;
        break;
    }
}

static void sendCSW()
{
    ep1Send(&csw, BOT_CSW_LEN);
    botState = BOT_IDLE;
}

// --- Public API ---

void usb_msd::init()
{
    // Enable USB clock
    RCC->AHBPCENR |= RCC_USBFS;

    // Configure USB pins PC16 (UDM/D-), PC17 (UDP/D+)
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

    GPIO_InitTypeDef g = {0};
    g.GPIO_Pin = GPIO_Pin_16 | GPIO_Pin_17;

    // Actually, PC16 and PC17 need special USB peripheral mode.
    // Set as alternate function push-pull with proper USB AF mode.
    // On CH32X035, the USB pins are automatically routed when USBFS is enabled.
    g.GPIO_Mode = GPIO_Mode_IN_FLOATING;  // USB PHY handles pin direction
    g.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &g);

    // Reset USB peripheral
    USBFSD->BASE_CTRL = 0;
    for (volatile int i = 0; i < 200; i++) __asm__("nop");
    USBFSD->BASE_CTRL = 0x04;  // USB enable (bit 2 = USBEN)

    // Clear all interrupts
    USBFSD->INT_FG = 0xFF;

    // Enable interrupts: reset, suspend, CTR (correct transfer)
    USBFSD->INT_EN = (1 << 6) | (1 << 5) | (1 << 4) | (1 << 0);

    // Set up buffer descriptor table
    // Endpoint 0: TX at offset 0, RX at offset 64
    epSetTxAddr(EP0, EP0_TX_OFF);
    epSetRxAddr(EP0, EP0_RX_OFF);
    epSetRxCount(EP0, EP0_SIZE);  // enable RX
    epSetStatus(EP0, 0x0030);     // RX valid

    // Set device address to 0 initially
    USBFSD->DEV_ADDR = 0;

    // Enable USB pull-up on D+ (UDEV_CTRL bit 0 = PDEN)
    USBFSD->UDEV_CTRL = 0x01;

    // Enable USBFS interrupt
    NVIC_EnableIRQ(USBFS_IRQn);

    // Initialize MSD
    msdBlockSize = 512;
    if (storage::sdAvailable())
        msdBlockCount = sd::blockCount();
    else
        msdBlockCount = 0;

    botState = BOT_IDLE;
    cbwPos = 0;
}

bool usb_msd::isConnected()
{
    // Check if USB reset has been received (host is present)
    return hostPresent;
}

static bool dataProcessed = false;

bool usb_msd::update()
{
    if (cbwReady) {
        processCBW();
        return true;
    }
    return false;
}

// --- USB interrupt handler ---
extern "C" void USBFS_IRQHandler(void)
{
    uint8_t ist = USBFSD->INT_ST;

    // Reset
    if (ist & 0x10) {
        USBFSD->INT_FG = 0x10;
        usbAddr = 0;
        USBFSD->DEV_ADDR = 0;
        hostPresent = true;

        // Re-init buffer table
        epSetTxAddr(EP0, EP0_TX_OFF);
        epSetRxAddr(EP0, EP0_RX_OFF);
        epSetRxCount(EP0, EP0_SIZE);
        epSetStatus(EP0, 0x0030);
        epSetTxCount(EP0, 0);
        botState = BOT_IDLE;
        cbwPos = 0;
        epSetStatus(EP1_IN, 0x0000);
        epSetStatus(EP2_OUT, 0x0000);
    }

    // Suspend / Wakeup
    if (ist & 0x08) {
        USBFSD->INT_FG = 0x08;
    }
    if (ist & 0x20) {
        USBFSD->INT_FG = 0x20;
    }

    // Correct transfer (CTR) — endpoint activity
    if (ist & 0x80) {
        uint8_t ep = (USBFSD->INT_ST >> 4) & 0x07;

        // Determine direction
        uint16_t ctrl;
        if (ep == 0) ctrl = USBFSD->UEP0_CTRL;
        else if (ep == 1) ctrl = USBFSD->UEP1_CTRL;
        else if (ep == 2) ctrl = USBFSD->UEP2_CTRL;
        else if (ep == 3) ctrl = USBFSD->UEP3_CTRL;
        else goto ctr_done;

        bool isRx = (ctrl & 0x8000) != 0;  // RX transfer complete

        if (ep == 0) {
            // Control endpoint
            if (isRx) {
                // Check if this is a SETUP packet
                if (USBFSD->UEP0_CTRL & 0x0800) {  // SETUP flag
                    handleSetupPacket();
                } else {
                    // EP0 OUT data
                    handleEP0Out();
                }
            } else {
                // EP0 IN complete
                handleEP0In();
                // Apply address if pending
                if (usbAddr) {
                    USBFSD->DEV_ADDR = usbAddr;
                    usbAddr = 0;
                }
            }
        } else if (ep == EP1_IN) {
            // Bulk IN complete
            USBFSD->UEP1_CTRL &= ~0x0040;
            if (botState == BOT_DATA_IN) {
                botState = BOT_CSW;
                sendCSW();
            } else if (botState == BOT_CSW) {
                // CSW sent, reset state, prepare for next command
                botState = BOT_IDLE;
                cbwPos = 0;
                ep2Recv();
            }
            dataProcessed = true;
        } else if (ep == EP2_OUT) {
            // Bulk OUT received
            USBFSD->UEP2_CTRL &= ~0x8000;
            uint16_t len = (USBFSD->RX_LEN & 0x3FF);
            if (len > 0) {
                if (botState == BOT_DATA_OUT) {
                    // Receiving write data — store in msdBuf
                    if (writeBufPos + len > 512) len = 512 - writeBufPos;
                    pbuf_read(EP2_RX_OFF, msdBuf + writeBufPos, len);
                    writeBufPos += len;
                    if (writeBufPos >= 512) {
                        // Full sector received — write to SD
                        if (!sd::writeBlock(writeLBA, msdBuf)) {
                            csw.bCSWStatus = 1;
                            senseKey = 4;
                        }
                        writeLBA++;
                        writeRemain--;
                        writeBufPos = 0;
                        if (writeRemain == 0) {
                            csw.dCSWDataResidue = 0;
                            botState = BOT_CSW;
                            sendCSW();
                        }
                    }
                } else {
                    // Receiving CBW
                    pbuf_read(EP2_RX_OFF, cbwBuf + cbwPos, len);
                    cbwPos += len;
                    if (cbwPos >= BOT_CBW_LEN) {
                        cbwReady = true;
                        cbwPos = 0;
                    }
                }
            }
            ep2Recv();
        }
    }
ctr_done:
    ;

    // Clear CTR interrupt flag (write 0 to clear)
    USBFSD->INT_FG = 0x80;
}
