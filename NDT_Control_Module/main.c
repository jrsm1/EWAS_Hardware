/*******************************/
//      Includes                //
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
//      Defines                 //
/********************************/

//
/* Console UART Macros */
//
#define CONSOLE_UART_BASE           EUSCI_A0_BASE
#define CONSOLE_UART_GPIO_PORT      GPIO_PORT_P1
#define CONSOLE_UART_INT            INT_EUSCIA0
#define CONSOLE_UART_RX             GPIO_PIN2
#define CONSOLE_UART_TX             GPIO_PIN3


//
/* GPS UART Macros*/
//
#define GPS_UART_BASE               EUSCI_A2_BASE
#define GPS_UART_GPIO_PORT          GPIO_PORT_P3
#define GPS_UART_INT                INT_EUSCIA2
#define GPS_UART_RX                 GPIO_PIN2
#define GPS_UART_TX                 GPIO_PIN3


//
/* GPS Timer Macros*/
//
#define MAX_FAILS                   5
#define GPS_TIMER_PERIOD            0x8000
#define TIME_WAIT                   15
#define MAX_RETRIES                 3
#define GPS_TIMER_BASE              TIMER_A1_BASE
#define GPS_TIMER_INT               INT_TA1_0


//
/* Solenoid Macros */
//
#define SOLENOID_GPIO_PORT          GPIO_PORT_P2
#define SOLENOID_GPIO_PIN           GPIO_PIN4


//
/* FTDI UART Macros */
//
#define FTDI_UART_BASE              EUSCI_A1_BASE
#define FTDI_UART_GPIO_PORT         GPIO_PORT_P2
#define FTDI_UART_INT               INT_EUSCIA1
#define FTDI_UART_RX                GPIO_PIN2
#define FTDI_UART_TX                GPIO_PIN3


//
/* I2C Bus Macros */
//
#define DEFAULT_SLAVE_ADDRESS       0x60 //0xC0 >> 1
#define I2C_BUS_BASE                EUSCI_B2_BASE
#define I2C_BUS_GPIO_PORT           GPIO_PORT_P3
#define I2C_BUS_INT                 INT_EUSCIB2
#define I2C_BUS_SCL                 GPIO_PIN6
#define I2C_BUS_SDA                 GPIO_PIN7
#define I2C_BUS_MAX_DAQS            8

#define I2C_DAQ1_GPIO_PORT          GPIO_PORT_P4
#define I2C_DAQ1_GPIO_PIN           GPIO_PIN0
#define I2C_DAQ2_GPIO_PORT          GPIO_PORT_P4
#define I2C_DAQ2_GPIO_PIN           GPIO_PIN1
#define I2C_DAQ3_GPIO_PORT          GPIO_PORT_P4
#define I2C_DAQ3_GPIO_PIN           GPIO_PIN2
#define I2C_DAQ4_GPIO_PORT          GPIO_PORT_P5
#define I2C_DAQ4_GPIO_PIN           GPIO_PIN3

#define I2C_DAQ5_GPIO_PORT          GPIO_PORT_P4
#define I2C_DAQ5_GPIO_PIN           GPIO_PIN4
#define I2C_DAQ6_GPIO_PORT          GPIO_PORT_P5
#define I2C_DAQ6_GPIO_PIN           GPIO_PIN5
#define I2C_DAQ7_GPIO_PORT          GPIO_PORT_P5
#define I2C_DAQ7_GPIO_PIN           GPIO_PIN6
#define I2C_DAQ8_GPIO_PORT          GPIO_PORT_P5
#define I2C_DAQ8_GPIO_PIN           GPIO_PIN7

#define I2C_DAQ_DRDY_PORT           GPIO_PORT_P6
#define I2C_DAQ_DRDY_INT            INT_PORT6
#define I2C_DAQ1_DRDY_PIN           GPIO_PIN0
#define I2C_DAQ2_DRDY_PIN           GPIO_PIN1
#define I2C_DAQ3_DRDY_PIN           GPIO_PIN2

#define I2C_DAQ4_DRDY_PIN           GPIO_PIN3
#define I2C_DAQ5_DRDY_PIN           GPIO_PIN4
#define I2C_DAQ6_DRDY_PIN           GPIO_PIN5
#define I2C_DAQ7_DRDY_PIN           GPIO_PIN6
#define I2C_DAQ8_DRDY_PIN           GPIO_PIN7

#define DATA_BUF_SIZE               1200


//
/* General System Macros */
//
#define BUFFER_SIZE                 128
#define SYSTEM_FREQ                 CS_DCO_FREQUENCY_12
#define DAQ_RESET_PORT              GPIO_PORT_P3
#define DAQ_RESET_PIN               GPIO_PIN0


//
/* SD Card Macros */
//
/* Buffer size used for the file copy process */
#ifndef CPY_BUFF_SIZE
#define CPY_BUFF_SIZE               4096
#endif


/* String conversion macro */
#define STR_(n)                     #n
#define STR(n)                      STR_(n)


/* Drive number used for FatFs */
#define DRIVE_NUM                   0


/********************************/
//      Function Prototypes     //
/********************************/

