/*-----------------------------------------------------------------------
* at24c16.h -
*
*
*
* Copyright (C) 2017 XCMG Group.
*
*-----------------------------------------------------------------------*/
#ifndef _AT24C16_H_
#define _AT24C16_H_

uint8_t at24c16_write(uint32_t address, uint8_t *buffer, uint32_t length);
uint8_t at24c16_write_page(uint32_t address, uint8_t *buffer, uint32_t length);
uint8_t at24c16_read(uint32_t address, uint8_t *buffer, uint32_t length);
uint8_t at24c16_write_byte(uint32_t address, uint8_t value);
uint32_t at24c16_size(void);

void at24c16_init(void);

#endif  /* _AT24C16_H_ */
