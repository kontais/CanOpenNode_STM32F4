/*-----------------------------------------------------------------------
* i2c1.c  -
*
*
*
* Copyright (C) 2017 kontais@aliyun.com
*
*-----------------------------------------------------------------------*/
#include <board.h>

void i2c1_clock_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
}

void i2c1_io_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);

    GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void i2c1_init(void)
{
    I2C_InitTypeDef  I2C_InitStructure;

    i2c1_clock_init();

    i2c1_io_init();

    I2C_DeInit(I2C1);
    I2C_InitStructure.I2C_Mode        = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle   = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0;
    I2C_InitStructure.I2C_Ack         = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed          = 100000;     // 100kbps

    I2C_Init(I2C1, &I2C_InitStructure);

    I2C_Cmd(I2C1, ENABLE);

    I2C_AcknowledgeConfig(I2C1, ENABLE);
}