void configureConsoleUART(void);
void configureGPSUART(void);
void configureGPSTimer(void);
void configureSolenoid(void);
void configureDAQReset(void);
void configureRTC();
void configureDataReadyPort(void);
void configureFTDIUART(void);
void configureI2CBus(void);
void configureSDCard(void);
void initializeSlaves(void);
void processCommand(char *CommandText);
void sendUARTString(uint32_t moduleInstance, char * msg);
void decodeInstruction();


/********************************/
//      Global Variables        //
/********************************/

//
/* General Variables */
//
static char consoleInput[BUFFER_SIZE];
static char COMMAND[BUFFER_SIZE];
static volatile uint8_t inputIndex = 0;
static volatile uint8_t slaveIndex = 0;
static volatile uint32_t bytesToRecCounter = 0;
static volatile uint32_t bytesToRec = 0;
static volatile uint8_t inputFlag = 0;
static volatile uint8_t appFlag = 0;
static volatile uint8_t dreadyFlag = 0;
const uint8_t DAQAddresses[] = {0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57};
const uint16_t samplingRates[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 20000};
static uint8_t ADCData[DATA_BUF_SIZE];
static volatile uint16_t xferIndex = 0;
static volatile uint8_t bufferFull = 0;


//
/* GPS Global Variables */
//
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
* Default is UNIX Epoch Time: Thursday, January 1st 1970 00:00:00 AM
*/
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
/* SD card global variables */
//
SDFatFS_Handle sdfatfsHandle;
FILE *src, *dst;

/* File name prefix for this filesystem for use with TI C RTS */
char fatfsPrefix[] = "fat";

const char daq1[] = "fat:"STR(DRIVE_NUM)":DAQ1.txt";
const char daq2[] = "fat:"STR(DRIVE_NUM)":DAQ2.txt";
const char daq3[] = "fat:"STR(DRIVE_NUM)":DAQ3.txt";
const char daq4[] = "fat:"STR(DRIVE_NUM)":DAQ4.txt";
const char daq5[] = "fat:"STR(DRIVE_NUM)":DAQ5.txt";
const char daq6[] = "fat:"STR(DRIVE_NUM)":DAQ6.txt";
const char daq7[] = "fat:"STR(DRIVE_NUM)":DAQ7.txt";
const char daq8[] = "fat:"STR(DRIVE_NUM)":DAQ8.txt";


//
/* PC Application global variables*/
//
uint8_t rxdata[1000];
uint_fast8_t received_byte;
char test_char;
int rcount = 0;
uint8_t modules_connected_code[8] = {1, 0, 0, 0, 0, 0, 0, 0}; //TODO Hardwired, must be configured during slave init
char configuration[4500];
int conf_place =  0;
uint8_t live1[3] = {1, 1, 1};
uint8_t live2[3] = {1, 1, 1};
char *gps_data = "gps data";
uint8_t valid_conf = 0;
uint8_t diagnosis = 0;


// module sensor visualization
uint8_t store_enable = 0; //Flag for determining if DAta shall be stored in sd card
uint8_t vis_sens1 = 0;
uint8_t vis_mod2 = 0;
uint8_t vis_sens2 = 0;


uint8_t sensors_enabled[32];

//
/* recording parameters */
//
uint8_t sample_rate;
uint8_t cutoff;
uint8_t gain;
short duration;
uint8_t start_delay;

char experiment_name[20];
char localization_name[20];


//
/*status variables */
//
uint8_t recorded = 0;
uint8_t stored = 0;
uint8_t gps_synched = 0;


//
/* Module Configuration Global Variables */
//
const eUSCI_UART_Config consoleConfig =         //Console Baudrate 230400bps
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        3,                                     // BRDIV = 3
        4,                                       // UCxBRF = 4
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // LSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};



const eUSCI_UART_Config gpsConfig =             //GPS Baudrate 9600bps
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




const eUSCI_UART_Config ftdiConfig =            //FTDI Baudrate 230400bps
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
        EUSCI_B_I2C_SET_DATA_RATE_100KBPS,      // Desired I2C Clock of 100khz
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


/********************************/
//      Main                    //
/********************************/

