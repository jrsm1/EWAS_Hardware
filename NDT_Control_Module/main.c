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


/* GPS UART Macros*/
#define GPS_UART_BASE				EUSCI_A2_BASE
#define GPS_UART_GPIO_PORT			GPIO_PORT_P3
#define GPS_UART_INT				INT_EUSCIA2
#define GPS_UART_RX					GPIO_PIN2
#define GPS_UART_TX					GPIO_PIN3


/* GPS Timer Macros*/
#define MAX_FAILS 					5
#define GPS_TIMER_PERIOD    			0x8000
#define TIME_WAIT 					15
#define MAX_RETRIES 				3
#define GPS_TIMER_BASE				TIMER_A1_BASE
#define GPS_TIMER_INT				INT_TA1_0

/* Solenoid Macros */
#define SOLENOID_TIMER_PERIOD    	0x4000
#define SOLENOID_TIMER_BASE			TIMER_A0_BASE
#define SOLENOID_TIMER_INT			INT_TA0_0

/* FTDI UART Macros */
#define FTDI_UART_BASE				EUSCI_A1_BASE
#define FTDI_UART_GPIO_PORT			GPIO_PORT_P2
#define FTDI_UART_INT				INT_EUSCIA1
#define FTDI_UART_RX				GPIO_PIN2
#define FTDI_UART_TX				GPIO_PIN3


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
void configureGPSUART(void);
void configureGPSTimer(void);
void configureSolenoidTimer(void);
void configureRTC();
void configureFTDIUART(void);
void configureI2CBus(void);
void configureSDCard(void);
void initializeSlaves(void);
void processCommand(char *CommandText);
void sendUARTString(uint32_t moduleInstance, char * msg);
void decodeInstruction();

/********************************/
//		Global Variables		//
/********************************/


static char consoleInput[BUFFER_SIZE];
static char COMMAND[BUFFER_SIZE];
char masterToSlavePacket[MASTER_TO_SLAVE_PACKET_SIZE];
uint8_t masterToSlaveIndex = 0;
uint8_t masterToSlaveByteCtr = 3;
static volatile uint8_t inputIndex = 0;
static volatile bool slavesInitialized = false;
static volatile uint8_t slaveIndex = 0;
static volatile uint8_t inputFlag = 0;
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
short timeout_timeout = 0;
char format[6];
short compare_value;
short stop_rtc;
char current_time[6];


