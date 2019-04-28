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
	{"diagnose", CMD_diagnose, " : Enable/Disable Diagnostic Mode"},
	{"datareq", CMD_getData, " : Request Data from DAQ units"},
	{"gain", CMD_setGain, " : Configure PGA Gain factor"},
	{"create", CMD_create, " : Read data from DAQ file"},
	{"read", CMD_read, " : Read data from DAQ file"},
	{"write", CMD_write, " : Read data from DAQ file"},
	{"cutoff", CMD_setCutoffFreq, " : Set cutoff frequency"},
	{"sample", CMD_setSamplingFreq, " : Set sampling frequency"},
	{"start", CMD_start, " : Start test"},
	{"duration", CMD_duration, " : Send duration of test"},
	{"initcard", CMD_initCard, " : Reinitialize SD Card"},
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




int CMD_diagnose(int argc, char **argv){

	uint8_t p = atoi(argv[1]);

	while (MAP_I2C_masterIsStopSent(EUSCI_B2_BASE) == EUSCI_B_I2C_SENDING_STOP);

	/* Sending the initial start condition */
	MAP_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, I2C_PACKET_HEADER);
	MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, CMD_DIAGNOSE_DEFINITION);
	MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, p);
	MAP_I2C_masterSendMultiByteStop(EUSCI_B2_BASE);

	return (0);
}

int CMD_setCutoffFreq(int argc, char **argv){
	uint8_t p = atoi(argv[1]);

	while (MAP_I2C_masterIsStopSent(EUSCI_B2_BASE) == EUSCI_B_I2C_SENDING_STOP);

	/* Sending the initial start condition */
	MAP_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, I2C_PACKET_HEADER);
	MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, CMD_SET_CUTOFF_FREQ_DEFINITION);
	MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, p);
	MAP_I2C_masterSendMultiByteStop(EUSCI_B2_BASE);

	return (0);


}


int CMD_setSamplingFreq(int argc, char **argv){

	uint8_t p = atoi(argv[1]);

	while (MAP_I2C_masterIsStopSent(EUSCI_B2_BASE) == EUSCI_B_I2C_SENDING_STOP);

	/* Sending the initial start condition */
	MAP_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, I2C_PACKET_HEADER);
	MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, CMD_SET_SAMPLING_FREQ);
	MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, p);
	MAP_I2C_masterSendMultiByteStop(EUSCI_B2_BASE);

	return 0;
}

int CMD_initCard(int argc, char **argv){

	//TODO
	return 0;
}


int CMD_start(int argc, char ** argv){
		GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);

		//Delay
		int i = 0;
		while(i < 400000)
			i++;
		GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);

		return 0;
}


int CMD_getData(int argc, char ** argv){

    while (MAP_I2C_masterIsStopSent(EUSCI_B2_BASE));


    MAP_I2C_masterReceiveStart(EUSCI_B2_BASE);
	return (0);
}


int CMD_setGain(int argc, char ** argv){

	uint8_t p = atoi(argv[1]);

	/* Making sure the last transaction has been completely sent out */
	while (MAP_I2C_masterIsStopSent(EUSCI_B2_BASE) == EUSCI_B_I2C_SENDING_STOP);

	/* Sending the initial start condition */
	MAP_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, I2C_PACKET_HEADER);
	MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, CMD_GAIN_DEFINITION);
	MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, p);
	MAP_I2C_masterSendMultiByteStop(EUSCI_B2_BASE);
	return 0;
}


int CMD_create(int argc, char ** argv){

    if ((src = fopen(daq1, "r")))
    {
    	UARTprintf(EUSCI_A0_BASE, "File for DAQ1 already exists\r\n");
    }else{
    	UARTprintf(EUSCI_A0_BASE, "Creating File for DAQ1 module\r\n");
    	src = fopen(daq1, "w+");

    }


    if ((src = fopen(daq2, "r")))
    {
    	UARTprintf(EUSCI_A0_BASE, "File for DAQ2 already exists\r\n");
    }else{
    	UARTprintf(EUSCI_A0_BASE, "Creating File for DAQ2 module\r\n");
    	src = fopen(daq2, "w+");

    }

    	fclose(src);
    	return 0;

}