int main(void){

    //Stop Watchdog Timer
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    //Flush buffers
    memset(consoleInput, 0x00, BUFFER_SIZE);
    memset((char *)ADCData, 0x00, sizeof(ADCData));

    //Configure SD Card first because SDInit changes clock configuration
    configureSDCard();

    //SD Card configuration changes MCLK, SMCLK, ACLK, re-init to desired values
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1); //DCO at 12MHz
    CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1); //ACLK at 32.768kHz

    configureConsoleUART();
    configureFTDIUART();
    configureGPSUART();
    configureRTC();

    configureGPSTimer();
    configureDAQReset();
    configureDataReadyPort();
    configureSolenoid();

    configureI2CBus();
    initializeSlaves();
    Interrupt_enableMaster();

    //Clear all files upon reset
    src = fopen(daq1, "w");
    fclose(src);
    src = fopen(daq2, "w");
    fclose(src);
    src = fopen(daq3, "w");
    fclose(src);
    src = fopen(daq4, "w");
    fclose(src);

    src = fopen(daq5, "w");
    fclose(src);
    src = fopen(daq6, "w");
    fclose(src);
    src = fopen(daq7, "w");
    fclose(src);
    src = fopen(daq8, "w");
    fclose(src);


    while(true){

        if(inputFlag){  //Command was received through console (putty) terminal
            inputFlag = 0;
            inputIndex = 0;
            processCommand(consoleInput);

        }else if(appFlag){ //Command was received from the PC application (from FTDI chip)
            appFlag = 0;
            decodeInstruction();

        }else if(dreadyFlag){ //Signal received from DAQ Module to initiate data request
            dreadyFlag = 0;
            processCommand("datareq");
        }
        else if(bufferFull){ //Global buffer for DAQ data is full, must write to SD Card before receiving more data
            bufferFull = 0;

            src = fopen(daq1, "a"); //TODO HARDWIRED, must open file of DAQ who sent the DRDY signal

            int i;
            for(i = 0; i < xferIndex; i++){
                putc(ADCData[i], src);  //Write buffer to DAQ file on SD card
            }

            memset(ADCData, 0x00, sizeof(ADCData)); //Flush ADCData

            fclose(src);

            xferIndex = 0;

            ADCData[xferIndex++] = EUSCI_B2->RXBUF; //Receive first byte of next buffer here, else we lose it

            if(bytesToRecCounter == bytesToRec - 1){ //All data was received, Replace bytesToRec with
                stored = 1;                         //NUM_OF_REC_BYTES for testing using the terminal
                bytesToRecCounter = 0;
                EUSCI_B2->IE &= ~BIT0;  //Disable I2C RX interrupt

            }else{ //More Data still left
                processCommand("datareq");
            }

        }

    }


}


/********************************/
//      Function Definitions    //
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

    strcpy(gps.nMEA_Record, "GPGGA"); //Minimum initialization for GPS struct
    gps.valid_Data = 0;

}


void configureGPSTimer(void){
    Timer_A_configureUpMode(GPS_TIMER_BASE, &gpsTimerConfig);
    Interrupt_enableInterrupt(GPS_TIMER_INT);

    Timer_A_startCounter(GPS_TIMER_BASE, TIMER_A_UP_MODE);
}


void configureSolenoid(void){
    GPIO_setAsOutputPin(SOLENOID_GPIO_PORT, SOLENOID_GPIO_PIN);

    GPIO_setOutputLowOnPin(SOLENOID_GPIO_PORT, SOLENOID_GPIO_PIN);

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

}


void configureDAQReset(void){

    GPIO_setAsOutputPin(DAQ_RESET_PORT, DAQ_RESET_PIN);

    GPIO_setOutputHighOnPin(DAQ_RESET_PORT, DAQ_RESET_PIN);
}


void configureDataReadyPort(void){
    GPIO_setAsInputPinWithPullDownResistor(I2C_DAQ_DRDY_PORT, I2C_DAQ1_DRDY_PIN | I2C_DAQ2_DRDY_PIN | I2C_DAQ3_DRDY_PIN
            | I2C_DAQ4_DRDY_PIN | I2C_DAQ5_DRDY_PIN | I2C_DAQ6_DRDY_PIN | I2C_DAQ7_DRDY_PIN | I2C_DAQ8_DRDY_PIN);

    GPIO_clearInterruptFlag(I2C_DAQ_DRDY_PORT, I2C_DAQ1_DRDY_PIN | I2C_DAQ2_DRDY_PIN | I2C_DAQ3_DRDY_PIN
            | I2C_DAQ4_DRDY_PIN | I2C_DAQ5_DRDY_PIN | I2C_DAQ6_DRDY_PIN | I2C_DAQ7_DRDY_PIN | I2C_DAQ8_DRDY_PIN);

    GPIO_enableInterrupt(I2C_DAQ_DRDY_PORT, I2C_DAQ1_DRDY_PIN | I2C_DAQ2_DRDY_PIN | I2C_DAQ3_DRDY_PIN
            | I2C_DAQ4_DRDY_PIN | I2C_DAQ5_DRDY_PIN | I2C_DAQ6_DRDY_PIN | I2C_DAQ7_DRDY_PIN | I2C_DAQ8_DRDY_PIN);

    Interrupt_enableInterrupt(I2C_DAQ_DRDY_INT);

}

