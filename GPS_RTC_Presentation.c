/*
 * GPS_RTC.c
 *
 *  Created on: Mar 27, 2019
 *  Updated on: Apr 10, 2019
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
#include <stdlib.h>

#define MAX_FAILS 5
#define TIMER_PERIOD    0x8000
#define TIME_WAIT 15
#define MAX_RETRIES 3

short timeout = 0;
short timeout_timeout = 0;
short seconds = 0;
short check_format = 0;
short record = 0;
short counter = 0;
char format[6];
short comma_counter = 0;
short compare_value;

/* UART Configuration Parameter.
 * Operates with a 9600 baud rate at 12MHz.
 * These values were calculated using the online calculator that TI provides
 * at:
 *http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
const eUSCI_UART_Config uartConfig = {
EUSCI_A_UART_CLOCKSOURCE_SMCLK,                         // SMCLK Clock Source
        78,                                             // BRDIV = 78
        2,                                              // UCxBRF = 2
        0,                                              // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                         // No Parity
        EUSCI_A_UART_LSB_FIRST,                         // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,                      // One stop bit
        EUSCI_A_UART_MODE,                              // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION   // Oversampling
        };

/* Timer_A UpMode Configuration Parameter.
 * Operates at sweet frequency 32.768kHz,
 * generates 1Hz timer that is used for the wait times. */
const Timer_A_UpModeConfig upConfig = {
TIMER_A_CLOCKSOURCE_ACLK,                               // ACLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,                  // ACLK/1 = 32.768kHz
        TIMER_PERIOD,                                   // 32768 ticks
        TIMER_A_TAIE_INTERRUPT_DISABLE,                 // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,             // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR                                // Clear value
        };

/* Structure to organize GPS information
 * by the desired parameters. */
typedef struct GPSformats
{
    char nMEA_Record[6];                                //NMEA-0183 Message format
    char utc[10];                                       //Current time in Coordinated Universal Time
    char latitude[10];                                  //Current Latitude
    char N_S;                                           //North or South
    char longtitude[11];                                //Current Longitude
    char E_W;                                           //East or West
    short valid_Data;                                   //If incoming data is valid; 1 - valid, 0 - invalid
} GPS;
GPS gps;

/*RTC Configuration
 * Default is UNIX Epoch Time: Thursday, January 1st 1970 00:00:00 AM */
const RTC_C_Calendar defaultTime = {
        0x00,                        //seconds
        0x00,                        //minutes
        0x00,                        //hours
        0x04,                        //Day of the Week
        0x01,                        //Day of the Month
        0x01,                        //Month
        0x7B2                        //Year
        };
RTC_C_Calendar newTime;

/* Display current time via UART in Terminal
 * For Testing and Demonstration Purposes. */
void printRTCCurrentTime(void)
{
    RTC_C_Calendar currTime = RTC_C_getCalendarTime();                  //Gets the current calendar time
    printf(EUSCI_A0_BASE, "%i: ", currTime.hours);                      //Send to terminal the current hour
    printf(EUSCI_A0_BASE, "%i: ", currTime.minutes);                    //Send to terminal the current minute
    printf(EUSCI_A0_BASE, "%i, ", currTime.seconds);                    //Send to terminal the current second
    printf(EUSCI_A0_BASE, "Day of Month: %i, ", currTime.dayOfmonth);   //Send to terminal the current day of the month
    printf(EUSCI_A0_BASE, "Month: %i, ", currTime.month);               //Send to terminal the current month
    printf(EUSCI_A0_BASE, "Year: %i\r\n", currTime.year);               //Send to terminal the current year
}

int main(void)
{
    /* Halting WDT  */
    MAP_WDT_A_holdTimer();
    /* Configuring pins for peripheral/crystal usage */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_PJ,
            GPIO_PIN0 | GPIO_PIN1,
            GPIO_PRIMARY_MODULE_FUNCTION);
    /* Selecting P3.2 and P3.3 in UART mode */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    /* Selecting P1.2 and P1.3 in UART mode */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3,
            GPIO_PRIMARY_MODULE_FUNCTION);
    /* Setting DCO to 12MHz */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);
    /* Setting the external clock frequency */
    CS_setExternalClockSourceFrequency(32000, 48000000);
    /* Starting LFXT in non-bypass mode without a timeout */
    CS_startLFXT(CS_LFXT_DRIVE3);
    /* Configure ACLK */
    MAP_CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
    /* Initializing RTC with default time */
    MAP_RTC_C_initCalendar(&defaultTime, RTC_C_FORMAT_BINARY);
    /* Configuring Timer_A1 for Up Mode */
    MAP_Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);
    /* Configuring UART Modules */
    MAP_UART_initModule(EUSCI_A2_BASE, &uartConfig);
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
    /* Enable UART modules */
    MAP_UART_enableModule(EUSCI_A2_BASE);
    MAP_UART_enableModule(EUSCI_A0_BASE);
    printRTCCurrentTime();
    /* Start RTC Clock */
    MAP_RTC_C_startClock();
    /* Enabling interrupts */
    MAP_Interrupt_enableInterrupt(INT_TA1_0);
    /* Starting the timer */
    MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_Interrupt_enableMaster();
    strcpy(gps.nMEA_Record, "GPGGA");
    gps.valid_Data = 0;
    while (1)
    {
        MAP_PCM_gotoLPM0();
    }
}

