/*-----------------------------------------------------------------------
* bxCAN.h  -
*
*
*
* Copyright (C) 2017 kontais@aliyun.com
*
*-----------------------------------------------------------------------*/
#ifndef _BXCAN_H_
#define _BXCAN_H_

uint8_t bxCAN_init(CAN_TypeDef* CANbaseAddress, uint16_t CANbitRate);

#endif  /* _BXCAN_H_ */
