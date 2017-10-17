/*
 * CANopen main program file.
 *
 * This file is a template for other microcontrollers.
 *
 * @file        main_generic.c
 * @author      Janez Paternoster
 * @copyright   2004 - 2015 Janez Paternoster
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
#include <stdio.h>

#include "CANopen.h"
#include "CO_eeprom.h"

#include <systick.h>

#define TMR_TASK_INTERVAL   (1000)          /* Interval of tmrTask thread in microseconds */
#define INCREMENT_1MS(var)  (var++)         /* Increment 1ms variable in tmrTask */


static uint32_t canopen_tick;

void systick_init(void);

/* timer thread executes in constant intervals ********************************/
static void tmrTask_thread(void)
{
    if(CO->CANmodule[0]->CANnormal) {
        bool_t syncWas;

        /* Process Sync and read inputs */
        syncWas = CO_process_SYNC_RPDO(CO, TMR_TASK_INTERVAL);

        /* Further I/O or nonblocking application code may go here. */

        /* Write outputs */
        CO_process_TPDO(CO, syncWas, TMR_TASK_INTERVAL);

        /* verify timer overflow */
        if(0) {
            CO_errorReport(CO->em, CO_EM_ISR_TIMER_OVERFLOW, CO_EMC_SOFTWARE_INTERNAL, 0U);
        }
    }
}

static CO_NMT_reset_cmd_t reset = CO_RESET_APP;
static uint32_t timer1msPrevious, timer1msCopy, timer1msDiff;

int CO_main(void)
{
    CO_ReturnError_t err;

    switch (reset) {
        case CO_RESET_APP:
            /* Configure microcontroller. */

            /* initialize EEPROM */
            EEPROM_init();

            /* increase variable each startup. Variable is stored in EEPROM. */
            OD_powerOnCounter++;
            printf("PowerOn count = %d\n", OD_powerOnCounter);
            reset = CO_RESET_COMM;
            break;

        case CO_RESET_COMM:
            /* CANopen communication reset - initialize CANopen objects *******************/


            /* disable CAN and CAN interrupts */

            /* initialize CANopen */
            err = CO_init((int32_t)CAN1, OD_CANNodeID, OD_CANBitRate);
            if(err != CO_ERROR_NO) {
                CO_errorReport(CO->em, CO_EM_MEMORY_ALLOCATION_ERROR, CO_EMC_SOFTWARE_INTERNAL, err);
                while(1);
            }

            // register object dictionary functions to support store and restore
            // of parameters via objects 0x1010 and 0x1011
            CO_eepromRegisterODFunctions(CO);

            /* Configure Timer interrupt function for execution every 1 millisecond */

            /* Configure CAN transmit and receive interrupt */


            /* start CAN */
            CO_CANsetNormalMode(CO->CANmodule[0]);

            reset = CO_RESET_NOT;
        timer1msPrevious = system_tick;
            
            reset = CO_RESET_NOT;        
            break;

        case CO_RESET_NOT:
            /* loop for normal program execution ******************************************/
            timer1msCopy = system_tick;
            timer1msDiff = timer1msCopy - timer1msPrevious;
            timer1msPrevious = timer1msCopy;


            /* CANopen process */
            reset = CO_process(CO, (uint16_t)timer1msDiff, NULL);

            /* Nonblocking application code may go here. */

            /* Process EEPROM */
            CO_EE_process(&ee);
            
            if (time_out(&canopen_tick, 1)) {
                tmrTask_thread();
            }
            break;

        default:
            printf("CanOpenNode reset = %d\n", reset);
            reset = CO_RESET_NOT;
            break;
    }

    return reset;
}

/* CAN interrupt function *****************************************************/
void CAN1_RX0_IRQHandler(void){
    CO_CANinterrupt_Rx(CO->CANmodule[0]);
}

void CAN1_TX_IRQHandler(void)
{
    CO_CANinterrupt_Tx(CO->CANmodule[0]);
}

void CAN1_SCE_IRQHandler(void)
{
    CO_CANinterrupt_Status(CO->CANmodule[0]);
}