void configureI2CBus(void){

    GPIO_setAsPeripheralModuleFunctionInputPin(I2C_BUS_GPIO_PORT,
            I2C_BUS_SCL + I2C_BUS_SDA, GPIO_PRIMARY_MODULE_FUNCTION);

    //GPIO Pins for I2C DAQ module enables
    GPIO_setAsOutputPin(I2C_DAQ1_GPIO_PORT, I2C_DAQ1_GPIO_PIN);
    GPIO_setOutputLowOnPin(I2C_DAQ1_GPIO_PORT, I2C_DAQ1_GPIO_PIN);

    GPIO_setAsOutputPin(I2C_DAQ2_GPIO_PORT, I2C_DAQ2_GPIO_PIN);
    GPIO_setOutputLowOnPin(I2C_DAQ2_GPIO_PORT, I2C_DAQ2_GPIO_PIN);

    GPIO_setAsOutputPin(I2C_DAQ3_GPIO_PORT, I2C_DAQ3_GPIO_PIN);
    GPIO_setOutputLowOnPin(I2C_DAQ3_GPIO_PORT, I2C_DAQ3_GPIO_PIN);

    GPIO_setAsOutputPin(I2C_DAQ4_GPIO_PORT, I2C_DAQ4_GPIO_PIN);
    GPIO_setOutputLowOnPin(I2C_DAQ4_GPIO_PORT, I2C_DAQ4_GPIO_PIN);

    GPIO_setAsOutputPin(I2C_DAQ5_GPIO_PORT, I2C_DAQ5_GPIO_PIN);
    GPIO_setOutputLowOnPin(I2C_DAQ5_GPIO_PORT, I2C_DAQ5_GPIO_PIN);

    GPIO_setAsOutputPin(I2C_DAQ6_GPIO_PORT, I2C_DAQ6_GPIO_PIN);
    GPIO_setOutputLowOnPin(I2C_DAQ6_GPIO_PORT, I2C_DAQ6_GPIO_PIN);

    GPIO_setAsOutputPin(I2C_DAQ7_GPIO_PORT, I2C_DAQ7_GPIO_PIN);
    GPIO_setOutputLowOnPin(I2C_DAQ7_GPIO_PORT, I2C_DAQ7_GPIO_PIN);

    GPIO_setAsOutputPin(I2C_DAQ8_GPIO_PORT, I2C_DAQ8_GPIO_PIN);
    GPIO_setOutputLowOnPin(I2C_DAQ8_GPIO_PORT, I2C_DAQ8_GPIO_PIN);

    /* Initializing I2C Master to SMCLK at 100khz with no autostop */
    I2C_initMaster(I2C_BUS_BASE, &i2cBusConfig);

    /* Specify slave address */
    I2C_setSlaveAddress(I2C_BUS_BASE, DEFAULT_SLAVE_ADDRESS);

    I2C_setMode(I2C_BUS_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

    /* Enable I2C Module to start operations */
    I2C_enableModule(I2C_BUS_BASE);


    /* Enable and clear the interrupt flag */
    I2C_clearInterruptFlag(I2C_BUS_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);

}


void configureRTC(){
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ, GPIO_PIN0 | GPIO_PIN1,
                                                    GPIO_PRIMARY_MODULE_FUNCTION);

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
    bool slavesInitialized = false;
    processCommand("reset");    //Reset all DAQ modules before sending DAQ addresses

    while(!slavesInitialized){

          /* Delay between Transmissions */
          for (ii = 0; ii < 4000; ii++);

          /* Making sure the last transaction has been completely sent out */
          while (MAP_I2C_masterIsStopSent(I2C_BUS_BASE) == EUSCI_B_I2C_SENDING_STOP);

          /* Send only a Start condition and the address byte */
          if(slaveIndex < I2C_BUS_MAX_DAQS){

              if(slaveIndex == 0){
                  GPIO_setOutputHighOnPin(I2C_DAQ1_GPIO_PORT, I2C_DAQ1_GPIO_PIN);

              }else if(slaveIndex == 1){
                  GPIO_setOutputHighOnPin(I2C_DAQ2_GPIO_PORT, I2C_DAQ2_GPIO_PIN);

              }else if(slaveIndex == 2){
                  GPIO_setOutputHighOnPin(I2C_DAQ3_GPIO_PORT, I2C_DAQ3_GPIO_PIN);

              }else if(slaveIndex == 3){
                  GPIO_setOutputHighOnPin(I2C_DAQ4_GPIO_PORT, I2C_DAQ4_GPIO_PIN);

              }else if(slaveIndex == 4){
                  GPIO_setOutputHighOnPin(I2C_DAQ5_GPIO_PORT, I2C_DAQ5_GPIO_PIN);

              }else if(slaveIndex == 5){
                  GPIO_setOutputHighOnPin(I2C_DAQ6_GPIO_PORT, I2C_DAQ6_GPIO_PIN);

              }else if(slaveIndex == 6){
                  GPIO_setOutputHighOnPin(I2C_DAQ7_GPIO_PORT, I2C_DAQ7_GPIO_PIN);

              }else if(slaveIndex == 7){
                  GPIO_setOutputHighOnPin(I2C_DAQ8_GPIO_PORT, I2C_DAQ8_GPIO_PIN);

              }

//                  I2C_masterSendSingleByteWithTimeout(I2C_BUS_BASE,
//                                 DAQAddresses[slaveIndex], 0x0000FFFF);
              //TODO Figure out how to initialize up to 8 DAQ Modules in a specified order
              //and properly initialize modules_connected
              /*
                 If DAQ responds, set bit on modules connected array, else
                 leave bit cleared and continue with the next iteration
               */

              if(slaveIndex == 0){
                  GPIO_setOutputLowOnPin(I2C_DAQ1_GPIO_PORT, I2C_DAQ1_GPIO_PIN);

              }else if(slaveIndex == 1){
                  GPIO_setOutputLowOnPin(I2C_DAQ2_GPIO_PORT, I2C_DAQ2_GPIO_PIN);

              }else if(slaveIndex == 2){
                  GPIO_setOutputLowOnPin(I2C_DAQ3_GPIO_PORT, I2C_DAQ3_GPIO_PIN);

              }else if(slaveIndex == 3){
                  GPIO_setOutputLowOnPin(I2C_DAQ4_GPIO_PORT, I2C_DAQ4_GPIO_PIN);

              }else if(slaveIndex == 4){
                  GPIO_setOutputLowOnPin(I2C_DAQ5_GPIO_PORT, I2C_DAQ5_GPIO_PIN);

              }else if(slaveIndex == 5){
                  GPIO_setOutputLowOnPin(I2C_DAQ6_GPIO_PORT, I2C_DAQ6_GPIO_PIN);

              }else if(slaveIndex == 6){
                  GPIO_setOutputLowOnPin(I2C_DAQ7_GPIO_PORT, I2C_DAQ7_GPIO_PIN);

              }else if(slaveIndex == 7){
                  GPIO_setOutputLowOnPin(I2C_DAQ8_GPIO_PORT, I2C_DAQ8_GPIO_PIN);

              }
              slaveIndex++;

          }else{
              slavesInitialized = true;
          }

    }
    //Only one slave, for demo purposes only
    I2C_setSlaveAddress(I2C_BUS_BASE, 0x50); //TODO HARDWIRED
    modules_connected_code[0] = 1; //TODO HARDWIRED

    Interrupt_enableInterrupt(I2C_BUS_INT);
}


