#include "HAL.h"
#include <Arduino.h>
#include <ch32x035.h>

#if HW_VERSION == 2

void spi_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef  SPI_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    // SCK: PA5
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // MISO: PA6
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // MOSI: PA7
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // CS: PA4 (manual control, not hardware NSS)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // CS idle high
    GPIO_SetBits(GPIOA, GPIO_Pin_4);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; // ~187 kHz (safe for SD init)
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SD_SPI_PORT, &SPI_InitStructure);

    SPI_Cmd(SD_SPI_PORT, ENABLE);
}

uint8_t spi_transfer(uint8_t data)
{
    while (SPI_I2S_GetFlagStatus(SD_SPI_PORT, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SD_SPI_PORT, data);
    while (SPI_I2S_GetFlagStatus(SD_SPI_PORT, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SD_SPI_PORT);
}

void spi_cs_low(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
}

void spi_cs_high(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

#endif // HW_VERSION == 2
