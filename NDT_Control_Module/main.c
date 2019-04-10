/********************************/
//		Includes				//
/********************************/

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <file.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <third_party/fatfs/ffcio.h>
#include <time.h>
#include <ti/drivers/SDFatFS.h>
#include "boardConfigs/Board.h"
#include "utils/cmd.h"
#include "utils/cmdline.h"
#include "utils/printf.h"


/********************************/
//		Defines					//
/********************************/


/* Console UART Macros */
#define CONSOLE_UART_BASE 			EUSCI_A0_BASE
#define CONSOLE_UART_GPIO_PORT		GPIO_PORT_P1
#define CONSOLE_UART_INT			INT_EUSCIA0
#define CONSOLE_UART_RX				GPIO_PIN2
#define CONSOLE_UART_TX				GPIO_PIN3


/* ESP32 UART Macros */
/*
#define ESP32_UART_BASE				EUSCI_A2_BASE
#define ESP32_UART_GPIO_PORT		GPIO_PORT_P3
#define ESP32_UART_INT				INT_EUSCIA2
#define ESP32_UART_RX				GPIO_PIN2
#define ESP32_UART_TX				GPIO_PIN3
*/


/* GPS UART Macros*/
#define GPS_UART_BASE				EUSCI_A2_BASE
#define GPS_UART_GPIO_PORT			GPIO_PORT_P3
#define GPS_UART_INT				INT_EUSCIA2
#define GPS_UART_RX					GPIO_PIN2
#define GPS_UART_TX					GPIO_PIN3


/* GPS Timer Macros*/
#define MAX_FAILS 					10
#define TIMER_PERIOD    			0x8000
#define TIME_WAIT 					30
#define GPS_TIMER_BASE				TIMER_A1_BASE
#define GPS_TIMER_INT				INT_TA1_0


/* FTDI UART Macros */
#define FTDI_UART_BASE				EUSCI_A3_BASE
#define FTDI_UART_GPIO_PORT			GPIO_PORT_P9
#define FTDI_UART_INT				INT_EUSCIA3
#define FTDI_UART_RX				GPIO_PIN6
#define FTDI_UART_TX				GPIO_PIN7


/* I2C Bus Macros */
#define DEFAULT_SLAVE_ADDRESS   	0x60 //0xC0 >> 1
#define I2C_BUS_BASE				EUSCI_B2_BASE
#define I2C_BUS_GPIO_PORT			GPIO_PORT_P3
#define I2C_BUS_INT					INT_EUSCIB2
#define I2C_BUS_SCL					GPIO_PIN6
#define I2C_BUS_SDA					GPIO_PIN7
#define I2C_BUS_MAX_DAQS			8
#define MASTER_TO_SLAVE_PACKET_SIZE 3
#define NUM_OF_REC_BYTES			600


/* General System Macros */
#define BUFFER_SIZE 				128
#define SYSTEM_FREQ					CS_DCO_FREQUENCY_12


/* Buffer size used for the file copy process */
#ifndef CPY_BUFF_SIZE
#define CPY_BUFF_SIZE       		2048
#endif


/* String conversion macro */
#define STR_(n)             		#n
#define STR(n)              		STR_(n)


/* Drive number used for FatFs */
#define DRIVE_NUM           		0


/********************************/
//		Function Prototypes		//
/********************************/


void configureConsoleUART(void);
//void configureESPUART(void);
void configureGPSUART(void);
void configureGPSTimer(void);
void configureFTDIUART(void);
void configureI2CBus(void);
void configureSDCard(void);
void initializeSlaves(void);
void processCommand(char *CommandText);
void sendUARTString(uint32_t moduleInstance, char * msg);


/********************************/
//		Global Variables		//
/********************************/


