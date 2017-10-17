/**
 * Microcontroller specific code for CANopenNode nonvolatile variables.
 *
 * This file is a template for other microcontrollers.
 *
 * @file        CO_eeprom.h
 * @ingroup     CO_eeprom
 * @author      Janez Paternoster
 * @copyright   2004 - 2013 Janez Paternoster
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


#include "CANopen.h"
#include "CO_driver.h"
#include "CO_OD.h"
#include "CO_SDO.h"
#include "CO_Emergency.h"
#include "CO_eeprom.h"
#include "crc16-ccitt.h"

#include "at24c16.h"

#include <stdio.h>

#define PARAM_STORE_PASSWORD    0x65766173UL
#define PARAM_RESTORE_PASSWORD  0x64616F6CUL

#define PARA_AREA_MAIN      0
#define PARA_AREA_DEFAULT   1

CO_EE_t  ee;

enum CO_OD_H1010_StoreParam_Sub
{
    OD_H1010_STORE_PARAM_COUNT,
    OD_H1010_STORE_PARAM_ALL,
    OD_H1010_STORE_PARAM_COMM,
    OD_H1010_STORE_PARAM_APP,
    OD_H1010_STORE_PARAM_MANUFACTURER,
    OD_H1010_STORE_PARAM_RESERVED     = 0x80
};

enum CO_OD_H1011_RestoreDefaultParam_Sub
{
    OD_H1011_RESTORE_PARAM_COUNT,
    OD_H1011_RESTORE_PARAM_ALL,
    OD_H1011_RESTORE_PARAM_COMM,
    OD_H1011_RESTORE_PARAM_APP,
    OD_H1011_RESTORE_PARAM_MANUFACTURER,
    OD_H1011_RESTORE_PARAM_RESERVED     = 0x80
};

/**
 * Store parameters of object dictionary into flash memory.
 * \param[in] FlashAddress Use CO_OD_Flash_Adress for the normal parameter
 *                         block and CO_OD_Flash_Default_Param for the
 *                         default parameters
 */
static CO_SDO_abortCode_t storeParameters(CO_EE_t *pEE, uint8_t ParamArea, uint8_t ParametersSub)
{
    uint32_t Address;
    uint8_t err;

    if (ParamArea == PARA_AREA_MAIN) {
        Address = pEE->ParaOffsetMain;
    } else {
        Address = pEE->ParaOffsetDefault;
    }
    
    err = at24c16_write(Address, pEE->OD_EEPROMAddress, pEE->OD_EEPROMSize);
    
    if (err) {
        return CO_SDO_AB_HW;
    }

    return CO_SDO_AB_NONE;
}


/**
 * Restore parameters of object dictionary from flash memory.
 * \param[in] FlashAddress Use CO_OD_Flash_Adress for the normal parameter
 *                         block and CO_OD_Flash_Default_Param for the
 *                         default parameters
 */
static CO_SDO_abortCode_t restoreParameters(CO_EE_t *pEE, uint8_t ParamArea, uint8_t ParametersSub)
{
    uint32_t Address;
    uint8_t err;

    if (ParamArea == PARA_AREA_MAIN) {
        Address = pEE->ParaOffsetMain;
    } else {
        Address = pEE->ParaOffsetDefault;
    }
    
    err = at24c16_read(Address, pEE->OD_EEPROMAddress, pEE->OD_EEPROMSize);
    
    if (err) {
        return CO_SDO_AB_HW;
    }

    return CO_SDO_AB_NONE;
}

/**
 * OD function for accessing _Store parameters_ (index 0x1010) from SDO server.
 *
 * For more information see file CO_SDO.h.
 */
static CO_SDO_abortCode_t CO_ODF_1010_StoreParam(CO_ODF_arg_t *ODF_arg)
{
    CO_EE_t *pEE;
    uint32_t value;
    CO_SDO_abortCode_t ret = CO_SDO_AB_NONE;

    pEE = (CO_EE_t*) ODF_arg->object;
    value = CO_getUint32(ODF_arg->data);

    if(!ODF_arg->reading){
        /* don't change the old value */
        CO_memcpy(ODF_arg->data, (const uint8_t*)ODF_arg->ODdataStorage, 4U);

        if(ODF_arg->subIndex == OD_H1010_STORE_PARAM_ALL){
            if(value == PARAM_STORE_PASSWORD){
                /* write ee->OD_ROMAddress, ee->OD_ROMSize to eeprom (blocking function) */
                ret = storeParameters(pEE, PARA_AREA_MAIN, ODF_arg->subIndex);
            }
            else{
                ret = CO_SDO_AB_DATA_TRANSF;
            }
        }
    }

    return ret;
}


/**
 * OD function for accessing _Restore default parameters_ (index 0x1011) from SDO server.
 *
 * For more information see file CO_SDO.h.
 */