void processCommand(char *CommandText){
    char *array[10];
    int i=0;

    array[i] = strtok(CommandText,":");

    while(array[i]!=NULL)
    {
        array[++i] = strtok(NULL,":");
    }

    memset(COMMAND, 0, sizeof(COMMAND));
    strncpy(COMMAND, array[0], (strlen(array[0])));

    CmdLineProcess(COMMAND);

}


void sendUARTString(uint32_t moduleInstance, char * msg){

    while(UART_queryStatusFlags(moduleInstance, EUSCI_A_UART_BUSY));

    //The null char is used to delimit strings in C
    while(*msg != '\0'){
        UART_transmitData(moduleInstance, (uint_fast8_t) *msg++);

    }

    //Return cursor to the start (left) of the screen
    UART_transmitData(moduleInstance, (uint_fast8_t) '\r'); //Carriage Return
    UART_transmitData(moduleInstance, (uint_fast8_t) '\n'); //New Line

}


void transmitByteData(uint8_t byte){
    MAP_UART_transmitData(FTDI_UART_BASE, (uint_fast8_t) byte);
    MAP_UART_transmitData(FTDI_UART_BASE, (uint_fast8_t) '\r'); //Carriage Return
    MAP_UART_transmitData(FTDI_UART_BASE, (uint_fast8_t) '\n'); //New Line
}


void transmitNByteData(uint8_t byte){
    MAP_UART_transmitData(FTDI_UART_BASE, (uint_fast8_t) byte);
}


void transmitTerminationSequence(){

    //Byte sequence used for the PC Application
    MAP_UART_transmitData(FTDI_UART_BASE, 0xAA);
    MAP_UART_transmitData(FTDI_UART_BASE, 0xBB);
    MAP_UART_transmitData(FTDI_UART_BASE, 0xAA);
    MAP_UART_transmitData(FTDI_UART_BASE, 0xBB);
    MAP_UART_transmitData(FTDI_UART_BASE, 0x0D);
    MAP_UART_transmitData(FTDI_UART_BASE, 0x0A);
}


void getCurrentTime(void){

    if(MAP_RTC_C_getEnabledInterruptStatus() & RTC_C_CLOCK_READ_READY_INTERRUPT){
        RTC_C_Calendar currTime = RTC_C_getCalendarTime();
        int num = currTime.hours*10000 + currTime.minutes*100 + currTime.seconds;
        sprintf(current_time, "%i", num);
    }
}


void updateRTC(void)
{
    MAP_RTC_C_holdClock();  //Disables RTC functionality to modify registers
    RTC_C_Calendar currTime = RTC_C_getCalendarTime();  //Gets the current calendar date
    uint_fast8_t hour = (gps.transmission[0] - 0x30)*10 + (gps.transmission[1] - 0x30); //Constructs decimal value from separate ascii characters
    uint_fast8_t minute = (gps.transmission[2] - 0x30)*10 + (gps.transmission[3] - 0x30);
    uint_fast8_t second = (gps.transmission[4] - 0x30)*10 + (gps.transmission[5] - 0x30);

    currTime.hours = hour;
    currTime.minutes= minute;
    currTime.seconds = second;
    MAP_RTC_C_initCalendar(&currTime, RTC_C_FORMAT_BINARY); //initializes RTC with updated struct
    MAP_RTC_C_clearInterruptFlag(RTC_C_CLOCK_READ_READY_INTERRUPT); //Clears Read Ready Interrupt Flag
//    MAP_RTC_C_enableInterrupt(RTC_C_CLOCK_READ_READY_INTERRUPT);        //Enables Read Ready Interupt Flag
    MAP_RTC_C_startClock(); //Starts Real Time Clock
}


