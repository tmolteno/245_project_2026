#include <ch32x035.h>
#include "HAL.h"

#define OLED_I2C_CLKRATE   800000   // I2C bus clock rate (Hz)
#define OLED_I2C_PORT I2C1
#define OLED_I2C_SCL_PIN GPIO_Pin_10
#define OLED_I2C_SDA_PIN GPIO_Pin_11

#define OLED_ADDR         0x3C    // OLED write address (0x3C << 1)

// Init I2C
void i2c_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    I2C_InitTypeDef  I2C_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	/* I2S interface pins */
    GPIO_InitStructure.GPIO_Pin = OLED_I2C_SCL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = OLED_I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    I2C_InitStructure.I2C_ClockSpeed = OLED_I2C_CLKRATE;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitStructure.I2C_OwnAddress1 = 0x77; // Unimportant Init
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(OLED_I2C_PORT, &I2C_InitStructure);

    I2C_Cmd(OLED_I2C_PORT, ENABLE);
    I2C_AcknowledgeConfig(OLED_I2C_PORT, ENABLE);

    while( I2C_GetFlagStatus( OLED_I2C_PORT, I2C_FLAG_BUSY ) != RESET );
}

// Start I2C transmission (addr must contain R/W bit)
//void i2c_start(unsigned char addr) {
void i2c_start() {
    unsigned char addr = OLED_ADDR;
	while( I2C_GetFlagStatus( OLED_I2C_PORT, I2C_FLAG_BUSY ) != RESET );
	I2C_GenerateSTART(OLED_I2C_PORT, ENABLE);
	while(!I2C_CheckEvent(OLED_I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT));

	I2C_Send7bitAddress( OLED_I2C_PORT, addr << 1, I2C_Direction_Transmitter );

  while( !I2C_CheckEvent( OLED_I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) );
}

// Send data byte via I2C bus
void i2c_send_byte(unsigned char data) {
    while (I2C_GetFlagStatus(OLED_I2C_PORT, I2C_FLAG_TXE) == RESET) ;
    I2C_SendData( OLED_I2C_PORT, data );
    while( !I2C_CheckEvent( OLED_I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) );
}

// Stop I2C transmission
void i2c_stop(void) {
    I2C_GenerateSTOP( OLED_I2C_PORT, ENABLE );
}
