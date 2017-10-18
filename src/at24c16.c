/*-----------------------------------------------------------------------
* at24c16.c  -
*
*
*
* Copyright (C) 2017 kontais@aliyun.com
*
*-----------------------------------------------------------------------*/
#include <board.h>

#include "i2c1.h"

#define AT24C16_SLAVE_ID    0xA0
#define AT24C16_PAGESIZE    16

#define EVENT_TIMEOUT       0x1000
#define BUSY_TIMEOUT        (EVENT_TIMEOUT * 10)

uint8_t I2C_WaitEvent(uint32_t event, uint32_t timeout, uint8_t error, uint8_t *flag)
{
    uint32_t i;

    i = 0;
    while (!I2C_CheckEvent(I2C1, event)) {
        if(i++ > timeout) {
            *flag |= error;
            return 1;
        }
    }
    return 0;
}

static void at24c16_acknowledge_polling(uint8_t slave_address)
{
    volatile uint16_t I2C_SR1;
    do {
        I2C_GenerateSTART(I2C1, ENABLE);

        I2C_SR1 = I2C_ReadRegister(I2C1, I2C_Register_SR1);

        I2C_Send7bitAddress(I2C1, slave_address, I2C_Direction_Transmitter);

    } while (!(I2C_ReadRegister(I2C1, I2C_Register_SR1) & I2C_SR1_ADDR));

    I2C_ClearFlag(I2C1, I2C_FLAG_AF);

    I2C_GenerateSTOP(I2C1, ENABLE);
}

uint8_t at24c16_write_byte(uint32_t address, uint8_t value)
{
    uint32_t i;
    uint8_t flag = 0;
    uint8_t page_select;
    uint8_t word_address;
    uint8_t slave_address;

    page_select   = (uint8_t)((address >> 8) & 0x7);        // 3bit segments
    word_address  = (uint8_t)(address & 0xff);              // 8bit rows
    slave_address = AT24C16_SLAVE_ID | (page_select << 1);

    i = 0;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) {         // Busy Check
        if (i++ > BUSY_TIMEOUT) {
            flag |= 0x01;
            break;
        }
    }

    // START
    I2C_GenerateSTART(I2C1, ENABLE);

    // EV5
    I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT, EVENT_TIMEOUT, 0x01, &flag);

    // Salve Addresss
    I2C_Send7bitAddress(I2C1, slave_address, I2C_Direction_Transmitter);

    // EV6
    I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, EVENT_TIMEOUT, 0x02, &flag);

    // Word Address
    I2C_SendData(I2C1, word_address);

    // EV8_2
    I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED, EVENT_TIMEOUT, 0x04, &flag);

    // Data Byte
    I2C_SendData(I2C1, value);

    // EV8_2
    I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED, EVENT_TIMEOUT, 0x08, &flag);

    // STOP
    I2C_GenerateSTOP(I2C1, ENABLE);

    at24c16_acknowledge_polling(slave_address);

    return flag;
}


/* Page Write */
uint8_t at24c16_write_page(uint32_t address, uint8_t *buffer, uint32_t length)
{
    uint8_t flag = 0;

    uint8_t page_select;
    uint8_t word_address;
    uint8_t slave_address;

    page_select   = (uint8_t)((address >> 8) & 0x7);     // 3bit segments
    word_address  = (uint8_t)(address & 0xff);           // 8bit rows
    slave_address = AT24C16_SLAVE_ID | (page_select << 1);

    if (length == 0) {
        return 0;
    }

    // START
    I2C_GenerateSTART(I2C1, ENABLE);

    // EV5
    I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT, EVENT_TIMEOUT, 0x01, &flag);

    // Slave Address
    I2C_Send7bitAddress(I2C1, slave_address, I2C_Direction_Transmitter);

    // EV6
    I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, EVENT_TIMEOUT, 0x02, &flag);

    // Word Address
    I2C_SendData(I2C1, word_address);

    // EV8_2
    I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED, EVENT_TIMEOUT, 0x04, &flag);

    while (length--) {
        // Data Byte
        I2C_SendData(I2C1, *buffer++);

        // EV8_2
        I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED, EVENT_TIMEOUT, 0x08, &flag);
    }

    // STOP
    I2C_GenerateSTOP(I2C1, ENABLE);

    return flag;
}


/* Random Read */
uint8_t at24c16_read(uint32_t address, uint8_t *buffer, uint32_t length)
{
    uint32_t i;
    uint8_t flag = 0;
    uint8_t page_select;
    uint8_t word_address;
    uint8_t slave_address;

    page_select   = (uint8_t)((address >> 8) & 0x7);     // 3bit segments
    word_address  = (uint8_t)(address & 0xff);           // 8bit rows
    slave_address = AT24C16_SLAVE_ID | (page_select << 1);

    if (length == 0) {
        return 0;
    }

    i = 0;
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) {  // Busy Check
        if (i++ > BUSY_TIMEOUT) {
            flag |= 0x01;
            break;
        }
    }

    // ACK Enable
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    // START
    I2C_GenerateSTART(I2C1, ENABLE);

    // EV5
    I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT, EVENT_TIMEOUT, 0x02, &flag);

    // Slave Address
    I2C_Send7bitAddress(I2C1,  slave_address, I2C_Direction_Transmitter);

    // EV6
    I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, EVENT_TIMEOUT, 0x04, &flag);

    // Word Address
    I2C_SendData(I2C1, word_address);

    // EV8_2
    I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED, EVENT_TIMEOUT, 0x08, &flag);

    // START
    I2C_GenerateSTART(I2C1, ENABLE);

    // EV5
    I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT, EVENT_TIMEOUT, 0x10, &flag);

    // Slave Address
    I2C_Send7bitAddress(I2C1, slave_address, I2C_Direction_Receiver);

    // EV6
    I2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, EVENT_TIMEOUT, 0x20, &flag);

    while (length) {
        if (length == 1) {
            // Last Byte, Set No Acknowledge
            I2C_AcknowledgeConfig(I2C1, DISABLE);

            // STOP
            I2C_GenerateSTOP(I2C1, ENABLE);
        }

        // EV7
        I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED, EVENT_TIMEOUT, 0x40, &flag);

        // Data Byte
        *buffer++ = I2C_ReceiveData(I2C1);
        length--;
    }

    // ACK Enable
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return flag;
}

uint8_t at24c16_write(uint32_t address, uint8_t *buffer, uint32_t length)
{
    while (length--) {
        if(at24c16_write_byte(address++, *buffer++)) {
            return 1;
        }
    }
    return 0;
}

uint32_t at24c16_size(void)
{
    return 16 * 1024 / 8;
}

void at24c16_init(void)
{
    i2c1_init();
}
