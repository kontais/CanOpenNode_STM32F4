/*
 * CAN module object for ST STM32F103 microcontroller.
 *
 * @file        CO_driver.c
 * @author      Janez Paternoster
 * @author      Ondrej Netik
 * @author      Vijayendra
 * @author      Jan van Lienden
 * @copyright   2013 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * CANopenNode is free and open source software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Following clarification and special exception to the GNU General Public
 * License is included to the distribution terms of CANopenNode:
 *
 * Linking this library statically or dynamically with other modules is
 * making a combined work based on this library. Thus, the terms and
 * conditions of the GNU General Public License cover the whole combination.
 *
 * As a special exception, the copyright holders of this library give
 * you permission to link this library with independent modules to
 * produce an executable, regardless of the license terms of these
 * independent modules, and to copy and distribute the resulting
 * executable under terms of your choice, provided that you also meet,
 * for each linked independent module, the terms and conditions of the
 * license of that module. An independent module is a module which is
 * not derived from or based on this library. If you modify this
 * library, you may extend this exception to your version of the
 * library, but you are not obliged to do so. If you do not wish
 * to do so, delete this exception statement from your version.
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "CO_driver.h"
#include "bxCAN.h"
#include "CO_Emergency.h"
//#include "led.h"
#include <string.h>

/* Private macro -------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variable ----------------------------------------------------------*/
/* Private function ----------------------------------------------------------*/
static void CO_CANsendToModule(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer, uint8_t transmit_mailbox);

typedef enum{
    CAN_MODE_START,
    CAN_MODE_CONFIG
} can_mode;

void InitCanLeds(void)
{
//    vLED_InitRCC();
//    vLED_InitPort();
}

void CanLedsSet(eCoLeds led)
{
//    if (led & eCoLed_Green)
//        vLED_OnPB14Led();
//    else
//        vLED_OffPB14Led();

//    if (led & eCoLed_Red)
//        vLED_OnPB15Led();
//    else
//        vLED_OffPB15Led();

}

/*******************************************************************************
   Macro and Constants - CAN module registers
 *******************************************************************************/
/**
 * Set mode of CAN controller (configuration, active...)
 * This function properly handles errors when setting mode
 */
static void setCAN_Mode(can_mode mode, CAN_TypeDef * CANbaseAddress)
{
    switch (mode) {
        case CAN_MODE_START:
            CAN_ITConfig(CANbaseAddress, CAN_IT_FMP0 | CAN_IT_TME, ENABLE);
            break;

        case CAN_MODE_CONFIG:
            CAN_ITConfig(CANbaseAddress, (CAN_IT_TME | CAN_IT_FMP0), DISABLE);
            break;

        default:
            break;
    }
}


/******************************************************************************/
void CO_CANsetConfigurationMode(CAN_TypeDef *CANbaseAddress)
{
    setCAN_Mode(CAN_MODE_CONFIG, CANbaseAddress);
}


/******************************************************************************/
void CO_CANsetNormalMode(CO_CANmodule_t *CANmodule)
{
    setCAN_Mode(CAN_MODE_START, CANmodule->CANbaseAddress);

    CANmodule->CANnormal = true;
}

/**
 * Translates CANopen node bitrate into eCos bitrate identifier
 */
uint16_t translateBaudRate(uint16_t CANbitRate)
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

/******************************************************************************/
CO_ReturnError_t CO_CANmodule_init(
        CO_CANmodule_t         *CANmodule,
        CAN_TypeDef            *CANbaseAddress,
        CO_CANrx_t              rxArray[],
        uint16_t                rxSize,
        CO_CANtx_t              txArray[],
        uint16_t                txSize,
        uint16_t                CANbitRate)
{
    int i;

    /* verify arguments */
    if(CANmodule==NULL || rxArray==NULL || txArray==NULL){
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }

    CANmodule->CANbaseAddress = CANbaseAddress;
    CANmodule->rxArray = rxArray;
    CANmodule->rxSize = rxSize;
    CANmodule->txArray = txArray;
    CANmodule->txSize = txSize;
    CANmodule->CANnormal = false;
    CANmodule->useCANrxFilters = false;
    CANmodule->bufferInhibitFlag = 0;
    CANmodule->firstCANtxMessage = 1;
    CANmodule->CANtxCount = 0;
    CANmodule->errOld = 0;
    CANmodule->em = 0;

    for (i = 0; i < rxSize; i++) {
        CANmodule->rxArray[i].ident = 0;
        CANmodule->rxArray[i].pFunct = 0;
    }
    for (i = 0; i < txSize; i++) {
        CANmodule->txArray[i].bufferFull = 0;
    }

    uint8_t result = bxCAN_init(CANbaseAddress, CANbitRate);
    if (result) {
        return CO_ERROR_TIMEOUT;
    }

    return CO_ERROR_NO;
}

/******************************************************************************/
void CO_CANmodule_disable(CO_CANmodule_t *CANmodule)
{
    CAN_DeInit(CANmodule->CANbaseAddress);
}