static char consoleInput[BUFFER_SIZE];
//static char wifiInput[BUFFER_SIZE];
static char COMMAND[BUFFER_SIZE];
char masterToSlavePacket[MASTER_TO_SLAVE_PACKET_SIZE];
uint8_t masterToSlaveIndex = 0;
uint8_t masterToSlaveByteCtr = 3;
//static volatile uint8_t wifiIndex = 0;
static volatile uint8_t inputIndex = 0;
static volatile bool slavesInitialized = false;
static volatile uint8_t slaveIndex = 0;
static volatile uint8_t inputFlag = 0;
//static volatile uint8_t wifiFlag = 0;
static volatile uint8_t cardFlag = 0;
const uint8_t DAQAddresses[] = {0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57};
//static uint8_t activeDAQModules = 0;
static uint8_t RXData[NUM_OF_REC_BYTES];
static volatile uint32_t xferIndex;


/* GPS Global Variables */
short timeout = 0;
short seconds = 0;
short check_format = 0;
short record = 0;
short counter = 0;
char format[6];
short comma_counter = 0;
short compare_value;

typedef struct GPSformats
{
    char nMEA_Record[6];
    char utc[10];
    char latitude[10];
    char N_S;
    char longtitude[11];
    char E_W;
    short valid_Data;
} GPS;

GPS gps;

//


/* Module Configuration Global Variables */

//Console Baudrate 9600bps
const eUSCI_UART_Config consoleConfig =
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


//ESP32 Baudrate 9600bps
/*
const eUSCI_UART_Config espConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        78,                                     // BRDIV = 78
        2,                                      // UCxBRF = 2
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};
*/

//GPS Baudrate 9600bps
const eUSCI_UART_Config gpsConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        78,                                     // BRDIV = 78
        2,                                      // UCxBRF = 2
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};



//FTDI Baudrate 115200bps
const eUSCI_UART_Config ftdiConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        6,                                     // BRDIV = 78
        8,                                      // UCxBRF = 2
        32,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};


/* I2C Master Configuration Parameters */
const eUSCI_I2C_MasterConfig i2cBusConfig =
{
        EUSCI_B_I2C_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        12000000,                                // SMCLK = 12MHz
        EUSCI_B_I2C_SET_DATA_RATE_400KBPS,      // Desired I2C Clock of 100khz
        0,                                      // No byte counter threshold
        EUSCI_B_I2C_NO_AUTO_STOP                // No Autostop
};


//GPS Timer Configuration
const Timer_A_UpModeConfig upConfig = {
		TIMER_A_CLOCKSOURCE_ACLK,              // ACLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,          // ACLK/1 = 32.768kHz
        TIMER_PERIOD,                           // 32768 ticks
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
};


////////////////////////////////////////

//TODO

SDFatFS_Handle sdfatfsHandle;
FILE *src, *dst;
const char inputfile[] = "fat:"STR(DRIVE_NUM)":input.txt";
const char daq1[] = "fat:"STR(DRIVE_NUM)":DAQ1.txt";
const char daq2[] = "fat:"STR(DRIVE_NUM)":DAQ2.txt";

/* File name prefix for this filesystem for use with TI C RTS */
char fatfsPrefix[] = "fat";

unsigned char cpy_buff[CPY_BUFF_SIZE + 1];
const char textarray[] = \
		"***********************************************************************\n"
		"0         1         2         3         4         5         6         7\n";


///////////////////////////////////////



/********************************/
//		Main					//
/********************************/

int main(void)
{

	//Stop Watchdog Timer
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

//    /* Variables to keep track of the file copy progress */
    //unsigned int bytesRead = 0;
    //unsigned int bytesWritten = 0;
    //unsigned int filesize;
    //unsigned int totalBytesCopied = 0;
    strcpy(gps.nMEA_Record, "GPGGA");
    gps.valid_Data = 0;

    memset(consoleInput, 0x00, BUFFER_SIZE);

    configureSDCard();

    //SD Card configuration changes MCLK, SMCLK, re-init to desired values
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    //CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    configureConsoleUART();
    //configureESPUART(); //Currently not in use
//    configureFTDIUART();
//    configureGPSUART();
    configureI2CBus();

    initializeSlaves();


//	masterToSlavePacket[0] = 0xCA;
    Interrupt_enableMaster();


    while(true){

//    	MAP_PCM_gotoLPM0();

        if(inputFlag){
        	inputFlag = 0;
/*
        	if(wifiFlag){
        		wifiFlag = 0;
        		processCommand(wifiInput);
        		memset(wifiInput, 0x00, BUFFER_SIZE);
        	}else{
        		processCommand(consoleInput);
        	}
*/
        	processCommand(consoleInput);
            inputIndex = 0;

        }


    }


}


