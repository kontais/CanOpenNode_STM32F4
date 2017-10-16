/*-----------------------------------------------------------------------
* systick.c  -
*
*
*
* Copyright (C) 2017 kontais@aliyun.com
*
*-----------------------------------------------------------------------*/
#include "stm32f4xx.h"

volatile uint32_t system_tick = 0;

void systick_init(void)
{
  RCC_ClocksTypeDef RCC_Clocks;

  /* SysTick end of count event each 1ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
}

uint8_t time_out(uint32_t *start, uint32_t interval)
{
    uint32_t current;

    current = system_tick;   // os_tick_get();

    if (current - *start < interval) {
        return 0;
    }

    *start = current;
    return 1;
}

void SysTick_Handler(void)
{
    system_tick++;
}