/******************************************************************************/
CO_ReturnError_t CO_CANrxBufferInit(
        CO_CANmodule_t         *CANmodule,
        uint16_t                index,
        uint16_t                ident,
        uint16_t                mask,
        int8_t                  rtr,
        void                   *object,
        void                  (*pFunct)(void *object, const CO_CANrxMsg_t *message))
{
    CO_CANrx_t *rxBuffer;
    //CanRxMsg *rxBuffer;
    uint16_t RXF, RXM;

    //safety
    if (!CANmodule || !object || !pFunct || index >= CANmodule->rxSize)
    {
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }

    //buffer, which will be configured
    rxBuffer = CANmodule->rxArray + index;

    //Configure object variables
    rxBuffer->object = object;
    rxBuffer->pFunct = pFunct;


    //CAN identifier and CAN mask, bit aligned with CAN module registers
    RXF = (ident & 0x07FF) << 2;
    if (rtr) RXF |= 0x02;
    RXM = (mask & 0x07FF) << 2;
    RXM |= 0x02;

    //configure filter and mask
    if (RXF != rxBuffer->ident || RXM != rxBuffer->mask)
    {
        rxBuffer->ident = RXF;
        rxBuffer->mask = RXM;
    }

    return CO_ERROR_NO;
}

/******************************************************************************/
CO_CANtx_t *CO_CANtxBufferInit(
        CO_CANmodule_t         *CANmodule,
        uint16_t                index,
        uint16_t                ident,
        int8_t                  rtr,
        uint8_t                 noOfBytes,
        int8_t                  syncFlag)
{
    uint32_t TXF;
    CO_CANtx_t *buffer;

    //safety
    if (!CANmodule || CANmodule->txSize <= index) return 0;

    //get specific buffer
    buffer = &CANmodule->txArray[index];

    //CAN identifier, bit aligned with CAN module registers

    TXF = ident << 21;
    TXF &= 0xFFE00000;
    if (rtr) TXF |= 0x02;

    //write to buffer
    buffer->ident = TXF;
    buffer->DLC = noOfBytes;
    buffer->bufferFull = 0;
    buffer->syncFlag = syncFlag ? 1 : 0;

    return buffer;
}

int8_t getFreeTxBuff(CO_CANmodule_t *CANmodule)
{
    uint8_t txBuff = CAN_TXMAILBOX_0;
    
    //if (CAN_TransmitStatus(CANmodule->CANbaseAddress, txBuff) == CAN_TxStatus_Ok)
    for (txBuff = CAN_TXMAILBOX_0; txBuff <= (CAN_TXMAILBOX_2 + 1); txBuff++)
    {
        switch (txBuff)
        {
        case (CAN_TXMAILBOX_0 ):
            if (CANmodule->CANbaseAddress->TSR & CAN_TSR_TME0 )
                return txBuff;
            else
                break;
        case (CAN_TXMAILBOX_1 ):
            if (CANmodule->CANbaseAddress->TSR & CAN_TSR_TME1 )
                return txBuff;
            else
                break;
        case (CAN_TXMAILBOX_2 ):
            if (CANmodule->CANbaseAddress->TSR & CAN_TSR_TME2 )
                return txBuff;
            else
                break;
                default:
                        break;
        }
    }
    return -1;
}

/******************************************************************************/
CO_ReturnError_t CO_CANsend(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer)
{
    CO_ReturnError_t err = CO_ERROR_NO;
    int8_t txBuff;

    /* Verify overflow */
    if(buffer->bufferFull)
    {
        if(!CANmodule->firstCANtxMessage)/* don't set error, if bootup message is still on buffers */
            CO_errorReport((CO_EM_t*)CANmodule->em, CO_EM_CAN_TX_OVERFLOW, CO_EMC_CAN_OVERRUN, 0);
        err = CO_ERROR_TX_OVERFLOW;
    }

    CO_LOCK_CAN_SEND();
    //if CAN TB buffer0 is free, copy message to it
     txBuff = getFreeTxBuff(CANmodule);
   // #error change this - use only one buffer for transmission - see generic driver
    if(txBuff != -1 && CANmodule->CANtxCount == 0)
    {
        CANmodule->bufferInhibitFlag = buffer->syncFlag;
        CO_CANsendToModule(CANmodule, buffer, txBuff);
    }
    //if no buffer is free, message will be sent by interrupt
    else
    {
        buffer->bufferFull = 1;
        CANmodule->CANtxCount++;
        // vsechny buffery jsou plny, musime povolit preruseni od vysilace, odvysilat az v preruseni
        CAN_ITConfig(CANmodule->CANbaseAddress, CAN_IT_TME, ENABLE);
    }
    CO_UNLOCK_CAN_SEND();

    return err;
}

/******************************************************************************/
void CO_CANclearPendingSyncPDOs(CO_CANmodule_t *CANmodule)
{

    /* See generic driver for implemetation. */
}

