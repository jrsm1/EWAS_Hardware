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
/** ============================================================================
 *  @file       MSP_EXP432P401R.h
 *
 *  @brief      MSP_EXP432P401R Board Specific APIs
 *
 *  The MSP_EXP432P401R header file should be included in an application as
 *  follows:
 *  @code
 *  #include <MSP_EXP432P401R.h>
 *  @endcode
 *
 *  ============================================================================
 */
#ifndef __MSP_EXP432P401R_H
#define __MSP_EXP432P401R_H

#ifdef __cplusplus
extern "C" {
#endif

/* LEDs on MSP_EXP432P401R are active high. */
#define MSP_EXP432P401R_GPIO_LED_OFF (0)
#define MSP_EXP432P401R_GPIO_LED_ON  (1)

/*!
 *  @def    MSP_EXP432P401R_GPIOName
 *  @brief  Enum of GPIO names on the MSP_EXP432P401R dev board
 */
typedef enum MSP_EXP432P401R_GPIOName {
    MSP_EXP432P401R_GPIO_S1 = 0,
    MSP_EXP432P401R_GPIO_S2,
    MSP_EXP432P401R_SPI_MASTER_READY,
    MSP_EXP432P401R_SPI_SLAVE_READY,
    MSP_EXP432P401R_GPIO_LED1,
    MSP_EXP432P401R_GPIO_LED_RED,
    /*
     * MSP_EXP432P401R_GPIO_LED_GREEN & MSP_EXP432P401R_GPIO_LED_BLUE are used for
     * PWM examples.  Uncomment the following lines if you would like to control
     * the LEDs with the GPIO driver.
     */
    /* MSP_EXP432P401R_GPIO_LED_GREEN, */
    /* MSP_EXP432P401R_GPIO_LED_BLUE, */

    /*
     * MSP_EXP432P401R_SPI_CS1 is used to control chip select pin for slave1
     * MSP_EXP432P401R_SPI_CS2 is used to control chip select pin for slave2
     */
    MSP_EXP432P401R_SPI_CS1,
    MSP_EXP432P401R_SPI_CS2,

    MSP_EXP432P401R_SDSPI_CS,

    /* Sharp LCD Pins */
    MSP_EXP432P401R_LCD_CS,
    MSP_EXP432P401R_LCD_POWER,
    MSP_EXP432P401R_LCD_ENABLE,

    MSP_EXP432P401R_GPIOCOUNT
} MSP_EXP432P401R_GPIOName;


/*!
 *  @def    MSP_EXP432P401R_SDFatFSName
 *  @brief  Enum of SDFatFS names on the MSP_EXP432P401R dev board
 */
typedef enum MSP_EXP432P401R_SDFatFSName {
    MSP_EXP432P401R_SDFatFS0 = 0,

    MSP_EXP432P401R_SDFatFSCOUNT
} MSP_EXP432P401R_SDFatFSName;

/*!
 *  @def    MSP_EXP432P401R_SDName
 *  @brief  Enum of SD names on the MSP_EXP432P401R dev board
 */
typedef enum MSP_EXP432P401R_SDName {
    MSP_EXP432P401R_SDSPI0 = 0,

    MSP_EXP432P401R_SDCOUNT
} MSP_EXP432P401R_SDName;

/*!
 *  @def    MSP_EXP432P401R_SPIName
 *  @brief  Enum of SPI names on the MSP_EXP432P401R dev board
 */
typedef enum MSP_EXP432P401R_SPIName {
    MSP_EXP432P401R_SPIB0 = 0,
    MSP_EXP432P401R_SPIB2,
    MSP_EXP432P401R_SPIB3,
    MSP_EXP432P401R_SPIB4,

    MSP_EXP432P401R_SPICOUNT
} MSP_EXP432P401R_SPIName;


/*!
 *  @brief  Initialize the general board specific settings
 *
 *  This function initializes the general board specific settings.
 */
extern void MSP_EXP432P401R_initGeneral(void);

#ifdef __cplusplus
}
#endif

#endif /* __MSP_EXP432P401R_H */
