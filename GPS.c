/*
 * GPS.c
 *
 *  Created on: Mar 27, 2019
 *      Author: Christian J. Cancel Ramos
 *
 *               MSP432P401
 *             -----------------
 *            |                 |
 *            |                 |
 *            |                 |
 *       RST -|     P3.3/UCA0TXD|----> Floating
 *            |                 |
 *            |                 |
 *            |     P3.2/UCA0RXD|<---- GPS
 *            |                 |
 *
 *******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "printf.h"

#define MAX_FAILS 10
#define TIMER_PERIOD    0x8000
#define TIME_WAIT 10

/* Character array to store incoming GPS data */
char attempt[10][82];
short col = 0;
short row = 0;
short check_format = 0;
short seconds = 0;
short run_it_twice = 1;

short record = 0;
short counter = 0;
char format[5];
short comma_counter = 0;

/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 9600 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 *http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
const eUSCI_UART_Config uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        78,                                     // BRDIV = 78
        2,                                       // UCxBRF = 2
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};

/* Timer_A UpMode Configuration Parameter */
const Timer_A_UpModeConfig upConfig =
{
        TIMER_A_CLOCKSOURCE_ACLK,              // ACLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,          // ACLK/1 = 32.768kHz
        TIMER_PERIOD,                           // 32768 ticks
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
};

/* Struct for the different GPS formats that have latitude, longitude and UTC */
typedef struct GPSformats{
    char nMEA_Record[5];
    char utc[10];
    char latitude[10];
    char N_S;
    char longtitude[11];
    char E_W;
    short valid_Data;
} GPS;

GPS gps;

int main(void)
{
    /* Halting WDT  */
    MAP_WDT_A_holdTimer();
    strcpy(gps.nMEA_Record, "GPGGA");
    gps.valid_Data = 0;

    /* Selecting P3.2 and P3.3 in UART mode */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    /* Selecting P1.2 and P1.3 in UART mode */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    /* Setting DCO to 12MHz */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    /* Configure ACLK */
    MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    /* Configuring Timer_A1 for Up Mode */
    MAP_Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);

    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A2_BASE, &uartConfig);
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A2_BASE);
    MAP_UART_enableModule(EUSCI_A0_BASE);
    /* Enabling interrupts */
//    MAP_UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
//    MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
    MAP_Interrupt_enableInterrupt(INT_TA1_0);
    /* Starting the timer */
    MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_Interrupt_enableMaster();

    while(1){
        MAP_PCM_gotoLPM0();
    }
}

/*
 * Timer0_A ISR - counts 30 seconds to enable UART interrupt and disables itself
 */
void TA1_0_IRQHandler(void)
{
    seconds++;
    printf(EUSCI_A0_BASE, "Time until we check GPS: %i\r\n", TIME_WAIT - seconds);
    if(seconds == TIME_WAIT)
    {
        MAP_UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
        MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
        MAP_Interrupt_disableInterrupt(INT_TA1_0);
    }
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

/* EUSCI A2 UART ISR - saves character into array */
void EUSCIA2_IRQHandler(void)
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);

    MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        char receivedChar = MAP_UART_receiveData(EUSCI_A2_BASE);
        //Check to see if we are within a valid transmission. If we are not, enter if statement.
        if(record)
        {
            printf(EUSCI_A0_BASE, "%c", receivedChar);
            if(receivedChar == ',')
            {
                comma_counter++;
                counter = 0;
                if(comma_counter == 7)
                {
                    comma_counter = 0;
                    MAP_Interrupt_disableInterrupt(INT_EUSCIA2);
                    if(gps.valid_Data)
                    {
                        printf(EUSCI_A0_BASE, "%s\r\n", "Data is Valid");
                    }
                    else
                    {
                        printf(EUSCI_A0_BASE, "%s\r\n", "Data is Invalid");
                        record = 0;
                        check_format = 0;
                        counter = 0;
                        MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
                    }
                }
            }
            else if(receivedChar == ' ')
            {
                //ignore this
            }
            else
            {
                switch(comma_counter){
                case 1://utc time
                    gps.utc[counter++] = receivedChar;
                    break;
                case 2://latitude
                    gps.latitude[counter++] = receivedChar;
                    break;
                case 3://N/S
                    gps.N_S = receivedChar;
                    break;
                case 4://longitude
                    gps.longtitude[counter++] = receivedChar;
                    break;
                case 5:
                    gps.E_W = receivedChar;
                    break;
                case 6:
                    if(receivedChar == '1')
                        gps.valid_Data = 1;
                    else
                        gps.valid_Data = 0;
                    break;
                }
            }
        }
        else if(check_format && counter<5)
        {
            format[counter++] = receivedChar;
            printf(EUSCI_A0_BASE, "%c", format[counter-1]);
            if(counter == 5)
            {
                //printf(EUSCI_A0_BASE, "\r\n");
                short compare_value = strcmp(format, gps.nMEA_Record);
                if(!compare_value)
                {
                    record = 1;
                    check_format = 0;
                    counter = 0;
                }
                else
                {
                    check_format = 0;
                    counter = 0;
                    record = 0;
                }
            }
        }
        else if(receivedChar == '$' && !check_format)
        {
            printf(EUSCI_A0_BASE, "%c", receivedChar);
            check_format = 1;
            //attempt[row][col++] = receivedChar;
        }
    }

}

