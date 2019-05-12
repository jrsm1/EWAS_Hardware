/*******************************/
//      Includes                //
/********************************/
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdio.h>
#include <file.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "utils/cmdline.h"
#include "utils/cmd.h"
#include "utils/printf.h"


//*****************************************************************************
//
// Table of valid command strings, callback functions and help messages.  This
// is used by the cmdline module.
//
//*****************************************************************************
tCmdLineEntry g_psCmdTable[] =
{
    {"help",     CMD_help, " : Display list of commands"},
    {"diagnose", CMD_diagnose, " : Select lines for turning On/Off Sensor/ADC channels"},
    {"datareq", CMD_getData, " : Request Data from DAQ modules"},
    {"gain", CMD_setGain, " : Configure PGA Gain factor"},
    {"create", CMD_create, " : Create DAQ module files on SD card"},
    {"read", CMD_read, " : Write data to a DAQ file"},
    {"write", CMD_write, " : Read data from a DAQ file"},
    {"cutoff", CMD_setCutoffFreq, " : Set filter cutoff frequency"},
    {"sample", CMD_setSamplingFreq, " : Set ADC sampling frequency"},
    {"start", CMD_start, " : Send excitation impulse to solenoid and start test"},
    {"duration", CMD_duration, " : Send duration of an experiment in seconds"},
    {"reset", CMD_reset, " : Reset DAQ modules"},
    { 0, 0, 0 }
};


//*****************************************************************************
//
// Command: help
//
// Print the help strings for all commands.
//
//*****************************************************************************
int CMD_help(int argc, char **argv)
{
    int32_t i32Index;
    (void)argc;
    (void)argv;
    i32Index = 0;

    UARTprintf(EUSCI_A3_BASE, "List of Commands\n\r");
    while(g_psCmdTable[i32Index].pcCmd){
        UARTprintf(EUSCI_A3_BASE, "%s %s\n\r", g_psCmdTable[i32Index].pcCmd,
                g_psCmdTable[i32Index].pcHelp);

      i32Index++;
    }

    return (0);
}



//*****************************************************************************
//
// Command: diagnose
// Syntax: "diagnose x" where x is the decimal equivalent of the desired channels
// to be selected
// Used to enable/disable an individual channel on the ADC and the MUX.
// The 4 least significant bits are used for the MUX select lines, where
// a 0 means the sensor input is selected and a 1 means the voltage reference
// input is selected for that particular channel.
// Bit 0 is the selector for channel 1, Bit 1 is the selector for channel 2, Bit
// Bit 2 is the selector for channel 3 and Bit 4 is the selector for channel 4.
//
// The 4 most significant bits are used for the ADC ~PWDN lines, where a 0 turns
// off the selected channel and a 1 turns on the selected channel. Bit4 is
// channel 1, Bit5 is channel 2, Bit6 is channel 3 and Bit7 is channel 4.
//
//*****************************************************************************
int CMD_diagnose(int argc, char **argv){

    uint8_t p = atoi(argv[1]);

    MAP_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, I2C_PACKET_HEADER);
    MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, CMD_DIAGNOSE_DEFINITION);
    MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, p);
    MAP_I2C_masterSendMultiByteStop(EUSCI_B2_BASE);

    return (0);
}

//*****************************************************************************
//
// Command: setCutoffFreq
// Syntax: "cutoff x" where x is the desired cutoff frequency
//  Parameter   Cutoff (HZ)
//    0         1
//    1         2
//    2         4
//    3         8
//    4         16
//    5         32
//    6         64
//    7         128
//    8         256
//    9         512
//    10        1024
//    11        2048
//    12        4096
//    13        8192
//    14        10000
//
//*****************************************************************************
int CMD_setCutoffFreq(int argc, char **argv){
    uint8_t p = atoi(argv[1]);

    MAP_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, I2C_PACKET_HEADER);
    MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, CMD_SET_CUTOFF_FREQ_DEFINITION);
    MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, p);
    MAP_I2C_masterSendMultiByteStop(EUSCI_B2_BASE);

    return (0);

}


