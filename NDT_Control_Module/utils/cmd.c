#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdio.h>
#include <file.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
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
    {"help",     CMD_help,      " : Display list of commands" },
	{"mode", CMD_mode, " : Select ADC Operation Mode"},
	{"diagnose", CMD_diagnose, " : Enable/Disable Diagnostic Mode"},
	{"pwdn", CMD_pwdn, " : Power Down ADC Channel"},
	{"pwup", CMD_pwup, " : Power Up ADC Channel"},
	{"datareq", CMD_getData, " : Request Data from DAQ units"},
	{"gain", CMD_setGain, " : Configure PGA Gain factor"},
	{"save", CMD_read, " : Read data from DAQ file"},
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
//    printf(EUSCI_A0_BASE, "\n\r");

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


int CMD_read(int argc, char ** argv){

	unsigned int bytesRead = 0;
    src = fopen(daq1, "r");
    if (!src) {


//        /* Open file for both reading and writing */
//        src = fopen(daq1, "w+");
        if (!src) {

            while (1);
        }

        //bytesWritten = fwrite(textarray, 1, strlen(textarray), src);
        //fflush(src);

        /* Reset the internal file pointer */
        rewind(src);

    }

    char cpy_buff[2048 + 1];
    memset(cpy_buff, 0x00, 2048 + 1);
    /*  Copy the contents from the src to the dst */

    while (true) {
        /*  Read from source file */
        bytesRead = fread(cpy_buff, 1, 2048, src);
        if (bytesRead == 0) {
            break; /* Error or EOF */
        }
        UARTprintf(EUSCI_A0_BASE, "%s", cpy_buff);
        //sendUARTString(EUSCI_A0_BASE, cpy_buff);


	}


    /* Get the filesize of the source file */
    fseek(src, 0, SEEK_END);
    //filesize = ftell(src);
    rewind(src);

    /* Close both inputfile[] and outputfile[] */
    fclose(src);

	return 0;
}


//
//int CMD_QuitProcess(int argc, char **argv)
//{
//	QuitProcess();
//	return 0;
//}
