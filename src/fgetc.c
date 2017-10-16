#include <stdio.h>
#include "stm32f4xx.h"

int fgetc(FILE *fp)
{
  int ch = 0;

  while(USART_GetFlagStatus(UART4, USART_FLAG_RXNE) == RESET);

  ch = (int)UART4->DR & 0xFF;

  putchar(ch);

  return ch;
}