//*****************************************************************************
//
// Command: setSamplingFrequency
// Syntax: "sample x" where x is the desired sampling frequency
//        Parameter   Sampling Freq (Hz)
//        0                 2
//        1                 4
//        2                 8
//        3                 16
//        4                 32
//        5                 64
//        6                 128
//        7                 256
//        8                 512
//        9                 1024
//        10                2048
//        11                4096
//        12                8192
//        13                16384
//        14                20000
//
//*****************************************************************************
int CMD_setSamplingFreq(int argc, char **argv){

    uint8_t p = atoi(argv[1]);


    MAP_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, I2C_PACKET_HEADER);
    MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, CMD_SET_SAMPLING_FREQ);
    MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, p);
    MAP_I2C_masterSendMultiByteStop(EUSCI_B2_BASE);


    return 0;
}


//*****************************************************************************
//
// Command: start
// Syntax: "start"
// Sends the enable signal to the solenoid, used to induce vibrations on the
// test specimen.
//*****************************************************************************
int CMD_start(int argc, char ** argv){

    int i = 0;
    while(i < 1600000)
        i++;

        P2->OUT |= BIT4;

        //Delay
        i = 0;
        while(i < 400000)
            i++;

        P2->OUT &= ~BIT4;

        return 0;
}

//*****************************************************************************
//
// Command: getData
// Syntax: "datareq"
// Initiates the transfer of data from DAQ Module SRAM to the Control Module.
// This command gets called when the Control Module receives a Data Ready
// (DRDY) Interrupt from a DAQ Module. This command also enables the I2C Rx
// Interrupt
//
//*****************************************************************************
int CMD_getData(int argc, char ** argv){

    EUSCI_B2->IE |= BIT0; //Enable RX interrupt
    EUSCI_B2->CTLW0 &= ~BIT4; //Sets the Control Module as a Receiver
    EUSCI_B2->CTLW0 |= BIT1; //Send Start condition

    return 0;
}


//*****************************************************************************
//
// Command: setGain
// Syntax: "gain x" where x is the gain step to
//        Parameter         Gain (V/V)
//        0                 0.2
//        1                 1
//        2                 10
//        3                 20
//        4                 30
//        5                 40
//        6                 60
//        7                 80
//        8                 120
//        9                 157
//*****************************************************************************
int CMD_setGain(int argc, char ** argv){

    uint8_t p = atoi(argv[1]);

    MAP_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, I2C_PACKET_HEADER);
    MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, CMD_GAIN_DEFINITION);
    MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, p);
    MAP_I2C_masterSendMultiByteStop(EUSCI_B2_BASE);

    return 0;
}


//*****************************************************************************
//
// Command: create
// Syntax: "create"
// Currently used for testing purposes only. Creates one file for each
// DAQ Module on the SD Card. If the file exists, it does nothing.
//*****************************************************************************
int CMD_create(int argc, char ** argv){

    if ((src = fopen(daq1, "r")))
    {
        UARTprintf(EUSCI_A0_BASE, "File for DAQ1 already exists\r\n");
    }else{
        UARTprintf(EUSCI_A0_BASE, "Creating File for DAQ1 module\r\n");
        src = fopen(daq1, "w+");

    }


    if ((src = fopen(daq2, "r"))){
        UARTprintf(EUSCI_A0_BASE, "File for DAQ2 already exists\r\n");
    }else{
        UARTprintf(EUSCI_A0_BASE, "Creating File for DAQ2 module\r\n");
        src = fopen(daq2, "w+");
    }

        fclose(src);
        return 0;

}

