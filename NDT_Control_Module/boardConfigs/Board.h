/*
 * Copyright (c) 2015-2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BOARD_H
#define __BOARD_H

#define Board_MSP_EXP432P401R

#ifdef __cplusplus
extern "C" {
#endif

#include "MSP_EXP432P401R.h"

#define Board_initGeneral           MSP_EXP432P401R_initGeneral


#define Board_GPIO_LED_ON           MSP_EXP432P401R_GPIO_LED_ON
#define Board_GPIO_LED_OFF          MSP_EXP432P401R_GPIO_LED_OFF

#define Board_GPIO_LED0             MSP_EXP432P401R_GPIO_LED1
#define Board_GPIO_LED1             MSP_EXP432P401R_GPIO_LED_RED
/*
 * MSP_EXP432P401R_GPIO_LED_GREEN & MSP_EXP432P401R_GPIO_LED_BLUE are used for
 * PWM examples.  Uncomment the following lines if you would like to control
 * the LEDs with the GPIO driver.
 */
/* #define Board_GPIO_LED2           MSP_EXP432P401R_GPIO_LED_GREEN */
/* #define Board_GPIO_LED3           MSP_EXP432P401R_GPIO_LED_BLUE */

#define Board_GPIO_BUTTON0          MSP_EXP432P401R_GPIO_S1
#define Board_GPIO_BUTTON1          MSP_EXP432P401R_GPIO_S2


#define Board_SD0                   MSP_EXP432P401R_SDSPI0

#define Board_SDFatFS0              MSP_EXP432P401R_SDSPI0

#define Board_SPI0                  MSP_EXP432P401R_SPIB0
#define Board_SPI1                  MSP_EXP432P401R_SPIB2
#define Board_SPI2                  MSP_EXP432P401R_SPIB3
#define Board_SPI3                  MSP_EXP432P401R_SPIB4
#define Board_SPI_CS1               MSP_EXP432P401R_SPI_CS1
#define Board_SPI_CS2               MSP_EXP432P401R_SPI_CS2

#define Board_SPI_MASTER            MSP_EXP432P401R_SPIB3
#define Board_SPI_SLAVE             MSP_EXP432P401R_SPIB3
#define Board_SPI_MASTER_READY      MSP_EXP432P401R_SPI_MASTER_READY
#define Board_SPI_SLAVE_READY       MSP_EXP432P401R_SPI_SLAVE_READY



#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H */
