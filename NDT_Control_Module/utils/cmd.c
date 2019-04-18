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
	{"mode", CMD_mode, " : Select ADC Operation Mode"},
	{"diagnose", CMD_diagnose, " : Enable/Disable Diagnostic Mode"},
	{"pwdn", CMD_pwdn, " : Power Down ADC Channel"},
	{"pwup", CMD_pwup, " : Power Up ADC Channel"},
	{"datareq", CMD_getData, " : Request Data from DAQ units"},
	{"gain", CMD_setGain, " : Configure PGA Gain factor"},
	{"create", CMD_create, " : Read data from DAQ file"},
	{"read", CMD_read, " : Read data from DAQ file"},
	{"write", CMD_write, " : Read data from DAQ file"},
//	{"setconfig", CMD_setconfig, " : TODO"},
//	{"requestconfig", CMD_requestconfig, " : TODO"},
//	{"requeststart", CMD_requeststart, " : TODO"},
//	{"requestdata", CMD_requestdata, " : TODO"},
//	{"requestlivebytes", CMD_requestlivebytes, " : TODO"},
//	{"cancelrequest", CMD_cancerequest, " : TODO"},
//	{"gpsdatarequest", CMD_gpsdatarequest, " : TODO"},
//	{"gpssyncrequest", CMD_gpssyncrequest, " : TODO"},
//	{"requestnumberofmodules", CMD_requestnumberofmodules, " : TODO"},
//	{"diagnoserequest", CMD_diagnoserequest, " : TODO"},
//	{"requestconfigurationvalidity", CMD_requestconfigurationvalidity, " : TODO"},
//	{"requeststatus", CMD_requeststatus, " : TODO"},
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


int CMD_mode(int argc, char **argv){

	int p = (int)strtol(argv[1], NULL, 16);

	if((p & 0x00E1) == 0x00E1){
		UARTprintf(EUSCI_A3_BASE, "Setting High Resolution Mode\r\n");

	}
	else if((p & 0x00E2) == 0x00E2){
		UARTprintf(EUSCI_A3_BASE, "Setting Low Power Mode\r\n");

	}
	else if((p & 0x00E3) == 0x00E3){
		UARTprintf(EUSCI_A3_BASE, "Setting Low Speed Mode\r\n");

	}

	return (0);
}


int CMD_diagnose(int argc, char **argv){

	uint8_t p = (uint8_t)strtol(argv[1], NULL, 16);

	//Construct packet
	masterToSlavePacket[1] = CMD_DIAGNOSE_DEFINITION;
	masterToSlavePacket[2] = p;
	masterToSlaveByteCtr = MASTER_TO_SLAVE_PACKET_SIZE - 1;

	/* Making sure the last transaction has been completely sent out */
	while (MAP_I2C_masterIsStopSent(EUSCI_B0_BASE) == EUSCI_B_I2C_SENDING_STOP);

	/* Sending the initial start condition */
	MAP_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, masterToSlavePacket[masterToSlaveIndex++]);

	return (0);
}


int CMD_pwdn(int argc, char **argv){

	uint8_t p = (uint8_t)strtol(argv[1], NULL, 16);

	//Construct packet
	masterToSlavePacket[1] = CMD_PWDN_DEFINITION;
	masterToSlavePacket[2] = p;
	masterToSlaveByteCtr = MASTER_TO_SLAVE_PACKET_SIZE - 1;

	/* Making sure the last transaction has been completely sent out */
	while (MAP_I2C_masterIsStopSent(EUSCI_B0_BASE) == EUSCI_B_I2C_SENDING_STOP);

	/* Sending the initial start condition */
	MAP_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, masterToSlavePacket[masterToSlaveIndex++]);


	if((p & 0x00E1) == 0x00E1){
		UARTprintf(EUSCI_A3_BASE, "Powering down Channel 1\r\n");

	}

	if((p & 0x00E2) == 0x00E2){
		UARTprintf(EUSCI_A3_BASE, "Powering down Channel 2\r\n");

	}
	if((p & 0x00E4) == 0x00E4){
		UARTprintf(EUSCI_A3_BASE, "Powering down Channel 3\r\n");

	}

	if((p & 0x00E8) == 0x00E8){
		UARTprintf(EUSCI_A3_BASE, "Powering down Channel 4\r\n");

	}

	return (0);
}