/********************************/
//  Interrupt Service Routines  //
/********************************/

void PORT6_IRQHandler(void)
{
    //TODO Figure out a way to manage a DRDY conflict between multiple DAQs

    uint32_t status = MAP_GPIO_getEnabledInterruptStatus(I2C_DAQ_DRDY_PORT);
    GPIO_clearInterruptFlag(I2C_DAQ_DRDY_PORT, status);

    if (status & GPIO_PIN0){
        I2C_setSlaveAddress(I2C_BUS_BASE, 0x50);
        dreadyFlag = 1;

    }else if (status & GPIO_PIN1){
        I2C_setSlaveAddress(I2C_BUS_BASE, 0x51);
        dreadyFlag = 1;

    }else if (status & GPIO_PIN2){
        I2C_setSlaveAddress(I2C_BUS_BASE, 0x52);
        dreadyFlag = 1;

    }else if (status & GPIO_PIN3){
        I2C_setSlaveAddress(I2C_BUS_BASE, 0x53);
        dreadyFlag = 1;

    }else if (status & GPIO_PIN4){
        I2C_setSlaveAddress(I2C_BUS_BASE, 0x54);
        dreadyFlag = 1;

    }else if (status & GPIO_PIN5){
        I2C_setSlaveAddress(I2C_BUS_BASE, 0x55);
        dreadyFlag = 1;

    }else if (status & GPIO_PIN6){
        I2C_setSlaveAddress(I2C_BUS_BASE, 0x56);
        dreadyFlag = 1;

    }else if (status & GPIO_PIN7){
        I2C_setSlaveAddress(I2C_BUS_BASE, 0x57);
        dreadyFlag = 1;

    }


}

//Console UART handler
void EUSCIA0_IRQHandler(void){

    uint32_t status = UART_getEnabledInterruptStatus(CONSOLE_UART_BASE);

    UART_clearInterruptFlag(CONSOLE_UART_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG){
        consoleInput[inputIndex++] = MAP_UART_receiveData(CONSOLE_UART_BASE); //Fill input buffer

        if(consoleInput[inputIndex - 1] == '\r'){   //Until enter is pressed, enter is equal to \r\n
            MAP_UART_transmitData(CONSOLE_UART_BASE, '\n');
            MAP_UART_transmitData(CONSOLE_UART_BASE, '\r');
            consoleInput[inputIndex - 1] = '\0';
            inputFlag = 1;

            inputIndex = 0;

        }else{
            MAP_UART_transmitData(CONSOLE_UART_BASE, consoleInput[inputIndex - 1]); //Echo back to console
        }

     }

}