/*
 *
 */
void printCoordinates(void)
{
    printf(EUSCI_A0_BASE, "GPS latitude: %s ", gps.latitude);           //Send to terminal the latitude
    printf(EUSCI_A0_BASE, "%c\r\n", gps.N_S);                           //Send to terminal the North/South Indicator
    float degree_buffer = atof(gps.latitude);
    float minute_buffer = atof(&gps.latitude[2]);
    degree_buffer = (degree_buffer - minute_buffer) + (minute_buffer/60);
    if(gps.N_S == 'S')
        degree_buffer *= -1;
    printf(EUSCI_A0_BASE, "latitude: %l\r\n", degree_buffer);           //Send to terminal the latitude in decimal degrees
    printf(EUSCI_A0_BASE, "GPS longitude: %s ", gps.longtitude);        //Send to terminal the longitude
    printf(EUSCI_A0_BASE, "%c\r\n", gps.E_W);                           //Send to terminal the East/West Indicator
    float degree_buffer = atof(gps.longtitude);
    float minute_buffer = atof(&gps.longtitude[2]);
    degree_buffer = (degree_buffer - minute_buffer) + (minute_buffer/60);
    if(gps.E_W == 'W')
        degree_buffer *= -1;
    printf(EUSCI_A0_BASE, "longitude: %l\r\n", degree_buffer);           //Send to terminal the longitude in decimal degrees
    printf(EUSCI_A0_BASE, "Time: %s\r\n", gps.utc);                      //Send to terminal the current time
}
/* RTC ISR
 * Every second the Read Ready Flag will raise,
 * telling us when we can read from the RTC. */
void RTC_C_IRQHandler(void)
{
    uint32_t status;
    status = MAP_RTC_C_getEnabledInterruptStatus();                     //Gets the current status on the interrupts
    MAP_RTC_C_clearInterruptFlag(status);                               //Clears the interrupt flag

    if (status & RTC_C_CLOCK_READ_READY_INTERRUPT)                      //Checks if the read ready interrupt was the raised flag
    {
        printRTCCurrentTime();                                             //Displays current time in terminal
    }
}
/*
 * updateRTC function
 * updates the RTC with the time given to us
 * by the GPS and restarts the RTC with
 * the interrupt to read from it enabled. */
void updateRTC(void)
{
    MAP_RTC_C_holdClock();                                              //Disables RTC functionality to modify registers
    printRTCCurrentTime();                                                 //Displays current time in terminal
    RTC_C_Calendar currTime = RTC_C_getCalendarTime();                  //Gets the current calendar date
    uint_fast8_t hour = (gps.utc[0] - 0x30)*10 + (gps.utc[1] - 0x30);           //constructs decimal value from separate ascii characters
    uint_fast8_t minute = (gps.utc[2] - 0x30)*10 + (gps.utc[3] - 0x30);
    uint_fast8_t second = (gps.utc[4] - 0x30)*10 + (gps.utc[5] - 0x30);
    currTime.hours = hour;                                              //Updates struct hour, minute, and second with GPS acquired time
    currTime.minutes= minute;
    currTime.seconds = second;
    MAP_RTC_C_initCalendar(&currTime, RTC_C_FORMAT_BINARY);             //initializes RTC with updated struct
    MAP_RTC_C_clearInterruptFlag(RTC_C_CLOCK_READ_READY_INTERRUPT);     //Clears Read Ready Interrupt Flag
    MAP_RTC_C_enableInterrupt(RTC_C_CLOCK_READ_READY_INTERRUPT);        //Enables Read Ready Interupt Flag
    MAP_RTC_C_startClock();                                             //Starts Real Time Clock
    MAP_Interrupt_enableInterrupt(INT_RTC_C);
}

/*
 * Timer0_A ISR
 * Counts up to TIME_WAIT seconds to enable GPS UART interrupt
 * and disables itself afterwards.
 */
void TA1_0_IRQHandler(void)
{
    seconds++;                                                          //Increases seconds by one
    printf(EUSCI_A0_BASE, "Time until we check GPS: %i\r\n",
           TIME_WAIT - seconds);                                        //Displays how much time is left before checking
if (seconds >= TIME_WAIT)                                               //Checks if we reached or gone past TIME_WAIT
    {
        MAP_UART_enableInterrupt(EUSCI_A2_BASE,
                                 EUSCI_A_UART_RECEIVE_INTERRUPT);       //Enables GPS UART receive interrupt
        MAP_Interrupt_enableInterrupt(INT_EUSCIA2);                     //Enables interrupts for GPS UART
        MAP_Interrupt_disableInterrupt(INT_TA1_0);                      //Disables timer interrupt
    }
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
    TIMER_A_CAPTURECOMPARE_REGISTER_0);                                 //Clears interrupt for timer to re-enter ISR in one second
}

