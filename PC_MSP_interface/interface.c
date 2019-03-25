/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
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
 * --/COPYRIGHT--*/
/******************************************************************************
 * MSP432 UART - PC Echo with 12MHz BRCLK
 *
 * Description: This demo echoes back characters received via a PC serial port.
 * SMCLK/DCO is used as a clock source and the device is put in LPM0
 * The auto-clock enable feature is used by the eUSCI and SMCLK is turned off
 * when the UART is idle and turned on when a receive edge is detected.
 * Note that level shifter hardware is needed to shift between RS232 and MSP
 * voltage levels.
 *
 *               MSP432P401
 *             -----------------
 *            |                 |
 *            |                 |
 *            |                 |
 *       RST -|     P1.3/UCA0TXD|----> PC (echo)
 *            |                 |
 *            |                 |
 *            |     P1.2/UCA0RXD|<---- PC
 *            |                 |
 *
 *******************************************************************************/
/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

//![Simple UART Config]
/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 9600 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 *http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 */
const eUSCI_UART_Config uartConfig = //baudrate = 230400
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        3,                                     // BRDIV = 78
        4,                                       // UCxBRF = 2
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling before:EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};
//![Simple UART Config]

uint8_t rxdata[1000];
uint_fast8_t received_byte;
char test_char;
int rcount = 0;
int modules_connected_code[8] = {0, 0, 0, 0, 0, 0, 0, 0};

int main(void)
{
    /* Halting WDT  */
    MAP_WDT_A_holdTimer();

    /* Selecting P1.2 and P1.3 in UART mode */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2,
            GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Setting DCO to 12MHz */
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    //![Simple UART Example]
    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A1_BASE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A1_BASE);

    /* Enabling interrupts */
    MAP_UART_enableInterrupt(EUSCI_A1_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA1);
    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_Interrupt_enableMaster();   
    //![Simple UART Example]
    char *data = "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.";
    transmitStringData(data);
//    uint8_t ndata = 128;
//    transmitByte(ndata);

    while(1)
    {
        MAP_PCM_gotoLPM0();
    }
}

uint_fast8_t toAscii(char c){
    char ch = "";
    sprintf(ch, "%d", c);
    return (uint_fast8_t) ch;
}

/* EUSCI A0 UART ISR - Echoes data back to PC host */
void EUSCIA1_IRQHandler(void)
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A1_BASE);

    MAP_UART_clearInterruptFlag(EUSCI_A1_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        received_byte = MAP_UART_receiveData(EUSCI_A1_BASE);
        rxdata[rcount] = received_byte;
    }
    if(received_byte == (uint8_t) 13){
        //if end of line, do something
        decodeInstruction();
        rcount = 0;
    } else {
        rcount++;
    }

}

void decodeInstruction(){
    uint8_t inst = rxdata[0];
    //set configuration
    if(inst == (uint8_t) 128){
        //todo: set configuration locally
        transmitByteData((uint8_t) 255); //send success byte
    } else if(inst == (uint8_t) 129){ //request configuration
        //todo: transmit configuration to application
        transmitByteData((uint8_t) 255);
    } else if(inst == (uint8_t) 130){ //request start
        //todo: locally request start of DAQ modules
        transmitByteData((uint8_t) 255);
    } else if(inst == (uint8_t) 132){ //request number of modules connected
        //todo: verify how many modules are connected
        transmitByteData((uint8_t) 255); //send success byte
        int i;
        for(i = 0; i < 7; i++){
            transmitNByteData((uint8_t) modules_connected_code[i]);
        }
        transmitByteData((uint8_t) modules_connected_code[7]);
    } else if(inst == (uint8_t) 136){ //set visualization module + channels
        //todo: avilitate live passing of the data
        transmitByteData((uint8_t) 255); //send success byte
    } else if(inst == (uint8_t) 255){ //request cancel
        //todo: implement cancel process
        transmitByteData((uint8_t) 255);
    } else { //send a message stating that the function was a mistake
        transmitByteData((uint8_t) 254);
    }
}

void transmitStringData(char *string){
    int i = 0;
    while(*string){
        MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) *string++);
    }
    MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) 0x0D);
    MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) 0x0A);
}

void transmitByteData(uint8_t byte){
    MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) byte);
    MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) 0x0D);
    MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) 0x0A);
}

void transmitNByteData(uint8_t byte){
    MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) byte);
}
