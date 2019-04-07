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

/*
 *  ======== MSP_EXP432P401R.c ========
 *  This file is responsible for setting up the board specific items for the
 *  MSP_EXP432P401R board.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef DeviceFamily_MSP432P401x
#define DeviceFamily_MSP432P401x
#endif

#include <ti/devices/DeviceFamily.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerMSP432.h>

#include <ti/devices/msp432p4xx/inc/msp.h>
#include <ti/devices/msp432p4xx/driverlib/rom.h>
#include <ti/devices/msp432p4xx/driverlib/rom_map.h>
#include <ti/devices/msp432p4xx/driverlib/adc14.h>
#include <ti/devices/msp432p4xx/driverlib/dma.h>
#include <ti/devices/msp432p4xx/driverlib/gpio.h>
#include <ti/devices/msp432p4xx/driverlib/i2c.h>
#include <ti/devices/msp432p4xx/driverlib/interrupt.h>
#include <ti/devices/msp432p4xx/driverlib/pmap.h>
#include <ti/devices/msp432p4xx/driverlib/ref_a.h>
#include <ti/devices/msp432p4xx/driverlib/spi.h>
#include <ti/devices/msp432p4xx/driverlib/timer_a.h>
#include <ti/devices/msp432p4xx/driverlib/timer32.h>
#include <ti/devices/msp432p4xx/driverlib/uart.h>
#include <ti/devices/msp432p4xx/driverlib/wdt_a.h>

#include "MSP_EXP432P401R.h"

/*
 *  ======== MSP_EXP432P401R_initGeneral ========
 */
void MSP_EXP432P401R_initGeneral(void)
{
    Power_init();
}


/*
 *  =============================== DMA ===============================
 */
#include <ti/drivers/dma/UDMAMSP432.h>

#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(dmaControlTable, 256)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=256
#elif defined(__GNUC__)
__attribute__ ((aligned (256)))
#endif
static DMA_ControlTable dmaControlTable[16];

/*
 *  ======== dmaErrorHwi ========
 *  This is the handler for the uDMA error interrupt.
 */
static void dmaErrorHwi(uintptr_t arg)
{
    int status = MAP_DMA_getErrorStatus();
    MAP_DMA_clearErrorStatus();

    /* Suppress unused variable warning */
    (void)status;

    while (1);
}

UDMAMSP432_Object udmaMSP432Object;

const UDMAMSP432_HWAttrs udmaMSP432HWAttrs = {
    .controlBaseAddr = (void *)dmaControlTable,
    .dmaErrorFxn = (UDMAMSP432_ErrorFxn)dmaErrorHwi,
    .intNum = INT_DMA_ERR,
    .intPriority = (~0)
};

const UDMAMSP432_Config UDMAMSP432_config = {
    .object = &udmaMSP432Object,
    .hwAttrs = &udmaMSP432HWAttrs
};




/*
 *  =============================== GPIO ===============================
 */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOMSP432.h>

/*
 * Array of Pin configurations
 * NOTE: The order of the pin configurations must coincide with what was
 *       defined in MSP_EXP432P401R.h
 * NOTE: Pins not used for interrupts should be placed at the end of the
 *       array.  Callback entries can be omitted from callbacks array to
 *       reduce memory usage.
 */
GPIO_PinConfig gpioPinConfigs[] = {
    /* Input pins */
    /*
     * NOTE: Specifying FALLING edge triggering for these buttons to ensure the
     * interrupts are signaled immediately.  See the description of the
     * PowerMSP432 driver's automatic pin parking feature for this rationale.
     */
    /* MSP_EXP432P401R_GPIO_S1 */
    GPIOMSP432_P1_1 | GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING,
    /* MSP_EXP432P401R_GPIO_S2 */
    GPIOMSP432_P1_4 | GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING,

    /* MSP_EXP432P401R_SPI_MASTER_READY */
    GPIOMSP432_P5_7 | GPIO_DO_NOT_CONFIG,
    /* MSP_EXP432P401R_SPI_SLAVE_READY */
    GPIOMSP432_P6_0 | GPIO_DO_NOT_CONFIG,

    /* Output pins */
    /* MSP_EXP432P401R_GPIO_LED1 */
    GPIOMSP432_P1_0 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_LOW | GPIO_CFG_OUT_LOW,
    /* MSP_EXP432P401R_GPIO_LED_RED */
    GPIOMSP432_P2_0 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,

    /*
     * MSP_EXP432P401R_GPIO_LED_GREEN & MSP_EXP432P401R_GPIO_LED_BLUE are used for
     * PWM examples.  Uncomment the following lines if you would like to control
     * the LEDs with the GPIO driver.
     */
    /* MSP_EXP432P401R_GPIO_LED_GREEN */
    /* GPIOMSP432_P2_1 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW, */
    /* MSP_EXP432P401R_GPIO_LED_BLUE */
    /* GPIOMSP432_P2_2 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW */
    /* MSP_EXP432P401R_SPI_CS1 */
    GPIOMSP432_P5_4 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_LOW | GPIO_CFG_OUT_HIGH,
    /* MSP_EXP432P401R_SPI_CS2 */
    GPIOMSP432_P5_5 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_LOW | GPIO_CFG_OUT_HIGH,

    /* MSP_EXP432P401R_SDSPI_CS */
    GPIOMSP432_P4_6 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_LOW | GPIO_CFG_OUT_HIGH,

    /* Sharp Display - GPIO configurations will be done in the Display files */
    GPIOMSP432_P4_3 | GPIO_DO_NOT_CONFIG, /* SPI chip select */
    GPIOMSP432_P4_1 | GPIO_DO_NOT_CONFIG, /* LCD power control */
    GPIOMSP432_P6_0 | GPIO_DO_NOT_CONFIG, /*LCD enable */
};