/* EUSCI A2 GPS UART ISR
 * Executes every time we receive a character from the GPS.
 * Depending on which state we are in, the ISR performs
 * a different action:
 *  a) Wait the Next Message -  Since every message
 *      format starts with the character '$', If we started
 *      to acquire the data or we are not reading the desired
 *      message, wait until we receive '$' before moving on
 *      to the next state.
*   b) Check Message Format - If previous character was a '$'
*       then the next five characters will tell us the format
*       for the current message. If the format is GPGGA, proceed
*       with acquiring the data. If not, return to a.
*   c) Store Incoming Data - If within the desired format,
*       store the next 6 parameters(separated by commas) in
*       separate character arrays. Parameters will arrive
*       in the order: UTC, latitude, N/S indicator, longitude,
*       E/W indicator, Position Fix indicator. Once we have all
*       six, proceed with data validation.
*   d) Check if Data is Valid - Position Fix Indicator is
*       the valid data parameter. If it is one, two,or three the
*       data is valid and we will proceed with updating the RTC.
*       If it is zero, data is invalid and we have to restart from a.
*       */
void EUSCIA2_IRQHandler(void)
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
    MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);
    if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        char receivedChar = MAP_UART_receiveData(EUSCI_A2_BASE);        //Stores the received character
        if (record)                                                     //State C, store incoming data
        {
            printf(EUSCI_A0_BASE, "%c", receivedChar);
            if (receivedChar == ',')
            {
                comma_counter++;                                        //Increase comma_counter, tells us which parameter is next
                counter = 0;
                if (comma_counter == 7)                                 //If comma_counter equals seven, We have stored all of the revelant data
                {
                    comma_counter = 0;
                    if (gps.valid_Data)                                 //Verify if the data is valid
                    {
                        MAP_Interrupt_disableInterrupt(INT_EUSCIA2);
                        printf(EUSCI_A0_BASE, "%s\r\n",
                               "Data is Valid");
                        updateRTC();                                    //Data is valid, update RTC
                    }
                    else                                                //Data is invalid, reset variables to wait for next message
                    {
                        printf(EUSCI_A0_BASE, "%s\r\n",
                               "Data is Invalid");
                        record = 0;
                        check_format = 0;
                        counter = 0;
                        timeout++;                                      //Variable to limit the amount of tries the gps gets to sync
                        if (timeout == MAX_FAILS)                       //If we reach the maximum amount of tries, restart with the timer count
                        {
                            MAP_Interrupt_disableInterrupt(INT_EUSCIA2);
                            timeout_timeout++;                          //Variable to limit the amount of resets the gps has
                            seconds = 0;
                            timeout = 0;
                            if (timeout_timeout == MAX_RETRIES)         //If we reach the maximum amount of tries, stop
                            {
                                printf(EUSCI_A0_BASE, "%s\r\n",
                                       "GPS did not sync after 3 tries.");
                            }
                            else                                        //Reset ISR, activates timer and wait and additional TIME_WAIT seconds
                            {
                                printf(EUSCI_A0_BASE,
                                       "%s\r\n",
                                       "GPS did not sync within 30 seconds. Retrying.");
                                MAP_Interrupt_enableInterrupt(INT_TA1_0);
                            }
                        }
                    }
                }
            }
            else if (receivedChar == ' ')                               //Ignore space characters
            {
                //ignore this
            }
            else
            {
                switch (comma_counter)                                  //Depending in which parameter we are currently at, store in a specific array
                {
                case 1:                                                 //utc
                    gps.utc[counter++] = receivedChar;
                    break;
                case 2:                                                 //latitude
                    gps.latitude[counter++] = receivedChar;
                    break;
                case 3:                                                 //N/S indicator
                    gps.N_S = receivedChar;
                    break;
                case 4:                                                 //longitude
                    gps.longtitude[counter++] = receivedChar;
                    break;
                case 5:                                                 //E/W indicator
                    gps.E_W = receivedChar;
                    break;
                case 6:                                                 //Position Fix Indicator
                    if (receivedChar == '1' || receivedChar == '2'
                            || receivedChar == '3')
                        gps.valid_Data = 1;
                    else
                        gps.valid_Data = 0;
                    break;
                }
            }
        }
        else if (check_format && counter < 5)                           //If statement that handles the verification of the message format
        {
            format[counter++] = receivedChar;                           //Adds character to an array to form the five character format
            printf(EUSCI_A0_BASE, "%c", format[counter - 1]);
            if (counter == 5)                                           //If we have all five characters, compare
            {
                printf(EUSCI_A0_BASE, "\r\n");
                compare_value = strcmp(format, gps.nMEA_Record);        //Compares message format with desired format(GPGGA)
                check_format = 0;
                counter = 0;
                if (!compare_value)                                     //If it is the desired format, start storing the incoming data
                {
                    record = 1;
                }
                else                                                    //Else restart the check
                {
                    record = 0;
                }
            }
        }
        else if (receivedChar == '$' && !check_format)                  //Checks if the character is the start delimiter
        {
            printf(EUSCI_A0_BASE, "%c", receivedChar);
            check_format = 1;                                           //Next five characters is the message format, check those.
        }
    }
}
