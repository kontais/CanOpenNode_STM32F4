#include <stdio.h>
#include "stm32f4xx.h"

int fputc(int ch, FILE *f)
{
  if (ch == '\n') {
    USART_SendData(UART4, '\r');
    while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);
  }

  USART_SendData(UART4, (uint8_t) ch);
  while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);

  return ch;
}
