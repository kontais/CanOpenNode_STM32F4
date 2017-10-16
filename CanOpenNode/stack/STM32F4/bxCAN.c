/*-----------------------------------------------------------------------
* bxCAN.c  -
*
*
*
* Copyright (C) 2017 kontais@aliyun.com
*
*-----------------------------------------------------------------------*/
#include "stm32f4xx.h"

#define CAN_KBAUD_10    300
#define CAN_KBAUD_20    150
#define CAN_KBAUD_50    60
#define CAN_KBAUD_100   30
#define CAN_KBAUD_125   24
#define CAN_KBAUD_250   12
#define CAN_KBAUD_500   6
//#define CAN_KBAUD_800   // 暂时不支持
#define CAN_KBAUD_1000  3

static uint16_t translateBaudRate(uint16_t CANbitRate)
{
    switch (CANbitRate) {
        case 10:  return CAN_KBAUD_10;
        case 20:  return CAN_KBAUD_20;
        case 50:  return CAN_KBAUD_50;
        case 100: return CAN_KBAUD_100;
        case 125: return CAN_KBAUD_125;
        case 250: return CAN_KBAUD_250;
        case 500: return CAN_KBAUD_500;
        case 1000: return CAN_KBAUD_1000;
    }

    return CAN_KBAUD_125;
}

uint8_t bxCAN_init(CAN_TypeDef* CANbaseAddress, uint16_t CANbitRate)
{
    uint8_t                 result;
    GPIO_InitTypeDef        GPIO_InitStructure;
    CAN_InitTypeDef         CAN_InitStructure;
    CAN_FilterInitTypeDef   CAN_FilterInitStructure;
    NVIC_InitTypeDef        NVIC_InitStructure;

    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    /* Connect CAN pins to AF9 */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_CAN1); /* PD0/CAN1_RX */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_CAN1); /* PD1/CAN1_TX */

    /* Configure CAN RX and TX pins */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* CAN configuration ********************************************************/
    /* Enable CAN clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

    /* CAN register init */
    CAN_DeInit(CANbaseAddress);

    /* CAN cell init */
    CAN_InitStructure.CAN_TTCM = DISABLE;     /* 时间触发 */
    CAN_InitStructure.CAN_ABOM = DISABLE;     /* 硬件自动总线关闭 */
    CAN_InitStructure.CAN_AWUM = DISABLE;     /* 硬件自动唤醒 */
    CAN_InitStructure.CAN_NART = DISABLE;     /* 硬件自动重传 */
    CAN_InitStructure.CAN_RFLM = DISABLE;     /* 接收FIFO锁模式 */
    CAN_InitStructure.CAN_TXFP = DISABLE;     /* 发送FIFO顺序优先 */
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal; /* 正常工作模式 */

    /* CAN Baudrate init(CAN clocked at APB1 = 48MHz) */
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;
    CAN_InitStructure.CAN_Prescaler = translateBaudRate(CANbitRate);    /* 24.7.7 */
    /* BaudRate = (APB1 / Prescaler / (SJW + BS1 + 1 + BS2 + 1) */

    /* Initializes the CAN */
    result = CAN_Init(CANbaseAddress, &CAN_InitStructure);
    if (result != CAN_InitStatus_Success) {
        return 1;
    }
    

    /* CAN filter init */
    CAN_FilterInitStructure.CAN_FilterNumber         = 0;   /* 选择过滤器组 */
    CAN_FilterInitStructure.CAN_FilterMode           = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale          = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh         = 0x0000;    /* 24.7.4 Identifier filtering */
    CAN_FilterInitStructure.CAN_FilterIdLow          = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh     = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow      = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;   /* FIFO选择 */
    CAN_FilterInitStructure.CAN_FilterActivation     = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);

    /* CAN IRQ init */
    NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

//    /* Enable FIFO 0 message pending Interrupt */
//    CAN_ITConfig(CANbaseAddress, CAN_IT_FMP0, ENABLE);

    return 0;
}