/******************************************************************************/
void CO_CANverifyErrors(CO_CANmodule_t *CANmodule)
{
   uint32_t err;
   CO_EM_t* em = (CO_EM_t*)CANmodule->em;

   err = CANmodule->CANbaseAddress->ESR;
   // if(CAN_REG(CANmodule->CANbaseAddress, C_INTF) & 4) err |= 0x80;

   if(CANmodule->errOld != err)
   {
      CANmodule->errOld = err;

      //CAN RX bus overflow
      if(CANmodule->CANbaseAddress->RF0R & 0x08)
      {
         CO_errorReport(em, CO_EM_CAN_RXB_OVERFLOW, CO_EMC_CAN_OVERRUN, err);
         CANmodule->CANbaseAddress->RF0R &=~0x08;//clear bits
      }

      //CAN TX bus off
      if(err & 0x04) CO_errorReport(em, CO_EM_CAN_TX_BUS_OFF, CO_EMC_BUS_OFF_RECOVERED, err);
      else           CO_errorReset(em, CO_EM_CAN_TX_BUS_OFF, err);

      //CAN TX or RX bus passive
      if(err & 0x02)
      {
         if(!CANmodule->firstCANtxMessage) CO_errorReport(em, CO_EM_CAN_TX_BUS_PASSIVE, CO_EMC_CAN_PASSIVE, err);
      }
      else
      {
        // int16_t wasCleared;
        /* wasCleared = */CO_errorReset(em, CO_EM_CAN_TX_BUS_PASSIVE, err);
        /* if(wasCleared == 1) */CO_errorReset(em, CO_EM_CAN_TX_OVERFLOW, err);
      }


      //CAN TX or RX bus warning
      if(err & 0x01)
      {
         CO_errorReport(em, CO_EM_CAN_BUS_WARNING, CO_EMC_NO_ERROR, err);
      }
      else
      {
         CO_errorReset(em, CO_EM_CAN_BUS_WARNING, err);
      }
   }
}

/******************************************************************************/
// Interrupt from Receiver
void CO_CANinterrupt_Rx(CO_CANmodule_t *CANmodule)
{
    CanRxMsg      CAN1_RxMsg;

    CAN_Receive(CANmodule->CANbaseAddress, CAN_FilterFIFO0, &CAN1_RxMsg);
    {
            uint16_t index;
            uint8_t msgMatched = 0;
            CO_CANrx_t *msgBuff = CANmodule->rxArray;
            for (index = 0; index < CANmodule->rxSize; index++)
            {
                uint16_t msg = (CAN1_RxMsg.StdId << 2) | (CAN1_RxMsg.RTR ? 2 : 0);
                if (((msg ^ msgBuff->ident) & msgBuff->mask) == 0)
                {
                    msgMatched = 1;
                    break;
                }
                msgBuff++;
            }
            //Call specific function, which will process the message
            if (msgMatched && msgBuff->pFunct)
                msgBuff->pFunct(msgBuff->object, (const CO_CANrxMsg_t *)&CAN1_RxMsg);
        }
}

/******************************************************************************/
// Interrupt from Transeiver
void CO_CANinterrupt_Tx(CO_CANmodule_t *CANmodule)
{

     int8_t txBuff;
    /* Clear interrupt flag */
    CAN_ITConfig(CANmodule->CANbaseAddress, CAN_IT_TME, DISABLE); // Transmit mailbox empty interrupt
    /* First CAN message (bootup) was sent successfully */
    CANmodule->firstCANtxMessage = 0;
    /* clear flag from previous message */
    CANmodule->bufferInhibitFlag = 0;
    /* Are there any new messages waiting to be send */
    if(CANmodule->CANtxCount > 0) {
        uint16_t i;             /* index of transmitting message */

        /* first buffer */
        CO_CANtx_t *buffer = CANmodule->txArray;
        /* search through whole array of pointers to transmit message buffers. */
        for(i = CANmodule->txSize; i > 0; i--) {
            /* if message buffer is full, send it. */
            if(buffer->bufferFull) {
                buffer->bufferFull = 0;
                CANmodule->CANtxCount--;
                txBuff = getFreeTxBuff(CANmodule);
                /* Copy message to CAN buffer */
                CANmodule->bufferInhibitFlag = buffer->syncFlag;
                CO_CANsendToModule(CANmodule, buffer, txBuff);
                break;                      /* exit for loop */
            }
            buffer++;
        }/* end of for loop */

        /* Clear counter if no more messages */
        if(i == 0) CANmodule->CANtxCount = 0;
    }
}

/******************************************************************************/
void CO_CANinterrupt_Status(CO_CANmodule_t *CANmodule)
{
  // status is evalved with pooling
}

/******************************************************************************/
static void CO_CANsendToModule(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer, uint8_t transmit_mailbox)
{

    CanTxMsg      CAN1_TxMsg;
    int i;

    CAN1_TxMsg.IDE = CAN_ID_STD;
    CAN1_TxMsg.DLC = buffer->DLC;
    for (i = 0; i < 8; i++) CAN1_TxMsg.Data[i] = buffer->data[i];
    CAN1_TxMsg.StdId = ((buffer->ident) >> 21);
    CAN1_TxMsg.RTR = CAN_RTR_DATA;

    CAN_Transmit(CANmodule->CANbaseAddress, &CAN1_TxMsg);
    CAN_ITConfig(CANmodule->CANbaseAddress, CAN_IT_TME, ENABLE);
}