/*
 * Array of callback function pointers
 * NOTE: The order of the pin configurations must coincide with what was
 *       defined in MSP_EXP432P401R.h
 * NOTE: Pins not used for interrupts can be omitted from callbacks array to
 *       reduce memory usage (if placed at end of gpioPinConfigs array).
 */
GPIO_CallbackFxn gpioCallbackFunctions[] = {
    /* MSP_EXP432P401R_GPIO_S1 */
    NULL,
    /* MSP_EXP432P401R_GPIO_S2 */
    NULL,
    /* MSP_EXP432P401R_SPI_MASTER_READY */
    NULL,
    /* MSP_EXP432P401R_SPI_SLAVE_READY */
    NULL
};

const GPIOMSP432_Config GPIOMSP432_config = {
    .pinConfigs = (GPIO_PinConfig *)gpioPinConfigs,
    .callbacks = (GPIO_CallbackFxn *)gpioCallbackFunctions,
    .numberOfPinConfigs = sizeof(gpioPinConfigs)/sizeof(GPIO_PinConfig),
    .numberOfCallbacks = sizeof(gpioCallbackFunctions)/sizeof(GPIO_CallbackFxn),
    .intPriority = (~0)
};


/*
 *  =============================== Power ===============================
 */
const PowerMSP432_ConfigV1 PowerMSP432_config = {
    .policyInitFxn = &PowerMSP432_initPolicy,
    .policyFxn = &PowerMSP432_sleepPolicy,
    .initialPerfLevel = 2,
    .enablePolicy = true,
    .enablePerf = true,
    .enableParking = true
};


/*
 *  =============================== SDFatFS ===============================
 */
#include <ti/drivers/SD.h>
#include <ti/drivers/SDFatFS.h>

/*
 * Note: The SDFatFS driver provides interface functions to enable FatFs
 * but relies on the SD driver to communicate with SD cards.  Opening a
 * SDFatFs driver instance will internally try to open a SD driver instance
 * reusing the same index number (opening SDFatFs driver at index 0 will try to
 * open SD driver at index 0).  This requires that all SDFatFs driver instances
 * have an accompanying SD driver instance defined with the same index.  It is
 * acceptable to have more SD driver instances than SDFatFs driver instances
 * but the opposite is not supported & the SDFatFs will fail to open.
 */
SDFatFS_Object sdfatfsObjects[MSP_EXP432P401R_SDFatFSCOUNT];

const SDFatFS_Config SDFatFS_config[MSP_EXP432P401R_SDFatFSCOUNT] = {
    {
        .object = &sdfatfsObjects[MSP_EXP432P401R_SDFatFS0]
    }
};

const uint_least8_t SDFatFS_count = MSP_EXP432P401R_SDFatFSCOUNT;

/*
 *  =============================== SD ===============================
 */
#include <ti/drivers/SD.h>
#include <ti/drivers/sd/SDSPI.h>

SDSPI_Object sdspiObjects[MSP_EXP432P401R_SDCOUNT];

const SDSPI_HWAttrs sdspiHWAttrs[MSP_EXP432P401R_SDCOUNT] = {
    {
        .spiIndex = MSP_EXP432P401R_SPIB0,
        .spiCsGpioIndex = MSP_EXP432P401R_SDSPI_CS
    }
};

const SD_Config SD_config[MSP_EXP432P401R_SDCOUNT] = {
    {
        .fxnTablePtr = &SDSPI_fxnTable,
        .object = &sdspiObjects[MSP_EXP432P401R_SDSPI0],
        .hwAttrs = &sdspiHWAttrs[MSP_EXP432P401R_SDSPI0]
    },
};

const uint_least8_t SD_count = MSP_EXP432P401R_SDCOUNT;

/*
 *  =============================== SPI ===============================
 */
#include <ti/drivers/SPI.h>
#include <ti/drivers/spi/SPIMSP432DMA.h>

SPIMSP432DMA_Object spiMSP432DMAObjects[MSP_EXP432P401R_SPICOUNT];

/*
 * NOTE: The SPI instances below can be used by the SD driver to communicate
 * with a SD card via SPI.  The 'defaultTxBufValue' fields below are set to 0xFF
 * to satisfy the SDSPI driver requirement.
 */