/********************************/
//		Function Definitions	//
/********************************/


void configureConsoleUART(void){
    GPIO_setAsPeripheralModuleFunctionInputPin(CONSOLE_UART_GPIO_PORT,
    		CONSOLE_UART_RX | CONSOLE_UART_TX, GPIO_PRIMARY_MODULE_FUNCTION);

    CS_setDCOCenteredFrequency(SYSTEM_FREQ); //DCO Freq is used for MCLK, SMCLK upon reset

    UART_initModule(CONSOLE_UART_BASE, &consoleConfig);

    UART_enableModule(CONSOLE_UART_BASE);

    UART_enableInterrupt(CONSOLE_UART_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(CONSOLE_UART_INT);
}


/*
void configureESPUART(void){
    GPIO_setAsPeripheralModuleFunctionInputPin(ESP32_UART_GPIO_PORT,
    		ESP32_UART_RX | ESP32_UART_TX, GPIO_PRIMARY_MODULE_FUNCTION);

    CS_setDCOCenteredFrequency(SYSTEM_FREQ); //DCO Freq is used for MCLK, SMCLK upon reset

    // Configuring UART Module
    UART_initModule(ESP32_UART_BASE, &espConfig);

    // Enable UART module
    UART_enableModule(ESP32_UART_BASE);

    // Enabling interrupts
    UART_enableInterrupt(ESP32_UART_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(ESP32_UART_INT);
    //Interrupt_enableSleepOnIsrExit();

}
*/


void configureGPSUART(void){
    GPIO_setAsPeripheralModuleFunctionInputPin(GPS_UART_GPIO_PORT,
    		CONSOLE_UART_RX | CONSOLE_UART_TX, GPIO_PRIMARY_MODULE_FUNCTION);

    CS_setDCOCenteredFrequency(SYSTEM_FREQ); //DCO Freq is used for MCLK, SMCLK upon reset

    UART_initModule(GPS_UART_BASE, &gpsConfig);

    UART_enableModule(GPS_UART_BASE);

    //UART_enableInterrupt(CONSOLE_UART_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    //Interrupt_enableInterrupt(CONSOLE_UART_INT);
}


void configureGPSTimer(void){
	Timer_A_configureUpMode(GPS_TIMER_BASE, &upConfig);
	Interrupt_enableInterrupt(GPS_TIMER_INT);

	Timer_A_startCounter(GPS_TIMER_BASE, TIMER_A_UP_MODE);
}


void configureFTDIUART(void){
    GPIO_setAsPeripheralModuleFunctionInputPin(FTDI_UART_GPIO_PORT,
    		FTDI_UART_RX | FTDI_UART_TX, GPIO_PRIMARY_MODULE_FUNCTION);

    CS_setDCOCenteredFrequency(SYSTEM_FREQ); //DCO Freq is used for MCLK, SMCLK upon reset

    UART_initModule(FTDI_UART_BASE, &ftdiConfig);

    UART_enableModule(FTDI_UART_BASE);

    UART_enableInterrupt(FTDI_UART_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    Interrupt_enableInterrupt(FTDI_UART_INT);

}


void configureSDCard(void){
    Board_initGeneral();

    SDFatFS_init();

    /* add_device() should be called once and is used for all media types */
    add_device(fatfsPrefix, _MSA, ffcio_open, ffcio_close, ffcio_read,
        ffcio_write, ffcio_lseek, ffcio_unlink, ffcio_rename);

    /* Mount and register the SD Card */
    sdfatfsHandle = SDFatFS_open(Board_SDFatFS0, DRIVE_NUM);
    if (sdfatfsHandle == NULL) {

        while (1); //TODO handle sdcard failure case
    }

}


void configureI2CBus(void){

	//TODO SD Card now uses these pins, switch I2C to UCB1!!!
	GPIO_setAsPeripheralModuleFunctionInputPin(I2C_BUS_GPIO_PORT,
			I2C_BUS_SCL + I2C_BUS_SDA, GPIO_PRIMARY_MODULE_FUNCTION);


	/* Initializing I2C Master to SMCLK at 400khz with no autostop */
	I2C_initMaster(I2C_BUS_BASE, &i2cBusConfig);

	/* Specify slave address */
	I2C_setSlaveAddress(I2C_BUS_BASE, DEFAULT_SLAVE_ADDRESS);

	I2C_setMode(I2C_BUS_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

	/* Enable I2C Module to start operations */
	I2C_enableModule(I2C_BUS_BASE);


	/* Enable and clear the interrupt flag */
	I2C_clearInterruptFlag(I2C_BUS_BASE,EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
			EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_RECEIVE_INTERRUPT0);

}


void initializeSlaves(void){

    int ii;
    while(!slavesInitialized){

	      /* Delay between Transmissions */
	      for (ii = 0; ii < 4000; ii++);

	      /* Making sure the last transaction has been completely sent out */
	      while (MAP_I2C_masterIsStopSent(I2C_BUS_BASE) == EUSCI_B_I2C_SENDING_STOP);

	      /* Send only a Start condition and the address byte */
	      if(slaveIndex < I2C_BUS_MAX_DAQS){
	    	  I2C_masterSendMultiByteStartWithTimeout(I2C_BUS_BASE,
	        		  DAQAddresses[slaveIndex++], 0x0000FFFF);
	    	  //TODO
	    	  	 /*
	    	  	  * If DAQ responds, add 1 to DAQ counter, else continue
	    	  	  */

	      }else{
	    	  slavesInitialized = true;
	      }
	//          MAP_Interrupt_enableSleepOnIsrExit();
    }
    //Only one slave, for demo purposes only
    //I2C_setSlaveAddress(I2C_BUS_BASE, 0x50);
    //Enable Interrupts after initialization
    I2C_enableInterrupt(I2C_BUS_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0 +
    			EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_RECEIVE_INTERRUPT0);
    Interrupt_enableInterrupt(I2C_BUS_INT);
}


void processCommand(char *CommandText){
	long Status;
	char *array[10];
	int i=0;

	array[i] = strtok(CommandText,":");

	while(array[i]!=NULL)
	{
		array[++i] = strtok(NULL,":");
	}

	memset(COMMAND, 0, sizeof(COMMAND));
	strncpy(COMMAND, array[0], (strlen(array[0])));

	UARTprintf(FTDI_UART_BASE, "CMD->%s\n\r",COMMAND);


	Status = CmdLineProcess(COMMAND);

	if(Status == CMDLINE_BAD_CMD)
	{
		UARTprintf(FTDI_UART_BASE, "Bad command!\n\r");
	}
}


void sendUARTString(uint32_t moduleInstance, char * msg){

	while(UART_queryStatusFlags(moduleInstance, EUSCI_A_UART_BUSY));

	//The null char is used to delimit strings in C
	while(*msg != '\0')
	{
		UART_transmitData(moduleInstance, *msg++);
	}

	//Return cursor to the start (left) of the screen
	UART_transmitData(moduleInstance, '\r');
	UART_transmitData(moduleInstance, '\n');

}


/********************************/
//	Interrupt Service Routines	//
/********************************/


//Console UART handler, lookup way to change function name without errors
void EUSCIA0_IRQHandler(void){

    uint32_t status = UART_getEnabledInterruptStatus(CONSOLE_UART_BASE);

    UART_clearInterruptFlag(CONSOLE_UART_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG){
    	consoleInput[inputIndex++] = MAP_UART_receiveData(CONSOLE_UART_BASE);
        if(consoleInput[inputIndex - 1] == '\r'){
        	MAP_UART_transmitData(CONSOLE_UART_BASE, '\n');
        	MAP_UART_transmitData(CONSOLE_UART_BASE, '\r');
        	consoleInput[inputIndex - 1] = '\0';
        	inputFlag = 1;

        	inputIndex = 0;

        }else{
        	MAP_UART_transmitData(CONSOLE_UART_BASE, consoleInput[inputIndex - 1]);
        }

     }

}




//ESP32 UART handler, lookup way to change function name without errors
/*
void EUSCIA2_IRQHandler(void){

    uint32_t status = MAP_UART_getEnabledInterruptStatus(ESP32_UART_BASE);

    MAP_UART_clearInterruptFlag(ESP32_UART_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG){
    	wifiInput[wifiIndex++] = MAP_UART_receiveData(ESP32_UART_BASE);
    	MAP_UART_transmitData(CONSOLE_UART_BASE, wifiInput[wifiIndex - 1]);
        if(wifiInput[wifiIndex - 1] == '\0'){
        	inputFlag = 1;
        	wifiFlag = 1;
        	wifiIndex = 0;

        }

     }

}
*/

//GPS UART Handler
void EUSCIA2_IRQHandler(void){
	uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);

	    MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);

	    if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
	    {
	        char receivedChar = MAP_UART_receiveData(EUSCI_A2_BASE);
	        //Check to see if we are within a valid transmission. If we are not, enter if statement.
	        if (record)
	        {
	            UARTprintf(CONSOLE_UART_BASE, "%c", receivedChar);
	            if (receivedChar == ',')
	            {
	                comma_counter++;
	                counter = 0;
	                if (comma_counter == 7)
	                {
	                    comma_counter = 0;
	                    if (gps.valid_Data)
	                    {
	                        MAP_Interrupt_disableInterrupt(INT_EUSCIA2);
	                        UARTprintf(CONSOLE_UART_BASE, "%s\r\n", "Data is Valid");
	                    }
	                    else
	                    {
	                        UARTprintf(CONSOLE_UART_BASE, "%s\r\n", "Data is Invalid");
	                        record = 0;
	                        check_format = 0;
	                        counter = 0;
	                        timeout++;
	                        if(timeout == TIME_WAIT)
	                        {
	                            MAP_Interrupt_disableInterrupt(INT_EUSCIA2);
	                            UARTprintf(CONSOLE_UART_BASE, "%s\r\n", "GPS has timed out, too many failed tries");
	                        }
	                    }
	                }
	            }
	            else if (receivedChar == ' ')
	            {
	                //ignore this
	            }
	            else
	            {
	                switch (comma_counter)
	                {
	                case 1:                        //utc time
	                    gps.utc[counter++] = receivedChar;
	                    break;
	                case 2:                        //latitude
	                    gps.latitude[counter++] = receivedChar;
	                    break;
	                case 3:                        //N/S
	                    gps.N_S = receivedChar;
	                    break;
	                case 4:                        //longitude
	                    gps.longtitude[counter++] = receivedChar;
	                    break;
	                case 5:
	                    gps.E_W = receivedChar;
	                    break;
	                case 6:
	                    if (receivedChar == '1' || receivedChar == '2'
	                            || receivedChar == '3')
	                        gps.valid_Data = 1;
	                    else
	                        gps.valid_Data = 0;
	                    break;
	                }
	            }
	        }
	        else if (check_format && counter < 5)
	        {
	            format[counter++] = receivedChar;
	            UARTprintf(CONSOLE_UART_BASE, "%c", format[counter - 1]);
	            if (counter == 5)
	            {
	                UARTprintf(CONSOLE_UART_BASE, "\r\n");
	                compare_value = strcmp(format, gps.nMEA_Record);
	                if (!compare_value)
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
	        else if (receivedChar == '$' && !check_format)
	        {
	            UARTprintf(CONSOLE_UART_BASE, "%c", receivedChar);
	            check_format = 1;
	        }
	    }


}




/*
 * FTDI UART handler, lookup way to change function name without errors
 */
void EUSCIA3_IRQHandler(void){

    uint32_t status = MAP_UART_getEnabledInterruptStatus(FTDI_UART_BASE);

    MAP_UART_clearInterruptFlag(FTDI_UART_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG){
    	consoleInput[inputIndex++] = MAP_UART_receiveData(FTDI_UART_BASE);
        if(consoleInput[inputIndex - 1] == '\r'){
        	MAP_UART_transmitData(FTDI_UART_BASE, '\n');
        	MAP_UART_transmitData(FTDI_UART_BASE, '\r');
        	consoleInput[inputIndex - 1] = '\0';
        	inputFlag = 1;

        	inputIndex = 0;

        }else{
        	MAP_UART_transmitData(FTDI_UART_BASE, consoleInput[inputIndex - 1]);
        }

     }

}



/*
 * I2C Bus handler
 */
void EUSCIB0_IRQHandler(void)
{
    uint_fast16_t status;

    status = MAP_I2C_getEnabledInterruptStatus(I2C_BUS_BASE);
    MAP_I2C_clearInterruptFlag(I2C_BUS_BASE, status);


    if (status & EUSCI_B_I2C_NAK_INTERRUPT){

        MAP_I2C_masterSendStart(I2C_BUS_BASE);
    }

    if (status & EUSCI_B_I2C_TRANSMIT_INTERRUPT0){
        /* Check the byte counter */
        if (masterToSlaveByteCtr){
            /* Send the next data and decrement the byte counter */
            if(I2C_masterSendMultiByteNextWithTimeout(I2C_BUS_BASE,
            		masterToSlavePacket[masterToSlaveIndex++], 0xFFFFFFFF)){
            	masterToSlaveByteCtr--;
            }
            //Decrement masterToSlaveIndex??
        }else{

            MAP_I2C_masterSendMultiByteStop(I2C_BUS_BASE);
            masterToSlaveIndex = 0;

        }
    }

    /* Receives bytes into the receive buffer. If we have received all bytes,
     * send a STOP condition */
    if (status & EUSCI_B_I2C_RECEIVE_INTERRUPT0){
//    	MAP_I2C_masterReceiveMultiByteStop(EUSCI_B0_BASE);
        if (xferIndex == NUM_OF_REC_BYTES - 2)
        {
//            MAP_I2C_disableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);
            MAP_I2C_enableInterrupt(I2C_BUS_BASE, EUSCI_B_I2C_STOP_INTERRUPT);

//            /*
//             * Switch order so that stop is being set during reception of last
//             * byte read byte so that next byte can be read.
//             */
            MAP_I2C_masterReceiveMultiByteStop(I2C_BUS_BASE);
            RXData[xferIndex++] = MAP_I2C_masterReceiveMultiByteNext(
            		I2C_BUS_BASE);
            cardFlag = true;
        } else
        {
            RXData[xferIndex++] = MAP_I2C_masterReceiveMultiByteNext(I2C_BUS_BASE);

        }
    }
    else if (status & EUSCI_B_I2C_STOP_INTERRUPT)
    {
//        MAP_Interrupt_disableSleepOnIsrExit();
//        MAP_I2C_disableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_STOP_INTERRUPT);
    }
}


void TA1_0_IRQHandler(void)
{
    seconds++;
    UARTprintf(EUSCI_A0_BASE, "Time until we check GPS: %i\r\n",
    TIME_WAIT - seconds);
    if (seconds == TIME_WAIT)
    {
        MAP_UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
        MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
        MAP_Interrupt_disableInterrupt(INT_TA1_0);
    }
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
}


/*
 *  ======== fatfs_getFatTime ========
 */
int32_t fatfs_getFatTime(void)
{
    /*
     *  FatFs uses this API to get the current time in FatTime format.  User's
     *  must implement this function based on their system's timekeeping
     *  mechanism.  See FatFs documentation for details on FatTime format.
     */
    /* Jan 1 2017 00:00:00 */
    return (0x4A210000);
}