static CO_SDO_abortCode_t CO_ODF_1011_RestoreParam(CO_ODF_arg_t *ODF_arg)
{
    CO_EE_t *pEE;
    uint32_t value;
    CO_SDO_abortCode_t ret = CO_SDO_AB_NONE;

    pEE = (CO_EE_t*) ODF_arg->object;
    value = CO_getUint32(ODF_arg->data);

    if(!ODF_arg->reading){
        /* don't change the old value */
        CO_memcpy(ODF_arg->data, (const uint8_t*)ODF_arg->ODdataStorage, 4U);

        if(ODF_arg->subIndex >= OD_H1011_RESTORE_PARAM_ALL){
            if(value == PARAM_RESTORE_PASSWORD){
                ret = restoreParameters(pEE, PARA_AREA_DEFAULT, ODF_arg->subIndex);
                if (ret != CO_SDO_AB_NONE) {
                    return ret;
                }
                ret = storeParameters(pEE, PARA_AREA_MAIN, OD_H1011_RESTORE_PARAM_ALL);
            }
            else{
                ret = CO_SDO_AB_DATA_TRANSF;
            }
        }
    }

    return ret;
}


/******************************************************************************/
CO_ReturnError_t CO_EE_init_1(
        CO_EE_t                *ee,
        uint8_t                *OD_EEPROMAddress,
        uint32_t                OD_EEPROMSize,
        uint8_t                *OD_ROMAddress,
        uint32_t                OD_ROMSize)
{
    uint32_t FirstWord, LastWord;

    /* verify arguments */
    if(ee==NULL || OD_EEPROMAddress==NULL || OD_ROMAddress==NULL){
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }

    /* Configure eeprom */
    ee->ParaOffsetMain      = 0x00;     /* EEPROM保存数据 */
    ee->ParaOffsetDefault   = 1024;     /* EEPROM默认数据保存地址 */

    /* configure object variables */
    ee->OD_EEPROMAddress = OD_EEPROMAddress;
    ee->OD_EEPROMSize = OD_EEPROMSize;
    ee->OD_ROMAddress = OD_ROMAddress;
    ee->OD_ROMSize = OD_ROMSize;
    ee->OD_EEPROMCurrentIndex = 0U;
    ee->OD_EEPROMWriteEnable = false;

    /* read the CO_OD_EEPROM from EEPROM, first verify, if data are OK */
    at24c16_read(ee->ParaOffsetDefault, (uint8_t*)&FirstWord, 4);
    at24c16_read(ee->ParaOffsetDefault + ee->OD_EEPROMSize - 4, (uint8_t*)&LastWord, 4);    
    if ((FirstWord != CO_OD_FIRST_LAST_WORD)
      ||(LastWord != CO_OD_FIRST_LAST_WORD)) {
        storeParameters(ee, PARA_AREA_MAIN, OD_H1010_STORE_PARAM_ALL);
        storeParameters(ee, PARA_AREA_DEFAULT, OD_H1010_STORE_PARAM_ALL);
    } else {
        restoreParameters(ee, PARA_AREA_MAIN, OD_H1010_STORE_PARAM_ALL);
    }

    /* read the CO_OD_ROM from EEPROM and verify CRC */
    
    ee->OD_EEPROMWriteEnable = true;
    
    printf("EEPROM used size = %d\n", OD_EEPROMSize);
    if (OD_EEPROMSize > at24c16_size()) {
        printf("ERROR! exceed EEPROM size(max.%d)\n", at24c16_size()); 
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }

    return CO_ERROR_NO;
}

/******************************************************************************/
void CO_EE_process(CO_EE_t *ee){
    if((ee != 0) && (ee->OD_EEPROMWriteEnable)/* && !isWriteInProcess()*/){
        uint32_t i;
        uint8_t RAMdata, eeData;

        /* verify next word */
        if(++ee->OD_EEPROMCurrentIndex == ee->OD_EEPROMSize){
            ee->OD_EEPROMCurrentIndex = 0U;
        }
        i = ee->OD_EEPROMCurrentIndex;

        /* read eeprom */
        RAMdata = ee->OD_EEPROMAddress[i];
        eeData = EE_readByte(i);

        /* if bytes in EEPROM and in RAM are different, then write to EEPROM */
        if(eeData != RAMdata){
            EE_writeByteNoWait(RAMdata, i);
        }
    }
}

void EEPROM_init(void)
{
    at24c16_init();

    CO_EE_init_1(&ee,
        (uint8_t*)&CO_OD_EEPROM,
        sizeof(CO_OD_EEPROM),
        (uint8_t*)&CO_OD_ROM,
        sizeof(CO_OD_ROM));
}

uint8_t EE_readByte(uint32_t address)
{
    uint8_t data;

    at24c16_read(address, &data, 1);

    return data;
}

void EE_writeByteNoWait(uint8_t data, uint32_t address)
{
    at24c16_write_byte(address, data);
}

void CO_eepromRegisterODFunctions(CO_t* CO)
{
    CO_OD_configure(CO->SDO[0], OD_H1010_STORE_PARAM_FUNC, CO_ODF_1010_StoreParam, (void*)&ee, 0, 0);
    CO_OD_configure(CO->SDO[0], OD_H1011_REST_PARAM_FUNC, CO_ODF_1011_RestoreParam, (void*)&ee, 0, 0);
}