const SPIMSP432DMA_HWAttrsV1 spiMSP432DMAHWAttrs[MSP_EXP432P401R_SPICOUNT] = {
    {
        .baseAddr = EUSCI_B0_BASE,
        .bitOrder = EUSCI_B_SPI_MSB_FIRST,
        .clockSource = EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
        .defaultTxBufValue = 0xFF,
        .dmaIntNum = INT_DMA_INT1,
        .intPriority = (~0),
        .rxDMAChannelIndex = DMA_CH1_EUSCIB0RX0,
        .txDMAChannelIndex = DMA_CH0_EUSCIB0TX0,
        .clkPin  = SPIMSP432DMA_P1_5_UCB0CLK,
        .simoPin = SPIMSP432DMA_P1_6_UCB0SIMO,
        .somiPin = SPIMSP432DMA_P1_7_UCB0SOMI,
        .stePin  = SPIMSP432DMA_P1_4_UCB0STE,
        .pinMode  = EUSCI_SPI_3PIN,
        .minDmaTransferSize = 10
    },
    {
        .baseAddr = EUSCI_B2_BASE,
        .bitOrder = EUSCI_B_SPI_MSB_FIRST,
        .clockSource = EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
        .defaultTxBufValue = 0xFF,
        .dmaIntNum = INT_DMA_INT2,
        .intPriority = (~0),
        .rxDMAChannelIndex = DMA_CH5_EUSCIB2RX0,
        .txDMAChannelIndex = DMA_CH4_EUSCIB2TX0,
        .clkPin  = SPIMSP432DMA_P3_5_UCB2CLK,
        .simoPin = SPIMSP432DMA_P3_6_UCB2SIMO,
        .somiPin = SPIMSP432DMA_P3_7_UCB2SOMI,
        .stePin  = SPIMSP432DMA_P3_4_UCB2STE,
        .pinMode  = EUSCI_SPI_3PIN,
        .minDmaTransferSize = 10
    },
    {
        .baseAddr = EUSCI_A1_BASE,
        .bitOrder = EUSCI_A_SPI_MSB_FIRST,
        .clockSource = EUSCI_A_SPI_CLOCKSOURCE_SMCLK,
        .defaultTxBufValue = 0xFF,
        .dmaIntNum = INT_DMA_INT2,
        .intPriority = (~0),
        .rxDMAChannelIndex = DMA_CH3_EUSCIA1RX,
        .txDMAChannelIndex = DMA_CH2_EUSCIA1TX,
        .clkPin  = SPIMSP432DMA_P2_5_UCA1CLK,
        .simoPin = SPIMSP432DMA_P2_6_UCA1SIMO,
        .somiPin = SPIMSP432DMA_P2_7_UCA1SOMI,
        .stePin  = SPIMSP432DMA_P2_3_UCA1STE,
        .pinMode  = EUSCI_SPI_4PIN_UCxSTE_ACTIVE_LOW,
        .minDmaTransferSize = 10
    },
    {
        .baseAddr = EUSCI_B2_BASE,
        .bitOrder = EUSCI_B_SPI_MSB_FIRST,
        .clockSource = EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
        .defaultTxBufValue = 0xFF,
        .dmaIntNum = INT_DMA_INT3,
        .intPriority = (~0),
        .rxDMAChannelIndex = DMA_CH5_EUSCIB2RX0,
        .txDMAChannelIndex = DMA_CH4_EUSCIB2TX0,
        .clkPin  = SPIMSP432DMA_P3_5_UCB2CLK,
        .simoPin = SPIMSP432DMA_P3_6_UCB2SIMO,
        .somiPin = SPIMSP432DMA_P3_7_UCB2SOMI,
        .stePin  = SPIMSP432DMA_P2_4_UCB2STE,
        .pinMode  = EUSCI_SPI_4PIN_UCxSTE_ACTIVE_LOW,
        .minDmaTransferSize = 10
    }
};

const SPI_Config SPI_config[MSP_EXP432P401R_SPICOUNT] = {
    {
        .fxnTablePtr = &SPIMSP432DMA_fxnTable,
        .object = &spiMSP432DMAObjects[MSP_EXP432P401R_SPIB0],
        .hwAttrs = &spiMSP432DMAHWAttrs[MSP_EXP432P401R_SPIB0]
    },
    {
        .fxnTablePtr = &SPIMSP432DMA_fxnTable,
        .object = &spiMSP432DMAObjects[MSP_EXP432P401R_SPIB2],
        .hwAttrs = &spiMSP432DMAHWAttrs[MSP_EXP432P401R_SPIB2]
    },
    {
        .fxnTablePtr = &SPIMSP432DMA_fxnTable,
        .object = &spiMSP432DMAObjects[MSP_EXP432P401R_SPIB3],
        .hwAttrs = &spiMSP432DMAHWAttrs[MSP_EXP432P401R_SPIB3]
    },
    {
        .fxnTablePtr = &SPIMSP432DMA_fxnTable,
        .object = &spiMSP432DMAObjects[MSP_EXP432P401R_SPIB4],
        .hwAttrs = &spiMSP432DMAHWAttrs[MSP_EXP432P401R_SPIB4]
    }
};

const uint_least8_t SPI_count = MSP_EXP432P401R_SPICOUNT;
