#include "storage.h"
#include <string.h>

// FAT directory entry structure
struct DirEntry {
    uint8_t  name[11];   // 8.3 filename, space-padded
    uint8_t  attr;       // 0x10 = dir, 0x0F = LFN
    uint8_t  reserved;
    uint8_t  crtTimeTenth;
    uint16_t crtTime;
    uint16_t crtDate;
    uint16_t lstAccDate;
    uint16_t fstClusHI;  // FAT32: high word of start cluster
    uint16_t wrtTime;
    uint16_t wrtDate;
    uint16_t fstClusLO;  // low word of start cluster
    uint32_t fileSize;
};

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20
#define ATTR_LFN       0x0F

namespace fat {

// --- Internal state ---
static const BlockDevice *dev = 0;
static uint8_t  sectorBuf[512];
static uint32_t fatType;          // 16 or 32
static uint16_t bytesPerSector;
static uint8_t  sectorsPerCluster;
static uint16_t reservedSectors;
static uint8_t  numFATs;
static uint32_t sectorsPerFAT;
static uint32_t rootDirCluster;   // FAT32: first cluster of root dir
static uint32_t firstDataSector;
static uint32_t firstFATSector;
static uint32_t rootDirSectors;   // FAT16 only

// File state
static uint32_t fileStartCluster;
static uint32_t fileCurCluster;
static uint32_t fileSize;
static uint32_t filePos;
static bool     fileOpen;
static bool     fileWriting;

// Write cursor: current cluster and sector-within-cluster
static uint32_t writeCluster;
static uint8_t  writeSectorInCluster;

// Directory iterator state
static uint32_t dirCluster;
static uint32_t dirSector;
static uint8_t  dirEntryIndex;   // entry index within sector (0-15)

// --- Helpers ---

static uint32_t clustToSector(uint32_t cluster)
{
    return firstDataSector + (cluster - 2) * sectorsPerCluster;
}

static uint32_t getStartCluster(const DirEntry *entry)
{
    uint32_t c = entry->fstClusLO | ((uint32_t)entry->fstClusHI << 16);
    return c;
}

// Convert 8.3 raw name to null-terminated string
static void rawNameToStr(const uint8_t *raw, char *out)
{
    // Name (8 chars)
    for (int i = 0; i < 8; i++) {
        if (raw[i] == ' ' || raw[i] == 0) break;
        *out++ = raw[i];
    }
    // Extension (3 chars), skip if no extension
    if (raw[8] != ' ' && raw[8] != 0) {
        *out++ = '.';
        for (int i = 8; i < 11; i++) {
            if (raw[i] == ' ' || raw[i] == 0) break;
            *out++ = raw[i];
        }
    }
    *out = '\0';
}

// Compare a C string path to an 8.3 raw name
// Handles: "FILENAME.EXT" or "FILENAME" (case-insensitive)
static bool nameMatch(const uint8_t *raw, const char *path)
{
    // Parse the C string into name (8) + ext (3)
    char name[12];
    int i = 0;
    while (*path && *path != '.' && i < 8)
        name[i++] = *path++;
    while (i < 8) name[i++] = ' ';
    if (*path == '.') {
        path++; // skip dot
        while (*path && i < 11)
            name[i++] = *path++;
    }
    while (i < 11) name[i++] = ' ';

    // Case-insensitive compare
    for (int j = 0; j < 11; j++) {
        uint8_t a = raw[j];
        uint8_t b = name[j];
        // ASCII uppercase comparison
        if (a >= 'a' && a <= 'z') a -= 32;
        if (b >= 'a' && b <= 'z') b -= 32;
        if (a != b) return false;
    }
    return true;
}

// --- Cluster chain traversal ---

static uint32_t nextCluster(uint32_t cluster)
{
    if (cluster < 2) return 0;

    uint32_t sector;
    uint32_t offset;

    if (fatType == 32) {
        // FAT32: 4 bytes per entry
        sector = firstFATSector + (cluster * 4) / bytesPerSector;
        offset = (cluster * 4) % bytesPerSector;
    } else {
        // FAT16: 2 bytes per entry
        sector = firstFATSector + (cluster * 2) / bytesPerSector;
        offset = (cluster * 2) % bytesPerSector;
    }

    if (!dev->readBlock(sector, sectorBuf))
        return 0;

    if (fatType == 32) {
        uint32_t val = sectorBuf[offset]
                     | ((uint32_t)sectorBuf[offset + 1] << 8)
                     | ((uint32_t)sectorBuf[offset + 2] << 16)
                     | ((uint32_t)sectorBuf[offset + 3] << 24);
        return val & 0x0FFFFFFF;
    } else {
        uint16_t val = sectorBuf[offset] | ((uint16_t)sectorBuf[offset + 1] << 8);
        if (val >= 0xFFF8) return 0xFFFFFFFF;
        return val;
    }
}

static bool isLastCluster(uint32_t cluster)
{
    if (fatType == 32)
        return (cluster >= 0x0FFFFFF8);
    else
        return (cluster >= 0xFFF8);
}

// --- FAT entry read/write ---

static bool writeFATEntry(uint32_t cluster, uint32_t value)
{
    if (cluster < 2) return false;

    uint32_t sector;
    uint32_t offset;

    if (fatType == 32) {
        sector = firstFATSector + (cluster * 4) / bytesPerSector;
        offset = (cluster * 4) % bytesPerSector;
    } else {
        sector = firstFATSector + (cluster * 2) / bytesPerSector;
        offset = (cluster * 2) % bytesPerSector;
    }

    if (!dev->readBlock(sector, sectorBuf))
        return false;

    if (fatType == 32) {
        sectorBuf[offset]     = value & 0xFF;
        sectorBuf[offset + 1] = (value >> 8) & 0xFF;
        sectorBuf[offset + 2] = (value >> 16) & 0xFF;
        sectorBuf[offset + 3] = ((value >> 24) & 0x0F) | (sectorBuf[offset + 3] & 0xF0);
    } else {
        sectorBuf[offset]     = value & 0xFF;
        sectorBuf[offset + 1] = (value >> 8) & 0xFF;
    }

    // Write to all FAT copies
    for (uint8_t f = 0; f < numFATs; f++) {
        if (!dev->writeBlock(sector + f * sectorsPerFAT, sectorBuf))
            return false;
    }
    return true;
}

// Find a free cluster, mark it as end-of-chain, return cluster number
static uint32_t allocCluster()
{
    uint32_t totalClusters = (dev->blockCount() - firstDataSector) / sectorsPerCluster + 2;
    for (uint32_t c = 2; c < totalClusters; c++) {
        uint32_t val = nextCluster(c);
        if (val == 0) { // free
            // Get the FAT end-of-chain marker
            uint32_t eoc = (fatType == 32) ? 0x0FFFFFFF : 0xFFFF;
            if (writeFATEntry(c, eoc))
                return c;
        }
    }
    return 0;
}

// Append a cluster to an existing chain. Returns the new cluster.
// `prev` is the last cluster in the chain.
static uint32_t appendCluster(uint32_t prev)
{
    uint32_t newClust = allocCluster();
    if (newClust == 0) return 0;
    if (!writeFATEntry(prev, newClust)) {
        writeFATEntry(newClust, 0); // free it
        return 0;
    }
    return newClust;
}

// --- Directory iteration ---

// Read a specific sector of a directory cluster chain
static bool readDirSector(uint32_t cluster, uint32_t sectorIndex, uint8_t *buf)
{
    uint32_t sec = clustToSector(cluster) + sectorIndex;
    return dev->readBlock(sec, buf);
}

// Write a directory sector (used for file creation)
static bool writeDirSector(uint32_t cluster, uint32_t sectorIndex, const uint8_t *buf)
{
    uint32_t sec = clustToSector(cluster) + sectorIndex;
    return dev->writeBlock(sec, buf);
}

// Find a directory entry by name within a directory cluster chain.
// Returns true and fills `entry` if found.
// If `findFree` is true, looks for a free entry slot instead.
// `foundCluster` and `foundSector` and `foundIndex` report where it was found.
static bool findDirEntry(uint32_t dirClust, const char *name, DirEntry *entry,
                         bool findFree,
                         uint32_t *foundCluster, uint32_t *foundSector, uint8_t *foundIndex)
{
    uint32_t cluster = dirClust;
    uint8_t  sectorIdx = 0;

    while (cluster && cluster < 0x0FFFFFF8) {
        if (!readDirSector(cluster, sectorIdx, sectorBuf))
            return false;

        for (uint8_t i = 0; i < 16; i++) {
            DirEntry *e = (DirEntry *)(sectorBuf + i * 32);
            uint8_t firstByte = e->name[0];

            if (firstByte == 0x00) {
                // End of directory
                if (findFree) {
                    *foundCluster = cluster;
                    *foundSector = sectorIdx;
                    *foundIndex = i;
                    return true;
                }
                return false;
            }
            if (firstByte == 0xE5) {
                // Deleted entry — free slot
                if (findFree) {
                    *foundCluster = cluster;
                    *foundSector = sectorIdx;
                    *foundIndex = i;
                    return true;
                }
                continue;
            }
            if (e->attr == ATTR_LFN) continue; // skip LFN entries

            if (!findFree && nameMatch(e->name, name)) {
                memcpy(entry, e, sizeof(DirEntry));
                return true;
            }
        }

        // Move to next sector
        sectorIdx++;
        if (sectorIdx >= sectorsPerCluster) {
            sectorIdx = 0;
            cluster = nextCluster(cluster);
        }
    }
    return false;
}

// --- Parse a path into directory cluster + filename ---
// Returns the cluster of the parent directory and the final component name.
// If path is just a filename, parent is root dir.
static bool parsePath(const char *path, uint32_t *parentClust, const char **fname)
{
    *parentClust = (fatType == 32) ? rootDirCluster : 0;
    *fname = path;

    // Skip leading slash
    if (*path == '/') path++;

    const char *slash = strchr(path, '/');
    if (!slash) {
        *fname = path;
        return true;
    }

    // Traverse subdirectories
    DirEntry entry;
    while (slash) {
        // Extract component
        char component[13];
        int len = slash - path;
        if (len > 12) len = 12;
        memcpy(component, path, len);
        component[len] = '\0';

        // Look up in current directory
        if (!findDirEntry(*parentClust, component, &entry, false, 0, 0, 0))
            return false;
        if (!(entry.attr & ATTR_DIRECTORY))
            return false;

        *parentClust = getStartCluster(&entry);
        path = slash + 1;
        slash = strchr(path, '/');
    }
    *fname = path;
    return true;
}

// --- Public API ---

Result mount(const BlockDevice *d)
{
    dev = d;
    fileOpen = false;
    fileWriting = false;

    if (!dev->init())
        return DISK_ERR;

    // Read MBR/BPB (sector 0)
    if (!dev->readBlock(0, sectorBuf))
        return DISK_ERR;

    // Check for MBR (partition table at offset 0x1BE)
    // If byte 0 is 0xEB or 0xE9, it's a BPB directly
    if (sectorBuf[0] == 0xEB || sectorBuf[0] == 0xE9) {
        // BPB at sector 0
    } else if (sectorBuf[0x1FE] == 0x55 && sectorBuf[0x1FF] == 0xAA) {
        // MBR present — read first partition entry
        uint8_t *part = sectorBuf + 0x1BE;
        uint32_t partStart = part[8] | ((uint32_t)part[9] << 8)
                           | ((uint32_t)part[10] << 16) | ((uint32_t)part[11] << 24);
        if (!dev->readBlock(partStart, sectorBuf))
            return DISK_ERR;
    } else {
        return NO_FILESYSTEM;
    }

    // Parse BPB
    bytesPerSector    = sectorBuf[11] | ((uint16_t)sectorBuf[12] << 8);
    sectorsPerCluster = sectorBuf[13];
    reservedSectors   = sectorBuf[14] | ((uint16_t)sectorBuf[15] << 8);
    numFATs           = sectorBuf[16];
    uint16_t rootEnts = sectorBuf[17] | ((uint16_t)sectorBuf[18] << 8);
    uint16_t totalSec16 = sectorBuf[19] | ((uint16_t)sectorBuf[20] << 8);
    uint32_t totalSec32 = sectorBuf[32] | ((uint32_t)sectorBuf[33] << 8)
                        | ((uint32_t)sectorBuf[34] << 16) | ((uint32_t)sectorBuf[35] << 24);

    sectorsPerFAT     = sectorBuf[22] | ((uint16_t)sectorBuf[23] << 8);
    if (sectorsPerFAT == 0) {
        // FAT32: 32-bit value at offset 36
        sectorsPerFAT = sectorBuf[36] | ((uint32_t)sectorBuf[37] << 8)
                      | ((uint32_t)sectorBuf[38] << 16) | ((uint32_t)sectorBuf[39] << 24);
    }

    uint32_t totalSectors = (totalSec16 != 0) ? totalSec16 : totalSec32;

    rootDirSectors = ((rootEnts * 32) + (bytesPerSector - 1)) / bytesPerSector;

    firstFATSector  = reservedSectors;
    firstDataSector = reservedSectors + (numFATs * sectorsPerFAT) + rootDirSectors;

    uint32_t dataSectors = totalSectors - firstDataSector;
    uint32_t countOfClusters = dataSectors / sectorsPerCluster;

    if (countOfClusters < 4085)
        fatType = 12; // unlikely but handle
    else if (countOfClusters < 65525)
        fatType = 16;
    else
        fatType = 32;

    // FAT32 root directory cluster
    if (fatType == 32) {
        rootDirCluster = sectorBuf[44] | ((uint32_t)sectorBuf[45] << 8)
                       | ((uint32_t)sectorBuf[46] << 16) | ((uint32_t)sectorBuf[47] << 24);
        firstDataSector = reservedSectors + (numFATs * sectorsPerFAT); // no root dir area
    }

    return OK;
}

Result open(const char *path)
{
    if (!dev) return NOT_READY;

    uint32_t parentClust;
    const char *fname;
    if (!parsePath(path, &parentClust, &fname))
        return NO_FILE;

    DirEntry entry;
    // FAT16: root dir is a fixed region, not in cluster chain
    if (fatType != 32 && parentClust == 0) {
        // Search root directory region
        bool found = false;
        for (uint32_t sec = 0; sec < rootDirSectors; sec++) {
            if (!dev->readBlock(firstFATSector + numFATs * sectorsPerFAT + sec, sectorBuf))
                return DISK_ERR;
            for (uint8_t i = 0; i < 16; i++) {
                DirEntry *e = (DirEntry *)(sectorBuf + i * 32);
                if (e->name[0] == 0x00) { found = false; break; }
                if (e->name[0] == 0xE5) continue;
                if (e->attr == ATTR_LFN) continue;
                if (nameMatch(e->name, fname)) {
                    memcpy(&entry, e, sizeof(DirEntry));
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        if (!found) return NO_FILE;
    } else {
        if (!findDirEntry(parentClust, fname, &entry, false, 0, 0, 0))
            return NO_FILE;
    }

    if (entry.attr & ATTR_DIRECTORY)
        return NO_FILE; // can't open a directory as a file

    fileStartCluster = getStartCluster(&entry);
    fileCurCluster   = fileStartCluster;
    fileSize         = entry.fileSize;
    filePos          = 0;
    fileOpen         = true;
    fileWriting      = false;

    return OK;
}

Result read(void *buf, uint16_t btr, uint16_t *br)
{
    if (!fileOpen) return NOT_OPENED;
    if (fileWriting) return NOT_ENABLED;

    uint8_t *dst = (uint8_t *)buf;
    uint16_t bytesRead = 0;

    while (bytesRead < btr && filePos < fileSize) {
        // Determine sector within current cluster
        uint32_t sectorInCluster = (filePos % (sectorsPerCluster * bytesPerSector)) / bytesPerSector;
        uint32_t sector = clustToSector(fileCurCluster) + sectorInCluster;

        if (!dev->readBlock(sector, sectorBuf))
            return DISK_ERR;

        uint16_t offset = filePos % bytesPerSector;
        uint16_t remaining = bytesPerSector - offset;
        uint16_t fileRemaining = fileSize - filePos;
        uint16_t toRead = btr - bytesRead;
        if (toRead > remaining) toRead = remaining;
        if (toRead > fileRemaining) toRead = fileRemaining;

        memcpy(dst + bytesRead, sectorBuf + offset, toRead);
        bytesRead += toRead;
        filePos += toRead;

        // Move to next cluster if at cluster boundary
        if (filePos % (sectorsPerCluster * bytesPerSector) == 0 && filePos < fileSize) {
            fileCurCluster = nextCluster(fileCurCluster);
            if (fileCurCluster == 0 || isLastCluster(fileCurCluster))
                break;
        }
    }

    if (br) *br = bytesRead;
    return OK;
}

Result close()
{
    fileOpen = false;
    fileWriting = false;
    return OK;
}

// --- Directory listing ---

Result openDir(const char *path)
{
    if (!dev) return NOT_READY;

    if (path && path[0] != '\0' && !(path[0] == '/' && path[1] == '\0')) {
        // Open a specific subdirectory
        uint32_t parentClust;
        const char *dname;
        if (!parsePath(path, &parentClust, &dname))
            return NO_FILE;

        DirEntry entry;
        if (fatType != 32 && parentClust == 0 && dname[0] == '\0') {
            // Root dir of FAT16
            dirCluster = 0;
        } else if (dname[0] == '\0') {
            dirCluster = parentClust;
        } else {
            if (!findDirEntry(parentClust, dname, &entry, false, 0, 0, 0))
                return NO_FILE;
            if (!(entry.attr & ATTR_DIRECTORY))
                return NO_FILE;
            dirCluster = getStartCluster(&entry);
        }
    } else {
        // Root directory
        dirCluster = (fatType == 32) ? rootDirCluster : 0;
    }

    dirSector = 0;
    dirEntryIndex = 0;
    return OK;
}

bool readDir(char *name, bool *isDir, uint32_t *size)
{
    if (fatType != 32 && dirCluster == 0) {
        // FAT16 root directory: linear sector scan
        while (dirSector < rootDirSectors) {
            if (!dev->readBlock(firstFATSector + numFATs * sectorsPerFAT + dirSector, sectorBuf))
                return false;

            while (dirEntryIndex < 16) {
                DirEntry *e = (DirEntry *)(sectorBuf + dirEntryIndex * 32);
                dirEntryIndex++;

                if (e->name[0] == 0x00) return false; // end
                if (e->name[0] == 0xE5) continue;
                if (e->attr == ATTR_LFN) continue;
                if (e->attr & ATTR_VOLUME_ID) continue;

                rawNameToStr(e->name, name);
                if (isDir) *isDir = (e->attr & ATTR_DIRECTORY) != 0;
                if (size) *size = e->fileSize;
                return true;
            }
            dirEntryIndex = 0;
            dirSector++;
        }
        return false;
    }

    // Cluster-chained directory
    while (dirCluster && !isLastCluster(dirCluster)) {
        if (!readDirSector(dirCluster, dirSector, sectorBuf))
            return false;

        while (dirEntryIndex < 16) {
            DirEntry *e = (DirEntry *)(sectorBuf + dirEntryIndex * 32);
            dirEntryIndex++;

            if (e->name[0] == 0x00) return false;
            if (e->name[0] == 0xE5) continue;
            if (e->attr == ATTR_LFN) continue;
            if (e->attr & ATTR_VOLUME_ID) continue;

            rawNameToStr(e->name, name);
            if (isDir) *isDir = (e->attr & ATTR_DIRECTORY) != 0;
            if (size) *size = e->fileSize;
            return true;
        }

        dirEntryIndex = 0;
        dirSector++;
        if (dirSector >= sectorsPerCluster) {
            dirSector = 0;
            dirCluster = nextCluster(dirCluster);
        }
    }
    return false;
}

// --- Write support ---

// Create or truncate a file
Result create(const char *path)
{
    if (!dev) return NOT_READY;

    uint32_t parentClust;
    const char *fname;
    if (!parsePath(path, &parentClust, &fname))
        return DISK_ERR;

    // Handle FAT16 root dir specially
    bool fat16root = (fatType != 32 && parentClust == 0);

    // Check if file already exists
    DirEntry entry;
    uint32_t foundClust = 0, foundSector = 0;
    uint8_t foundIndex = 0;
    bool exists;

    if (fat16root) {
        exists = false;
        for (uint32_t sec = 0; sec < rootDirSectors; sec++) {
            if (!dev->readBlock(firstFATSector + numFATs * sectorsPerFAT + sec, sectorBuf))
                return DISK_ERR;
            for (uint8_t i = 0; i < 16; i++) {
                DirEntry *e = (DirEntry *)(sectorBuf + i * 32);
                if (e->name[0] == 0x00 || e->name[0] == 0xE5) {
                    if (!exists && e->name[0] == 0x00) {
                        foundSector = sec;
                        foundIndex = i;
                        exists = true; // mark as found a free spot
                    }
                } else if (e->attr != ATTR_LFN && nameMatch(e->name, fname)) {
                    // File exists — truncate
                    memcpy(&entry, e, sizeof(DirEntry));
                    exists = true;
                    foundSector = sec;
                    foundIndex = i;
                    break;
                }
            }
        }
        if (!exists) {
            // Need to find a free entry (marked 0xE5 or 0x00)
            exists = false;
            for (uint32_t sec = 0; sec < rootDirSectors; sec++) {
                if (!dev->readBlock(firstFATSector + numFATs * sectorsPerFAT + sec, sectorBuf))
                    return DISK_ERR;
                for (uint8_t i = 0; i < 16; i++) {
                    DirEntry *e = (DirEntry *)(sectorBuf + i * 32);
                    if (e->name[0] == 0x00 || e->name[0] == 0xE5) {
                        foundSector = sec;
                        foundIndex = i;
                        exists = true;
                        break;
                    }
                }
                if (exists) break;
            }
        }
        if (!exists) return DISK_ERR;
    } else {
        exists = findDirEntry(parentClust, fname, &entry, false, &foundClust, &foundSector, &foundIndex);
        if (!exists) {
            // Look for a free slot
            if (!findDirEntry(parentClust, fname, &entry, true, &foundClust, &foundSector, &foundIndex))
                return DISK_ERR;
        }
    }

    // Allocate first cluster
    uint32_t firstClust = 0;
    if (exists && getStartCluster(&entry) != 0) {
        // Free existing cluster chain
        firstClust = getStartCluster(&entry);
        uint32_t c = firstClust;
        while (c && !isLastCluster(c)) {
            uint32_t nxt = nextCluster(c);
            writeFATEntry(c, 0);
            c = nxt;
        }
        if (c) writeFATEntry(c, 0);
    }
    firstClust = allocCluster();
    if (firstClust == 0) return DISK_ERR;

    // Build 8.3 name
    uint8_t rawName[11];
    memset(rawName, ' ', 11);
    const char *dot = strchr(fname, '.');
    int nameLen = dot ? (dot - fname) : strlen(fname);
    if (nameLen > 8) nameLen = 8;
    for (int i = 0; i < nameLen; i++) {
        char c = fname[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        rawName[i] = c;
    }
    if (dot) {
        dot++;
        int extLen = strlen(dot);
        if (extLen > 3) extLen = 3;
        for (int i = 0; i < extLen; i++) {
            char c = dot[i];
            if (c >= 'a' && c <= 'z') c -= 32;
            rawName[8 + i] = c;
        }
    }

    // Write directory entry
    if (fat16root) {
        uint32_t rootSec = firstFATSector + numFATs * sectorsPerFAT + foundSector;
        if (!dev->readBlock(rootSec, sectorBuf))
            return DISK_ERR;
    } else {
        if (!readDirSector(foundClust, foundSector, sectorBuf))
            return DISK_ERR;
    }

    DirEntry *e = (DirEntry *)(sectorBuf + foundIndex * 32);
    memset(e, 0, sizeof(DirEntry));
    memcpy(e->name, rawName, 11);
    e->attr      = ATTR_ARCHIVE;
    e->fstClusLO = firstClust & 0xFFFF;
    e->fstClusHI = (firstClust >> 16) & 0xFFFF;
    e->fileSize  = 0;

    if (fat16root) {
        uint32_t rootSec = firstFATSector + numFATs * sectorsPerFAT + foundSector;
        if (!dev->writeBlock(rootSec, sectorBuf))
            return DISK_ERR;
    } else {
        if (!writeDirSector(foundClust, foundSector, sectorBuf))
            return DISK_ERR;
    }

    // Set up file state for writing
    fileStartCluster = firstClust;
    fileCurCluster   = firstClust;
    fileSize         = 0;
    filePos          = 0;
    fileOpen         = true;
    fileWriting      = true;
    writeCluster     = firstClust;
    writeSectorInCluster = 0;

    return OK;
}

Result write(const void *buf, uint16_t btw, uint16_t *bw)
{
    if (!fileOpen) return NOT_OPENED;
    if (!fileWriting) return NOT_ENABLED;

    const uint8_t *src = (const uint8_t *)buf;
    uint16_t bytesWritten = 0;

    while (bytesWritten < btw) {
        // Read the current sector to preserve partial data
        uint32_t sector = clustToSector(writeCluster) + writeSectorInCluster;
        if (!dev->readBlock(sector, sectorBuf))
            return DISK_ERR;

        uint16_t offset = filePos % bytesPerSector;
        uint16_t remaining = bytesPerSector - offset;
        uint16_t toWrite = btw - bytesWritten;
        if (toWrite > remaining) toWrite = remaining;

        memcpy(sectorBuf + offset, src + bytesWritten, toWrite);

        if (!dev->writeBlock(sector, sectorBuf))
            return DISK_ERR;

        bytesWritten += toWrite;
        filePos += toWrite;
        fileSize = filePos;

        // Move to next sector
        writeSectorInCluster++;
        if (writeSectorInCluster >= sectorsPerCluster) {
            writeSectorInCluster = 0;
            uint32_t next = nextCluster(writeCluster);
            if (isLastCluster(next)) {
                // Allocate new cluster
                next = appendCluster(writeCluster);
                if (next == 0) return DISK_ERR;
            }
            writeCluster = next;
            fileCurCluster = next;
        }
    }

    if (bw) *bw = bytesWritten;
    return OK;
}

} // namespace fat
