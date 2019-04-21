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
char configuration[1000];
int conf_place =  0;
char *all_data = "this is all the data";
int live1[3] = {1, 1, 1};
int live2[3] = {1, 1, 1};
char *gps_data = "gps data";
int valid_conf = 0;
int trans_count = 0;
int status = 1;
int diagnosis = 1;

// module sensor visualization
int vis_mod1 = 0;
int vis_sens1 = 0;
int vis_mod2 = 0;
int vis_sens2 = 0;

//buffer for live data
int live_buffer[100];

// recording parameters
int sample_rate;
int cutoff;
int gain;
int duration;
int start_delay;

//experiment name
char experiment_name[20];

//localization identifier
char localization_name[20];

//testers
int test1;
int test_count;

//status variables
uint8_t recorded = 1;
uint8_t stored = 0;
uint8_t gps_synched = 1;

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

/**
 * Decode instruction funciton
 * uses the global variable as @rxdata the received data.
 * the first char byte in the rxdata array is the instruction
 *
 * for
 */
void decodeInstruction(){
    uint8_t inst = rxdata[0];
    //set configuration
    if(inst == (uint8_t) 128){ //x80
        //todo: set configuration locally
        transmitByteData((uint8_t) 128); //send success byte
        int i = 1;
        conf_place = 0;
        while(rcount > 1){
            configuration[i-1] = rxdata[i];
            rcount--;
            i++;
            conf_place++;
        }
        i = 0;
    } else if(inst == (uint8_t) 129){ //request configuration x81
        //todo: transmit configuration to application
        transmitByteData((uint8_t) 129);
        int temp = conf_place;
        int i = 0;
        while(conf_place > 1){//this is a place holder, but eventually it should transm
            transmitNByteData(configuration[i]);
            i++;
            conf_place--;
        }
        transmitByteData(configuration[i]);
        conf_place = temp;
        i = 0;                                   //it the requested configuration data.
    } else if(inst == (uint8_t) 130){ //request start x82
        //todo: locally request start of DAQ modules
        transmitByteData((uint8_t) 130);
    } else if(inst == (uint8_t) 131){//request control module status
        transmitByteData((uint8_t) 131); //acknowledge
        transmitNByteData(recorded);
        transmitNByteData(stored);
        transmitByteData(gps_synched);

    } else if(inst == (uint8_t) 132){ //request number of modules connected x84
        //todo: verify how many modules are connected
        transmitByteData((uint8_t) 132); //send success byte
        int i;
        for(i = 0; i < 7; i++){
            transmitNByteData((uint8_t) modules_connected_code[i]);
        }
        transmitByteData((uint8_t) modules_connected_code[7]);
    } else if(inst == (uint8_t) 133){ //instruction to receive recording parameters
        transmitByteData((uint8_t) 133);

        sample_rate = (uint8_t) rxdata[1];
        cutoff = (uint8_t) rxdata[2];
        gain = (uint8_t) rxdata[3];

        duration = ((uint8_t)rxdata[4])*1000 + ((uint8_t)rxdata[5])*100 + ((uint8_t)rxdata[6])*10 + ((uint8_t)rxdata[7]);

        start_delay = ((uint8_t)rxdata[8])*1000 + ((uint8_t)rxdata[9])*100 + ((uint8_t)rxdata[10])*10 + ((uint8_t)rxdata[11]);

        vis_mod1 = (uint8_t)rxdata[12];
        vis_sens1 = (uint8_t)rxdata[13];
        vis_mod2 = (uint8_t)rxdata[14];
        vis_sens2 = (uint8_t)rxdata[15];

        int hold_initial = 16;
        char a = rxdata[hold_initial];
        int count = 0;

        test1 = (int)(a!=',');
        test1 = (int)(a!=",");

        while(a != ',' && count < 20){
            experiment_name[count] = a;
            a = rxdata[count + hold_initial + 1];
            count++;
        }

        //treating edge case of being sent blanks
        if(count == 0) count ++;

        int hold_place = count + hold_initial + 1;
        test_count = hold_place;
        count = 0;

        a = rxdata[hold_place];

        while(a != ',' && count < 20 ){
            localization_name[count] = a;
            a = rxdata[hold_place + count + 1];
            count++;
        }
        count = 0;

    } else if(inst == (uint8_t) 134){ //instruction to send all the data last acquired
        transmitByteData((uint8_t) 134); //send success byte
        //code to send all the data
        transmitStringData(all_data);
        //end of data transmission
        transmitNByteData((uint8_t) 255);
        transmitNByteData((uint8_t) 255);
        transmitNByteData((uint8_t) 255);
        transmitNByteData((uint8_t) 255);
        transmitNByteData((uint8_t) 255);
        transmitByteData((uint8_t) 255);
    } else if(inst == (uint8_t) 136){ //set visualization module + channels x88
        //todo: avilitate live passing of the data
        transmitByteData((uint8_t) 136); //send success byte
        transmitNByteData(live1[0]);
        transmitNByteData(live1[1]);
        transmitNByteData(live1[2]);
        transmitNByteData(live2[0]);
        transmitNByteData(live2[1]);
        transmitByteData(live2[2]);
    } else if(inst == (uint8_t) 141){ //send the live bytes to the application x87
        transmitByteData((uint8_t) 141);
        //send the live bytes. we should do a buffer array

        //end of live stream by sending 255 255 255 255 255 255 \r\n

    } else if(inst == (uint8_t) 137){ //send gps data request x89
        transmitByteData((uint8_t) 137); //acknowledge
        transmitStringData(gps_data); //actually send the GPS data
    } else if(inst == (uint8_t) 138){//sync the RTC with the gps time x8A
        transmitByteData((uint8_t) 138); //acknowledge
        //todo:include code for internal sync of gps
    } else if(inst == (uint8_t) 139){ //diagnose request, this hasn't been completely thought about
        transmitByteData((uint8_t) 139); //acknowledge
        //todo:we must figure this out
        transmitByteData(diagnosis);
    } else if(inst == (uint8_t) 140){ //request configuration validity
        transmitByteData((uint8_t) 140); //acknowledge
        transmitByteData(valid_conf);
    } else if(inst == (uint8_t) 255){ //request cancel xff
        //todo: implement cancel process
        transmitByteData((uint8_t) 255);
    } else { //send a message stating that the function was a mistake
        transmitByteData((uint8_t) 254);
    }
}

void transmitStringData(char *string){
    int i = 0;
    trans_count = 0;
    while(*string){
        MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) *string++);
        trans_count++;
    }
    MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) 0x0D);
    MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) 0x0A);
    i = 0;
}

void transmitByteData(uint8_t byte){
    MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) byte);
    MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) 0x0D);
    MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) 0x0A);
}

void transmitNByteData(uint8_t byte){
    MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) byte);
}