//FTDI UART Handler
void EUSCIA1_IRQHandler(void)
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus(FTDI_UART_BASE);

    UART_clearInterruptFlag(FTDI_UART_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
    {
        received_byte = MAP_UART_receiveData(EUSCI_A1_BASE); //Fill input buffer
        rxdata[rcount] = received_byte;
    }
    if(received_byte == (uint8_t) 13){ //13 -> Carriage Return
        appFlag = 1;

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
        char receivedChar = MAP_UART_receiveData(EUSCI_A2_BASE); //Stores the received character
        if (record)             //State C, store incoming data
        {
            if (receivedChar == ',')
            {
                if(comma_counter)
                    gps.transmission[counter++] = receivedChar;
                comma_counter++;          //Increase comma_counter, tells us which parameter is next
                if (comma_counter == 7)       //If comma_counter equals seven, We have stored all of the revelant data
                {
                    comma_counter = 0;
                    if (gps.valid_Data)         //Verify if the data is valid
                    {
                        MAP_Interrupt_disableInterrupt(INT_EUSCIA2);
                        updateRTC();              //Data is valid, update RTC
                        gps_synched = 1;
                    }
                    else        //Data is invalid, reset variables to wait for next message
                    {
                        record = 0;
                        check_format = 0;
                        counter = 0;
                        timeout++;           //Variable to limit the amount of tries the gps gets to sync
                        if (timeout == MAX_FAILS)   //If we reach the maximum amount of tries, restart with the timer count
                        {
                            MAP_Interrupt_disableInterrupt(INT_EUSCIA2);
                            timeout_timeout++;      //Variable to limit the amount of resets the gps has
                            seconds = 0;
                            timeout = 0;
                            if (!(timeout_timeout == MAX_RETRIES))   //If we reach the maximum amount of tries, stop
                            {
                                MAP_Interrupt_enableInterrupt(INT_TA1_0);
                            }
                        }
                    }
                }
            }
            else if (receivedChar == ' ')//Ignore space characters
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
        else if (check_format && counter < 5)  //If statement that handles the verification of the message format
        {
            format[counter++] = receivedChar;   //Adds character to an array to form the five character format
            UARTprintf(EUSCI_A0_BASE, "%c", format[counter - 1]);
            if (counter == 5)   //If we have all five characters, compare
            {
                UARTprintf(EUSCI_A0_BASE, "\r\n");
                compare_value = strcmp(format, gps.nMEA_Record);//Compares message format with desired format(GPGGA)
                check_format = 0;
                counter = 0;
                if (!compare_value)    //If it is the desired format, start storing the incoming data.
                    record = 1;
                else             //Else restart the check
                    record = 0;
            }
        }
        else if (receivedChar == '$' && !check_format)   //Checks if the character is the start delimiter
            check_format = 1;   //Next five characters is the message format, check those.
    }
}


/*
 * I2C Bus handler
 */
void EUSCIB2_IRQHandler(void)
{
    uint_fast16_t status;

    status = MAP_I2C_getEnabledInterruptStatus(I2C_BUS_BASE);
    MAP_I2C_clearInterruptFlag(I2C_BUS_BASE, status);

    /* Receives bytes into the receive buffer. If we have received all bytes,
     * send a STOP condition */
    if (status & EUSCI_B_I2C_RECEIVE_INTERRUPT0){

        if(bytesToRecCounter == bytesToRec - 2){ //For some reason, this doesn't work if I use -1,
                                                    //All data received and buffer is not all the way filled
            EUSCI_B2->CTLW0 |= BIT2; //Send Stop Bit
            ADCData[xferIndex++] = EUSCI_B2->RXBUF; //Buffer filled at some point in the middle

            bytesToRecCounter++;
            bufferFull = 1;
            EUSCI_B2->IE &= ~BIT0;

        }
        else if(xferIndex == DATA_BUF_SIZE - 1){ //Buffer is full

            //DONT SEND STOP BIT HERE, FOR SOME REASON EVERYTHING BREAKS
            ADCData[xferIndex++] = EUSCI_B2->RXBUF; //Last empty index on buffer
            bytesToRecCounter++;
            bufferFull = 1;
            EUSCI_B2->IE &= ~BIT0;

        }
        else
        {
                ADCData[xferIndex++] = EUSCI_B2->RXBUF;
                bytesToRecCounter++;
        }
    }
}


//GPS Timer Handler
void TA1_0_IRQHandler(void)
{
    seconds++;                                                          //Increases seconds by one
    UARTprintf(EUSCI_A0_BASE, "Time until we check GPS: %i\r\n",
           TIME_WAIT - seconds);                                        //Displays how much time is left before checking
    if (seconds >= TIME_WAIT){                                               //Checks if we reached or gone past TIME_WAIT
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

/*
 *This function is called to interpret the instruction received from
 *The PC Application and perform any corresponding actions such as
 *configuring the DAQ Modules or activating the solenoid
 */
void decodeInstruction(){
    uint8_t inst = rxdata[0];
    if(inst == (uint8_t) 128){ //Set configuration x80
        transmitByteData((uint8_t) 128); //Acknowledge
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

        transmitByteData((uint8_t) 129);
        int temp = conf_place;
        int i = 0;

        while(conf_place > 1){//this is a place holder, but eventually it should transmit
            transmitNByteData(configuration[i]);
            i++;
            conf_place--;
        }

        transmitByteData(configuration[i]);
        conf_place = temp;
        i = 0;

    } else if(inst == (uint8_t) 130){ //request start x82

        transmitByteData((uint8_t) 130);
        recorded = 0;
        stored = 0;

        char instruction[20];

        sprintf(instruction, "sample %d", sample_rate);
        processCommand(instruction);    //Set ADC sampling frequency
        memset(instruction, 0x00, sizeof(instruction));

        processCommand("start");

    } else if(inst == (uint8_t) 131){//request control module status
        transmitByteData((uint8_t) 131); //acknowledge
        transmitNByteData(recorded);
        transmitNByteData(stored);
        transmitByteData(gps_synched);

    } else if(inst == (uint8_t) 132){ //request number of modules connected x84
        transmitByteData((uint8_t) 132); //send success byte
        uint8_t i;
        for(i = 0; i < 7; i++){
            transmitNByteData((uint8_t) modules_connected_code[i]);
        }
        transmitByteData((uint8_t) modules_connected_code[7]);

    }else if(inst == (uint8_t) 133){ // Set configuration
        transmitByteData((uint8_t) 133);

        transmitByteData((uint8_t) 133);

        sample_rate = (uint8_t) rxdata[1];
        cutoff = (uint8_t) rxdata[2];
        gain = (uint8_t) rxdata[3];

        duration = ((uint8_t)rxdata[4])*1000 + ((uint8_t)rxdata[5])*100 + ((uint8_t)rxdata[6])*10 + ((uint8_t)rxdata[7]);

        start_delay = ((uint8_t)rxdata[8])*1000 + ((uint8_t)rxdata[9])*100 + ((uint8_t)rxdata[10])*10 + ((uint8_t)rxdata[11]);
        bytesToRec = duration * samplingRates[sample_rate] * 12;

        store_enable = (uint8_t)rxdata[12];
        vis_sens1 = (uint8_t)rxdata[13];
        vis_mod2 = (uint8_t)rxdata[14];
        vis_sens2 = (uint8_t)rxdata[15];

        int count = 16;
        uint8_t i;
        for(i = 0; i<32; i++){
            sensors_enabled[i] = rxdata[count];
            count++;
        }

        int hold_initial = 49;
        char a = rxdata[hold_initial];
        count = 0;


        while(a != ',' && count < 20){
            experiment_name[count] = a;
            a = rxdata[count + hold_initial + 1];
            count++;
        }

        //treating edge case of being sent blanks
        if(count == 0) count ++;

        int hold_place = count + hold_initial + 1;
        count = 0;

        a = rxdata[hold_place];

        while(a != ',' && count < 20 ){
            localization_name[count] = a;
            a = rxdata[hold_place + count + 1];
            count++;
        }
        count = 0;

        char instruction[20];

        //TODO This always create a blank file before each test,
        //Figure out a way to delimit different tests on a single file
        //to avoid having to erase the file each time
        src = fopen(daq1, "w+");
        fclose(src);

        //TODO
        //Right now, this configuration only works for a single DAQ Module,
        //or for multiple DAQs but all of them are configured identically
        //This means the enabled sensors will be the same for all modules

        //Send mux channels as all 0 (Sensors enabled, voltage reference off)

        if(diagnosis){ //Diagnostic option was selected on Application

            diagnosis = 0;
            uint8_t muxChannels, channelParam;
            muxChannels = sensors_enabled[0] * 1 + sensors_enabled[1] * 2 +
                    sensors_enabled[2] * 4 + sensors_enabled[3] * 8;
            channelParam = (0xF0 | muxChannels);

            sprintf(instruction, "diagnose %d", channelParam);
            processCommand(instruction);
            memset(instruction, 0x00, sizeof(instruction));

        }else{ //Normal Acquisition

            uint8_t adcChannels, channelParam;
            adcChannels = sensors_enabled[0] * 1 + sensors_enabled[1] * 2 +
                    sensors_enabled[2] * 4 + sensors_enabled[3] * 8;

            channelParam = (0xF0 & (adcChannels << 4));

            sprintf(instruction, "diagnose %d", channelParam);
            processCommand(instruction);
            memset(instruction, 0x00, sizeof(instruction));
        }


        sprintf(instruction, "duration %d", duration);
        processCommand(instruction); //Set experiment duration
        memset(instruction, 0x00, sizeof(instruction));

        sprintf(instruction, "gain %d", gain);

        processCommand(instruction); //Set Gain
        memset(instruction, 0x00, sizeof(instruction));

        sprintf(instruction, "cutoff %d", cutoff);
        processCommand(instruction);    // Set cutoff Frequency
        memset(instruction, 0x00, sizeof(instruction));


}
    else if(inst == (uint8_t) 135){ //Request data for one module

        transmitByteData((uint8_t) 135); //Acknowledge
        rcount = 0;
        rxdata[0] = 0;

        if(rxdata[1] == 0x01){
            processCommand("read 1");
        }else if(rxdata[1] == 0x02){
            processCommand("read 2");
        }else if(rxdata[1] == 0x03){
            processCommand("read 3");
        }else if(rxdata[1] == 0x04){
            processCommand("read 4");
        }else if(rxdata[1] == 0x05){
            processCommand("read 5");
        }else if(rxdata[1] == 0x06){
            processCommand("read 6");
        }else if(rxdata[1] == 0x07){
            processCommand("read 7");
        }else if(rxdata[1] == 0x08){
            processCommand("read 8");
        }

    }
    else if(inst == (uint8_t) 136){ //set visualization module + channels x88
        //TODO NOT YET IMPLEMENTED
        transmitByteData((uint8_t) 136); //acknowledge

    } else if(inst == (uint8_t) 141){ //send the live bytes to the application x87
        //TODO NOT YET IMPLEMENTED
        transmitByteData((uint8_t) 141); //acknowledge

    } else if(inst == (uint8_t) 137){ //send GPS data request 0x89
        transmitByteData((uint8_t) 137); //acknowledge
        sendUARTString(FTDI_UART_BASE, gps.transmission);

    } else if(inst == (uint8_t) 138){//sync the RTC with the GPS time 0x8A

        gps_synched = 0;
        gps.valid_Data = 0;
        check_format = 0;
        record = 0;
        counter = 0;
        comma_counter = 0;
        MAP_Interrupt_enableInterrupt(INT_TA1_0);
        Timer_A_startCounter(GPS_TIMER_BASE, TIMER_A_UP_MODE);
        transmitByteData((uint8_t) 138); //acknowledge

    } else if(inst == (uint8_t) 139){
        transmitByteData((uint8_t) 139); //acknowledge
        diagnosis = 1;
        transmitByteData(diagnosis);

    } else if(inst == (uint8_t) 140){ //request if configuration is valid
        transmitByteData((uint8_t) 140);
        //acknowledge
        transmitByteData(valid_conf);
    } else if(inst == (uint8_t) 255){ //request cancel xff

        transmitByteData((uint8_t) 255);
        //Send Reset signal to DAQs
        MAP_Interrupt_disableInterrupt(INT_EUSCIA2);
        MAP_Interrupt_disableInterrupt(INT_TA1_0);
        processCommand("reset");

    } else { //send a message stating that the function was a mistake

        transmitByteData((uint8_t) 254);
    }

    rcount = 0;
    rxdata[0] = 0;
}