typedef struct GPSformats
{
    char nMEA_Record[6];
    char utc[10];
    char latitude[10];
    char N_S;
    char longtitude[11];
    char E_W;
    short valid_Data;
    char transmission[36];
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



//FTDI Baudrate 230400bps
const eUSCI_UART_Config ftdiConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        3,                                     // BRDIV = 3
        4,                                      // UCxBRF = 4
        0,                                       // UCxBRS = 0
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
const Timer_A_UpModeConfig gpsTimerConfig = {
		TIMER_A_CLOCKSOURCE_ACLK,              // ACLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,          // ACLK/1 = 32.768kHz
        GPS_TIMER_PERIOD,                           // 32768 ticks
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
};

//Solenoid Timer Configuration
const Timer_A_UpModeConfig solenoidTimerConfig = {
        TIMER_A_CLOCKSOURCE_ACLK,              // ACLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,          // ACLK/1 = 32.768kHz
        SOLENOID_TIMER_PERIOD,                           // 32768 ticks
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
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
const char textarray[] = "For Testing Only";


///////////////////////////////////////

//////////////////
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

////////////

/********************************/
//		Main					//
/********************************/

int main(void)
{

	//Stop Watchdog Timer
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    memset(consoleInput, 0x00, BUFFER_SIZE);

    configureSDCard();

    //SD Card configuration changes MCLK, SMCLK, re-init to desired values
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);

    configureConsoleUART();
    configureFTDIUART();
    configureGPSUART();
    configureRTC();
    configureGPSTimer();
//    configureI2CBus();

//    initializeSlaves();


//	masterToSlavePacket[0] = 0xCA;
    Interrupt_enableMaster();


    while(true){

        if(inputFlag){
        	inputFlag = 0;
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


void configureGPSUART(void){
    GPIO_setAsPeripheralModuleFunctionInputPin(GPS_UART_GPIO_PORT,
    		CONSOLE_UART_RX | CONSOLE_UART_TX, GPIO_PRIMARY_MODULE_FUNCTION);

    CS_setDCOCenteredFrequency(SYSTEM_FREQ); //DCO Freq is used for MCLK, SMCLK upon reset

    UART_initModule(GPS_UART_BASE, &gpsConfig);

    UART_enableModule(GPS_UART_BASE);

    strcpy(gps.nMEA_Record, "GPGGA");
    gps.valid_Data = 0;

}


void configureGPSTimer(void){
	Timer_A_configureUpMode(GPS_TIMER_BASE, &gpsTimerConfig);
	Interrupt_enableInterrupt(GPS_TIMER_INT);

	Timer_A_startCounter(GPS_TIMER_BASE, TIMER_A_UP_MODE);
}

void configureSolenoidTimer(void){
//	Timer_A_configureUpMode(GPS_TIMER_BASE, &gpsTimerConfig);
//	Interrupt_enableInterrupt(GPS_TIMER_INT);
//
//	Timer_A_startCounter(GPS_TIMER_BASE, TIMER_A_UP_MODE);
//    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN4);
//    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
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


void configureRTC(){
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
	        GPIO_PIN0 | GPIO_PIN1,GPIO_PRIMARY_MODULE_FUNCTION);

	CS_setExternalClockSourceFrequency(32000, 48000000);
	CS_startLFXT(CS_LFXT_DRIVE3);

	/* Initializing RTC with default time */
	RTC_C_initCalendar(&defaultTime, RTC_C_FORMAT_BINARY);
	RTC_C_clearInterruptFlag(RTC_C_CLOCK_READ_READY_INTERRUPT);     //Clears Read Ready Interrupt Flag
	RTC_C_enableInterrupt(RTC_C_CLOCK_READ_READY_INTERRUPT);        //Enables Read Ready Interupt Flag
	RTC_C_startClock();
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

	trans_count = 0;
	while(UART_queryStatusFlags(moduleInstance, EUSCI_A_UART_BUSY));

	//The null char is used to delimit strings in C
	while(*msg != '\0'){
		UART_transmitData(moduleInstance, (uint_fast8_t) *msg++);
		trans_count++;
	}

	//Return cursor to the start (left) of the screen
	UART_transmitData(moduleInstance, (uint_fast8_t) '\r');
	UART_transmitData(moduleInstance, (uint_fast8_t) '\n');

}


void transmitStringData(char *string){

    trans_count = 0;
    while(*string){
        MAP_UART_transmitData(EUSCI_A1_BASE, (uint_fast8_t) *string++);
        trans_count++;
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

void getCurrentTime(void)
{
    if(MAP_RTC_C_getEnabledInterruptStatus() & RTC_C_CLOCK_READ_READY_INTERRUPT)
    {
        RTC_C_Calendar currTime = RTC_C_getCalendarTime();
        int num = currTime.hours*10000 + currTime.minutes*100 + currTime.seconds;
	sprintf(current_time, "%i", num);
    }
}


void updateRTC(void)
{
    MAP_RTC_C_holdClock();                                              //Disables RTC functionality to modify registers
//    printRTCCurrentTime();                                                 //Displays current time in terminal
    RTC_C_Calendar currTime = RTC_C_getCalendarTime();                  //Gets the current calendar date
    uint_fast8_t hour = (gps.transmission[0] - 0x30)*10 + (gps.transmission[1] - 0x30);           //constructs decimal value from separate ascii characters
    uint_fast8_t minute = (gps.transmission[2] - 0x30)*10 + (gps.transmission[3] - 0x30);
    uint_fast8_t second = (gps.transmission[4] - 0x30)*10 + (gps.transmission[5] - 0x30);
    currTime.hours = hour;                                              //Updates struct hour, minute, and second with GPS acquired time
    currTime.minutes= minute;
    currTime.seconds = second;
    MAP_RTC_C_initCalendar(&currTime, RTC_C_FORMAT_BINARY);             //initializes RTC with updated struct
    MAP_RTC_C_clearInterruptFlag(RTC_C_CLOCK_READ_READY_INTERRUPT);     //Clears Read Ready Interrupt Flag
    MAP_RTC_C_enableInterrupt(RTC_C_CLOCK_READ_READY_INTERRUPT);        //Enables Read Ready Interupt Flag
    MAP_RTC_C_startClock();                                             //Starts Real Time Clock
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


void EUSCIA1_IRQHandler(void)
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus(FTDI_UART_BASE);

    UART_clearInterruptFlag(FTDI_UART_BASE, status);

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


//GPS UART Handler
void EUSCIA2_IRQHandler(void){
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
    MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);
    if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        char receivedChar = MAP_UART_receiveData(EUSCI_A2_BASE);        //Stores the received character
        if (record)                                                     //State C, store incoming data
        {
            if (receivedChar == ',')
            {
                if(comma_counter)
                    gps.transmission[counter++] = receivedChar;
                comma_counter++;                                        //Increase comma_counter, tells us which parameter is next
                if (comma_counter == 7)                                 //If comma_counter equals seven, We have stored all of the revelant data
                {
                    comma_counter = 0;
                    if (gps.valid_Data)                                 //Verify if the data is valid
                    {
                        MAP_Interrupt_disableInterrupt(INT_EUSCIA2);
                        updateRTC();                                    //Data is valid, update RTC

                    }
                    else                                                //Data is invalid, reset variables to wait for next message
                    {
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
                            if (!(timeout_timeout == MAX_RETRIES))         //If we reach the maximum amount of tries, stop
                            {
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
                if(comma_counter == 6)
                {
                    if (receivedChar == '1' || receivedChar == '2'
                            || receivedChar == '3')
                        gps.valid_Data = 1;
                    else
                        gps.valid_Data = 0;
                }
                else
                    gps.transmission[counter++] = receivedChar;
            }
        }
        else if (check_format && counter < 5)                           //If statement that handles the verification of the message format
        {
            format[counter++] = receivedChar;                           //Adds character to an array to form the five character format
            UARTprintf(EUSCI_A0_BASE, "%c", format[counter - 1]);
            if (counter == 5)                                           //If we have all five characters, compare
            {
                UARTprintf(EUSCI_A0_BASE, "\r\n");
                compare_value = strcmp(format, gps.nMEA_Record);        //Compares message format with desired format(GPGGA)
                check_format = 0;
                counter = 0;
                if (!compare_value)                                     //If it is the desired format, start storing the incoming data.
                    record = 1;
                else                                                    //Else restart the check
                    record = 0;
            }
        }
        else if (receivedChar == '$' && !check_format)                  //Checks if the character is the start delimiter
            check_format = 1;                                           //Next five characters is the message format, check those.
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


void TA0_0_IRQHandler(void)
{
	//Start Command
//	MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN4);
//	MAP_Interrupt_enableInterrupt(INT_TA1_0);
//	MAP_Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

	//ISR Code
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN4);
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
    MAP_Interrupt_disableInterrupt(INT_TA0_0);
    MAP_Timer_A_stopTimer(TIMER_A0_BASE);
}


void TA1_0_IRQHandler(void)
{
    seconds++;                                                          //Increases seconds by one
    UARTprintf(EUSCI_A0_BASE, "Time until we check GPS: %i\r\n",
           TIME_WAIT - seconds);                                        //Displays how much time is left before checking
    if (seconds >= TIME_WAIT)                                               //Checks if we reached or gone past TIME_WAIT
    {
        MAP_UART_enableInterrupt(EUSCI_A2_BASE,
                                 EUSCI_A_UART_RECEIVE_INTERRUPT);       //Enables GPS UART receive interrupt
        MAP_Interrupt_enableInterrupt(INT_EUSCIA2);                     //Enables interrupts for GPS UART
        MAP_Interrupt_disableInterrupt(INT_TA1_0);                      //Disables timer interrupt
    }
    MAP_Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,
    TIMER_A_CAPTURECOMPARE_REGISTER_0);
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

//////////////////////////////////////////////////////////////////////////////////////
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
    }else if(inst == (uint8_t) 133){
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

    }
    else if(inst == (uint8_t) 134){ //instruction to send all the data last acquired
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
        //Set Select lines on muxes for voltage references
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