int CMD_read(int argc, char ** argv){

	unsigned int bytesRead = 0;
	if(argc != 2){
		UARTprintf(EUSCI_A0_BASE, "Incorrect number of arguments\n");
		return -1;
	}

	if(atoi(argv[1]) == 1)
		src = fopen(daq1, "rb");
	else if(atoi(argv[1]) == 2)

		src = fopen(daq2, "rb");

	else if(atoi(argv[1]) == 3){
		src = fopen(daq3, "rb");

	}else if(atoi(argv[1]) == 4){
		src = fopen(daq4, "rb");

	}else if(atoi(argv[1]) == 5){
		src = fopen(daq5, "rb");

	}else if(atoi(argv[1]) == 6){
		src = fopen(daq6, "rb");

	}else if(atoi(argv[1]) == 7){
		src = fopen(daq7, "rb");

	}else if(atoi(argv[1]) == 8){
		src = fopen(daq8, "rb");
	}

    if (!src) {
    	UARTprintf(EUSCI_A0_BASE, "\nError opening file!\r\n");

    	return -1;
    }

    char cpy_buff[2048 + 1];
    memset(cpy_buff, 0x00, 2048 + 1);
    int i;
    while (true) {
    	i = 0;
        /*  Read from source file */
        bytesRead = fread(cpy_buff, sizeof(char), 2048, src);
        if (bytesRead == 0) {
            break; /* Error or EOF */
        }
        cpy_buff[bytesRead] = '\0';

    	while(i != bytesRead){
    		 MAP_UART_transmitData(EUSCI_A1_BASE, (char) cpy_buff[i++]);
//    		 MAP_UART_transmitData(EUSCI_A0_BASE, (char) cpy_buff[i++]);
    	}

        MAP_UART_transmitData(EUSCI_A1_BASE, 0xAA);
        MAP_UART_transmitData(EUSCI_A1_BASE, 0xBB);
        MAP_UART_transmitData(EUSCI_A1_BASE, 0xAA);
        MAP_UART_transmitData(EUSCI_A1_BASE, 0xBB);
        MAP_UART_transmitData(EUSCI_A1_BASE, 0x0D);
        MAP_UART_transmitData(EUSCI_A1_BASE, 0x0A);



	}

//    /* Get the filesize of the source file */
//    fseek(src, 0, SEEK_END);
//    //filesize = ftell(src);
    rewind(src);

    fclose(src);

	return 0;
}


int CMD_write(int argc, char ** argv){

//	unsigned int bytesRead = 0;
	if(argc != 3){
		UARTprintf(EUSCI_A0_BASE, "Incorrect number of arguments\n");
		return -1;
	}

	if(atoi(argv[2]) == 1)
		src = fopen(daq1, "a");
	else if(atoi(argv[2]) == 2)
		src = fopen(daq2, "a");

    if (!src) {
    	UARTprintf(EUSCI_A0_BASE, "\nError opening file!\r\n");

    	return -1;
    }

    fwrite(argv[1], sizeof(char), strlen(argv[1]), src);
    UARTprintf(EUSCI_A0_BASE, "\nFile written!\r\n");

    rewind(src);
    fclose(src);

	return 0;

}

int CMD_duration(int argc, char ** argv){

	short p = atoi(argv[1]);
	uint8_t msb = (p >> 8);
	uint8_t lsb = (uint8_t) p;

	while (MAP_I2C_masterIsStopSent(EUSCI_B2_BASE) == EUSCI_B_I2C_SENDING_STOP);

	/* Sending the initial start condition */
	MAP_I2C_masterSendMultiByteStart(EUSCI_B2_BASE, I2C_PACKET_HEADER);
	MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, CMD_DURATION_DEFINITION);
	MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, msb);
	MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, lsb);
	MAP_I2C_masterSendMultiByteStop(EUSCI_B2_BASE);

	return 0;
}

int CMD_reset(int argc, char **argv){

	GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0);
	int i;

	for(i = 0; i < 40000; i++){
		//Delay
	}

	GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0);
	return 0;

}