//*****************************************************************************
//
// Command: read
// Syntax: "read x" where x is the DAQ Module file number to read
// Read a DAQ Module file and output its entire contents to a serial
// interface (For this project, it is Connected to the PC Application)
// For testing using a console emulator such as putty, replace all instances
// of EUSCI_A1_BASE with EUSCI_A0_BASE
//
//*****************************************************************************
int CMD_read(int argc, char ** argv){

    if(argc != 2){ //Used in console emulator
        UARTprintf(EUSCI_A0_BASE, "Incorrect number of arguments\n");
        return -1;
    }

    if(atoi(argv[1]) == 1){
        src = fopen(daq1, "r");

    }else if(atoi(argv[1]) == 2){
        src = fopen(daq2, "r");

    }else if(atoi(argv[1]) == 3){
        src = fopen(daq3, "r");

    }else if(atoi(argv[1]) == 4){
        src = fopen(daq4, "r");

    }else if(atoi(argv[1]) == 5){
        src = fopen(daq5, "r");

    }else if(atoi(argv[1]) == 6){
        src = fopen(daq6, "r");

    }else if(atoi(argv[1]) == 7){
        src = fopen(daq7, "r");

    }else if(atoi(argv[1]) == 8){
        src = fopen(daq8, "r");

    }

    if (!src){ //Used in console emulator
        UARTprintf(EUSCI_A0_BASE, "\nError opening file!\r\n");

        return -1;
    }

    int c; // note: int, not char, required to handle EOF
    while ((c = fgetc(src)) != EOF) { //Transmit entire file contents without a buffer
       putchar(c);
       MAP_UART_transmitData(EUSCI_A1_BASE, (char) c);
    }

    //Termination sequence for PC Application
    MAP_UART_transmitData(EUSCI_A1_BASE, 0xAA);
    MAP_UART_transmitData(EUSCI_A1_BASE, 0xBB);
    MAP_UART_transmitData(EUSCI_A1_BASE, 0xAA);
    MAP_UART_transmitData(EUSCI_A1_BASE, 0xBB);
    MAP_UART_transmitData(EUSCI_A1_BASE, 0x0D);
    MAP_UART_transmitData(EUSCI_A1_BASE, 0x0A);

    rewind(src);
    fclose(src);

    return 0;
}


//*****************************************************************************
//
// Command: write
// Syntax: "write x y" where x is the text to append and
// y is the DAQ Module file number to write to.
// Currently used for testing purposes only, appends a string to the specified DAQ
// Module file.
//
//*****************************************************************************
int CMD_write(int argc, char ** argv){

    if(argc != 3){
        UARTprintf(EUSCI_A0_BASE, "Incorrect number of arguments\n");
        return -1;
    }

    if(atoi(argv[2]) == 1){
        src = fopen(daq1, "a");
    }
    else if(atoi(argv[2]) == 2){
        src = fopen(daq2, "a");
    }
    else if(atoi(argv[2]) == 3){
        src = fopen(daq3, "a");
    }
    else if(atoi(argv[2]) == 4){
        src = fopen(daq4, "a");
    }
    else if(atoi(argv[2]) == 5){
        src = fopen(daq5, "a");
    }
    else if(atoi(argv[2]) == 6){
        src = fopen(daq6, "a");
    }
    else if(atoi(argv[2]) == 7){
        src = fopen(daq7, "a");
    }
    else if(atoi(argv[2]) == 8){
        src = fopen(daq8, "a");
    }

    if (!src) {
        UARTprintf(EUSCI_A0_BASE, "\nError opening file!\r\n");

        return -1;
    }

    fwrite(argv[1], 1, strlen(argv[1]), src);
    UARTprintf(EUSCI_A0_BASE, "\nFile written!\r\n");

    rewind(src);
    fclose(src);

    return 0;

}


//*****************************************************************************
//
// Command: duration
// Syntax: "duration x" where is the duration in seconds
// Sends the duration (in seconds) of an experiment to the DAQ Modules
// Duration time can range from 5 seconds to 1800 seconds (30 minutes)
// Therefore, duration is of type uint16_t to accomodate this range.
//
//*****************************************************************************
int CMD_duration(int argc, char ** argv){

    short p = atoi(argv[1]);
    uint8_t msb = (p >> 8);
    uint8_t lsb = (uint8_t) p;

    MAP_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, I2C_PACKET_HEADER);
    MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, CMD_DURATION_DEFINITION);
    MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, msb);
    MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, lsb);
    MAP_I2C_masterSendMultiByteStop(EUSCI_B2_BASE);

    return 0;
}


//*****************************************************************************
//
// Command: reset
// Sends the reset (Active Low) signal to all the DAQ Modules
//*****************************************************************************
int CMD_reset(int argc, char **argv){

    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0);
    int i;

    for(i = 0; i < 40000; i++){
        //Delay
    }

    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0);
    return 0;

}


