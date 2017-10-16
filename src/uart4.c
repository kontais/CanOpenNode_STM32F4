/*-----------------------------------------------------------------------
* uart4.c  -
*
*
*
Copyright (C) 2017 kontais@aliyun.com
*
*-----------------------------------------------------------------------*/
#include "stm32f4xx.h"

void uart4_io_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  
  //
  // UART4 Clock
  //
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);

  //
  // PC10 UART4 TX
  //
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //
  // PC11 UART4 RX
  //
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void uart4_init(void)
{
    USART_InitTypeDef USART_InitStructure;

    uart4_io_init();

    USART_DeInit(UART4);
    USART_StructInit(&USART_InitStructure);

    //
    // 115200-8-N-1
    //
    USART_InitStructure.USART_BaudRate   = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits   = USART_StopBits_1;
    USART_InitStructure.USART_Parity     = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(UART4, &USART_InitStructure);

    USART_Cmd(UART4, ENABLE);

    //
    // Read Clear Status Register
    //
    USART_GetFlagStatus(UART4, USART_FLAG_TC);
}