int CMD_pwup(int argc, char **argv){

	uint8_t p = (uint8_t)strtol(argv[1], NULL, 16);

	//Construct packet
	masterToSlavePacket[1] = CMD_PWUP_DEFINITION;
	masterToSlavePacket[2] = p;
	masterToSlaveByteCtr = MASTER_TO_SLAVE_PACKET_SIZE - 1;

	/* Making sure the last transaction has been completely sent out */
	while (MAP_I2C_masterIsStopSent(EUSCI_B0_BASE) == EUSCI_B_I2C_SENDING_STOP);

	/* Sending the initial start condition */
	MAP_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, masterToSlavePacket[masterToSlaveIndex++]);


	if((p & 0x00E1) == 0x00E1){
		UARTprintf(EUSCI_A3_BASE, "Powering up Channel 1\r\n");

	}

	if((p & 0x00E2) == 0x00E2){
		UARTprintf(EUSCI_A3_BASE, "Powering up Channel 2\r\n");

	}
	if((p & 0x00E4) == 0x00E4){
		UARTprintf(EUSCI_A3_BASE, "Powering up Channel 3\r\n");

	}

	if((p & 0x00E8) == 0x00E8){
		UARTprintf(EUSCI_A3_BASE, "Powering up Channel 4\r\n");

	}

	return (0);
}


int CMD_getData(int argc, char ** argv){
	UARTprintf(EUSCI_A3_BASE, "Reading Slaves\r\n");

    while (MAP_I2C_masterIsStopSent(EUSCI_B0_BASE));


    MAP_I2C_masterReceiveStart(EUSCI_B0_BASE);
	return (0);
}


int CMD_setGain(int argc, char ** argv){
	UARTprintf(EUSCI_A3_BASE, "Changing Gain\r\n");

	uint8_t p = (uint8_t)strtol(argv[1], NULL, 16);

	//Construct packet
	masterToSlavePacket[1] = CMD_GAIN_DEFINITION;
	masterToSlavePacket[2] = (p & 0x0F);
	masterToSlaveByteCtr = MASTER_TO_SLAVE_PACKET_SIZE - 1;

	/* Making sure the last transaction has been completely sent out */
	while (MAP_I2C_masterIsStopSent(EUSCI_B0_BASE) == EUSCI_B_I2C_SENDING_STOP);

	/* Sending the initial start condition */
	MAP_I2C_masterSendMultiByteStart(EUSCI_B0_BASE, masterToSlavePacket[masterToSlaveIndex++]);

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
		src = fopen(daq1, "r");
	else if(atoi(argv[1]) == 2)
		src = fopen(daq2, "r");
    if (!src) {
    	UARTprintf(EUSCI_A0_BASE, "\nError opening file!\r\n");

//    if (!src) {
//
//    	while (1);
//    }

        //bytesWritten = fwrite(textarray, 1, strlen(textarray), src);
        //fflush(src);

        /* Reset the internal file pointer */
//        rewind(src);
    	return -1;
    }

    char cpy_buff[2048 + 1];
    memset(cpy_buff, 0x00, 2048 + 1);

    while (true) {
        /*  Read from source file */
        bytesRead = fread(cpy_buff, 1, 2048, src);
        if (bytesRead == 0) {
            break; /* Error or EOF */
        }
        UARTprintf(EUSCI_A0_BASE, "%s", cpy_buff);

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

    fwrite(argv[1], 1, strlen(argv[1]), src);
    UARTprintf(EUSCI_A0_BASE, "\nFile written!\r\n");

    rewind(src);
    fclose(src);

	return 0;

}





