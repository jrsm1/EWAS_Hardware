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
/* Character array to store incoming GPS data and pointer */
//char GPS_Info[79];
//char GPS_Type[5];
//short counter;
short correct_Message = 0;
char format[5];
//![Simple UART Config]
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

typedef struct {
	short start_delimiter = 0;
	char nMEA_Record[5];
	char transmission[76];
	char utc[10];
	char latitude[10];
	char N_S;
	char longtitude[11];
	char E_W;
	short valid_Data = 0;
}

//![Simple UART Config]

int main(void)
{
    /* Halting WDT  */
    MAP_WDT_A_holdTimer();
	
    /* Selecting P3.2 and P3.3 in UART mode */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
            GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Setting DCO to 12MHz */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A2_BASE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A2_BASE);

    /* Enabling interrupts */
    MAP_UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_Interrupt_enableMaster();
	
	GPS gps;
	gps.nMEA_Record = "GPGGA";
    counter = 0;
	
    while(1)
    {
        MAP_PCM_gotoLPM0();
    }
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
        if(!gps.start_delimiter)
		{
			//Check if we are about to receive a valid transmission. If not, ignore character.
			if(receivedChar == '$')
			{
				gps.start_delimiter = 1;
			}
			//else{ /* ignore character */ }
		}
		//If we are in a valid transmission and within the desired format
		else if(correct_Message)
		{
			//Check if we reached the end of the message
			if(receivedChar == 0x0D)
			{
				if(gps.valid_Data)
				{
					MAP_Interrupt_disableInterrupt(INT_EUSCIA2);
					//TODO fill struct with the necessary bits
				}
				else
				{
					//TODO Reset everything this ain't valid yet
					//TODO LIMIT THIS INCASE WE ARE IN A NO SIGNAL ZONE
				}
			}
			//Still in the middle of receiving the information
			else
			{
				//TODO Fill out gps.transmission, increase counter
				//TODO count commas incase the data is invalid and the only way to locate data fix value(gps.valid_Data) is by commas
			}
		}
		//Then we need to check if this transmission is the desired format
		else
		{
			//Check to see if we are verifying which valid transmission we are in.
			if(counter<5)
			{
				//Add character to string
				format[counter++] = receivedChar;
				//If string is full and we are in the desired GPS format, raise flag.
				if(counter == 5 && !strcmp(gps.nMEA_Record, format))
				{
					correct_Message = 1;
				}
				//If string is full and we are not within the desired format, reset counters and wait for next message.
				else if(strcmp(gps.nMEA_Record, format))
				{
					counter = 0;
					gps.start_delimiter = 0;
				}
			}
			//else{/* How did we get here? */ }
		}
    }

}

