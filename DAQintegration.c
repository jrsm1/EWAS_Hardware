#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
//#include "msp.h"
#include <stdint.h>

//make reading boolean checks easier
//typedef enum boolean {false, true} bool;

/* Globals */
//adc
uint8_t TXData = 0;
uint8_t pwm_divider = 0x00;
short ignore_sample = 0;
short ignore_sample_counter = 0;
Timer_A_PWMConfig pwmConfig;

//state values
bool initialized = false;
bool datarequested = false;
bool samplingStandby = false;

//test variables
int testarray[600];
short testcount = 0;

//variables for dealing with instructions from master
char instructions[20];
char count = 0;

int datastored = 0;

/***************************************************
 *                    GENERAL                      *
 **************************************************/

void clockSystem(short mode)
{
    //mode = 0 sets system to 16MHz, mode = 1 sets to 24MHz
    if (mode == 1)
    {
        PCM->CTL0 = (uint32_t) 0x695A0001;      //active mode request VCORE1
        while (PCM->CTL1 & BIT8);
        CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
        CS->CTL0 = (uint32_t) 0x08040000;       // Reset tuning parameters
        CS->CTL1 = (uint32_t) 0x00000033;
        CS->KEY = 0;                  // Lock CS module from unintended accesses
    }
    else{
        PCM_setPowerState(PCM_AM_LF_VCORE0);
        CS_setDCOFrequency(16777216);                           // Lock CS module from unintended accesses
    }
}

//Multiplexers
void MuxSelectors(char channels)
{
    //least significant 4 bits are used as the selectors for channels 4, 3, 2, and 1 respectively
    P8OUT = (channels & 0x0F) << 2;
}

void GPIOinit()
{
    //multiplexer selectors
    P8DIR = (uint8_t) 0x3C;

    //pga
    P2DIR |= (uint8_t) 0x0B;
    SPIport();

    //TIMER_A2 OUT1
    P5DIR |= BIT6;
    P5SEL1 &= ~BIT6;
    P5SEL0 |= BIT6;

    //TIMER_A1 OUT1
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4,
                GPIO_PRIMARY_MODULE_FUNCTION);

    //DRDY
    P5->IES |= BIT1;                            // Enable Interrupt on Falling Edge of DRDY
    P5->IE |= BIT1;                             // Port Interrupt Enable
    P5->IFG = 0;                                // Clear Port Interrupt Flag
    P5->DIR &= ~BIT1;                           // Set Pin as Input
    NVIC->ISER[1] |= 1 << ((PORT5_IRQn) & 31);  // Enable PORT5 interrupt in NVIC module

    //I2C initialization or Sampling start signal
    P5DIR &= ~BIT0;
    P5IE |= BIT0;
    P5IFG = 0;

    //ADC powerdown selectors
    P5DIR |= BIT4 | BIT5 | BIT6 | BIT7;

    //ADC sync signal
    P3DIR |= BIT2;
    P3OUT |= BIT2;
}

//last 4 bits of input are the power selectors for channels 4, 3, 2, and 1
void ADCchannelselect(char channels){
    P5OUT &= ~BIT2 & ~BIT4 & ~BIT5 & ~BIT7;
    P5OUT |= (channels & 0x01) << 2;
    P5OUT |= (channels & 0x06) << 3;
    P5OUT |= (channels & 0x08) << 4;
}

/***************************************************
 *                       ADC                       *
 **************************************************/
/*
 * Configure ADC SPI with the given divider value
 */
void ADC_SPI_Config(uint16_t divider)
{
    //SPI Initialization/Reconfiguration
    P1->SEL0 |= BIT5 | BIT6 | BIT7;                 // Set P1.5, P1.6, and P1.7 as
                                                    // SPI pins functionality
    EUSCI_B0->CTLW0 = EUSCI_B_CTLW0_SWRST;          // Put eUSCI state machine in reset
    EUSCI_B0->CTLW0 = EUSCI_B_CTLW0_SWRST |         // Remain eUSCI state machine in reset
            EUSCI_B_CTLW0_MST |                     // Set as SPI master
            EUSCI_B_CTLW0_SYNC |                    // Set as synchronous mode
            EUSCI_B_CTLW0_MSB;                      // MSB first
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SSEL__SMCLK;   // SMCLK
    EUSCI_B0->BRW = divider;                        // fBitClock = fSPICLK/BRW
    EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;        // Initialize USCI state machine
}

void freq_Config(int freq/*, Timer_A_PWMConfig *pwmConfig*/)
{
    ignore_sample = 0;
    ignore_sample_counter = 0;
    switch(freq)
    {
    case 7:                                           //sampling rate of 256Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_64;   // 8,388,608 / 64 = 131,072Hz
        ADC_SPI_Config(0x80);                               // 16,777,216/ 128= 131,072Hz
        break;
    case 8:                                           //sampling rate of 512Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_32;   // 8,388,608 / 16 = 262,144Hz
        ADC_SPI_Config(0x40);                               // 16,777,216/ 64 = 262,144Hz
        break;
    case 9:                                          //sampling rate of 1,024Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_16;   // 8,388,608 / 16 = 524,288Hz
        ADC_SPI_Config(0x20);                               // 16,777,216/ 32 = 524,288Hz
        break;
    case 10:                                          //sampling rate of 2,048Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_8;    // 8,388,608 / 8 = 1,048,576Hz
        ADC_SPI_Config(0x10);                               // 16,777,216/ 16= 1,048,576Hz
        break;
    case 11:                                          //sampling rate of 4,096Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_4;    // 8,388,608 / 4 = 2,097,152Hz
        ADC_SPI_Config(0x08);                               // 16,777,216/ 8 = 2,097,152Hz
        break;
    case 12:                                          //sampling rate of 8,192Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_2;    // 8,388,608 / 2 = 4,194,304Hz
        ADC_SPI_Config(0x04);                               // 16,777,216/ 128=4,194,304Hz
        break;
    case 13:                                         //sampling rate of 16,384Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_1;    // 8,388,608 / 1 = 8,388,608Hz
        ADC_SPI_Config(0x02);                               // 16,777,216/ 2 = 8,388,608Hz
        break;
    case 14:                                         //sampling rate of 20,000Hz
        clockSystem(1);                                //Set DCO frequency to 24MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_1;    // 12MHz / 1 = 12MHz
        ADC_SPI_Config(0x02);                               // 24MHz / 2 = 12MHz
        break;
    default:                                            // sampling rate of 2-4-8-16-32-64-128Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_64;   // 8,388,608 / 64 = 131,072Hz
        ADC_SPI_Config(0x80);                               // 16,777,216/ 128= 131,072Hz
        switch(freq)
        {
        case 0:
            ignore_sample = 127;
            break;
        case 1:
            ignore_sample = 63;
            break;
        case 2:
            ignore_sample = 31;
            break;
        case 3:
            ignore_sample = 15;
            break;
        case 4:
            ignore_sample = 7;
            break;
        case 5:
            ignore_sample = 3;
            break;
        case 6:
            ignore_sample = 1;
            break;
        default:
            ignore_sample = 0;
        }
    }
    pwmConfig = (Timer_A_PWMConfig) {TIMER_A_CLOCKSOURCE_SMCLK, pwm_divider, 1, TIMER_A_CAPTURECOMPARE_REGISTER_1, TIMER_A_OUTPUTMODE_RESET_SET, 1};
}

/***************************************************
 *                     FILTER                      *
 **************************************************/

void filtertimersetup()
{
    //Timer_A2
    TIMER_A2->CTL |= (uint16_t) 0x0200; //CTL[9:8]=TASSEL=10b - TA takes input from SCLK
    TIMER_A2->CTL |= (uint16_t) 0x0010; //CTL[5:4]=MC=01b - timer in count up mode (resets when it reaches CCR0 value)
    TIMER_A2->CTL |= (uint16_t) 0x0080; //CTL[7:6]=ID=10b - divide input by 4
    TIMER_A2->CCTL[1] &= (uint16_t) 0xFEFF; //CCTL1[8]=CAP=0b set CCTL1 to compare mode
    TIMER_A2->CCTL[1] |= (uint16_t) 0x00E0; //CCTL1[7:5]=OUTMOD=111b OUT1 is reset when counter reaches CCR1, it is set after reaching CCR0;
}

void FilterFreq(short mode)
{
    //Using 16MHz system clocks for all but the highest frequency, which uses 12MHz
//    if (mode == 8)
//        clockSystem(1);
//    else
//        clockSystem(0);
    //format: timer output - cutoff frequency
    switch (mode)
        {
        case 0:     //100Hz - 1Hz
            TIMER_A2->CCR[0] = (uint16_t) 41942;
            TIMER_A2->CCR[1] = (uint16_t) 20971;
            break;
        case 1:     //200Hz - 2Hz
            TIMER_A2->CCR[0] = (uint16_t) 20970;
            TIMER_A2->CCR[1] = (uint16_t) 10485;
            break;
        case 2:     //400Hz - 4Hz
            TIMER_A2->CCR[0] = (uint16_t) 10484;
            TIMER_A2->CCR[1] = (uint16_t) 5242;
            break;
        case 3:     //800Hz - 8Hz
            TIMER_A2->CCR[0] = (uint16_t) 5241;
            TIMER_A2->CCR[1] = (uint16_t) 2621;
            break;
        case 4:     //1.6kHz - 16Hz
            TIMER_A2->CCR[0] = (uint16_t) 2620;
            TIMER_A2->CCR[1] = (uint16_t) 1310;
            break;
        case 5:     //3.2kHz - 32Hz
            TIMER_A2->CCR[0] = (uint16_t) 1309;
            TIMER_A2->CCR[1] = (uint16_t) 655;
            break;
        case 6:     //6.4kHz - 64Hz
            TIMER_A2->CCR[0] = (uint16_t) 654;
            TIMER_A2->CCR[1] = (uint16_t) 327;
            break;
        case 7:     //12.8kHz - 128Hz
            TIMER_A2->CCR[0] = (uint16_t) 326;
            TIMER_A2->CCR[1] = (uint16_t) 163;
            break;
        case 8:     //25.6kHz - 256Hz
            TIMER_A2->CCR[0] = (uint16_t) 162;
            TIMER_A2->CCR[1] = (uint16_t) 81;
            break;
        case 9:     //51.2kHz - 512Hz
            TIMER_A2->CCR[0] = (uint16_t) 80;
            TIMER_A2->CCR[1] = (uint16_t) 40;
            break;
        case 10:     //102.4kHz - 1024Hz
            TIMER_A2->CCR[0] = (uint16_t) 39;
            TIMER_A2->CCR[1] = (uint16_t) 20;
            break;
        case 11:     //204.8kHz - 2048Hz
            TIMER_A2->CCR[0] = (uint16_t) 19;
            TIMER_A2->CCR[1] = (uint16_t) 10;
            break;
        case 12:     //409.6Hz - 4096Hz
            TIMER_A2->CCR[0] = (uint16_t) 9;
            TIMER_A2->CCR[1] = (uint16_t) 5;
            break;
        case 13:     //819.2kHz - 8.19kHz
            TIMER_A2->CCR[0] = (uint16_t) 4;
            TIMER_A2->CCR[1] = (uint16_t) 2;
            break;
        case 14:     //1MHz - 10kHz
            TIMER_A2->CCR[0] = (uint16_t) 3;
            TIMER_A2->CCR[1] = (uint16_t) 2;
            break;
        default:
            TIMER_A2->CCR[0] = (uint16_t) 41942;
            TIMER_A2->CCR[1] = (uint16_t) 20971;
            break;
        }
}

/***************************************************
 *                       PGA                       *
 **************************************************/

void GainSelect(short gain)
{
    uint8_t option = 0x01;
    switch (gain)
    {
    case 0:     //x1
        option = 0x01;
        break;
    case 1:     //x10
        option = 0x03;
        break;
    case 2:     //x20
        option = 0x05;
        break;
    case 3:     //x30
        option = 0x07;
        break;
    case 4:     //x40
        option = 0x09;
        break;
    case 5:     //x60
        option = 0x0B;
        break;
    case 6:     //x80
        option = 0x0D;
        break;
    case 7:     //x120
        option = 0x0F;
        break;
    case 8:     //x157
        option = 0x11;
        break;
    case 9:     //x0.2
        option = 0x13;
        break;
    }
    EUSCI_A1->TXBUF = option;
}

void SPIport()
{
    P2->SEL0 |= BIT0 | BIT1 | BIT3;
    UCA1CTLW0 |= UCMST |    //master mode
            UCMODE_2 |  //4 pin slave enabled on low
            UCSYNC |    //synchronous mode
            UCSSEL_2 |  //set clock source as SMCLK
            UCSTEM |    //used to generate signal for a 4-wire salve
            UCCKPH |
            UCSWRST;    //software reset enable
    UCA1BRW |= 0x00;                   // /2,fBitClock = fBRCLK/(UCBRx+1).
    UCA1CTLW0 &= ~UCSWRST;             // **Initialize USCI state machine**
}

/***************************************************
 *                      SRAM                       *
 **************************************************/

//Set SRAM operation mode
//S for storing, R for reading
void SRAMdir(char mode, unsigned int address){
    //I/O: port 6       //single byte io mode
    if(mode == 'S'){
        P6OUT = 0x00;
        P6DIR = 0xFF;
    }
    else{
        P6OUT = 0x00;
        P6DIR = 0x00;
    }

    //address ports
    P10DIR = 0xFF; //MSB
    P9DIR = 0xFF;
    P7DIR = 0xFF;   //LSB

    P7OUT = (uint8_t) address & 0xFF;
    P9OUT = (uint8_t) (address >> 8) & 0xFF;
    P10OUT = (uint8_t) (address >> 16) & 0xFF;

    //chip select CE2 (CE1 goes to ground since we only have 1 IC)
    P3DIR |= 0x01;
    P3OUT &= 0x00;

    //write, output, byte high, and byte low enable signals
    P4DIR |= 0x0F;
    P4OUT = (mode == 'S') ? P4OUT = 0x06 : 0x0A;
}

void storeByte(uint8_t data){
    P6OUT = data;
    P3OUT |= BIT0;           //enable to write
    P3OUT &= ~BIT0;           //disable to prepare next byte
    if(++P7OUT == 0)        //increment address
        if(++P9OUT == 0)
            P10OUT++;
}

char readByte(){
    char ret;
    P3OUT |= BIT0;
    ret = P6IN;
    if(++P7OUT == 0)        //increment address
        if(++P9OUT == 0)
            P10OUT++;
    return ret;
}



/***************************************************
 *                   DIAGNOSTICS                   *
 **************************************************/

char sramtest(){

    //creating array of 200 bytes
    uint8_t storethis[200];
    uint8_t readback[200];
    uint8_t readback2[100];
    uint8_t readback3[280];
    char testpassed = 0x07;
    int i = 0;
    for(i=0;i<200;i++){
        storethis[i] = (uint8_t) i;
    }

    ////TEST1

    //storing in SRAM
    SRAMdir('S', 0);            //store mode, address 0
    for(i=0;i<200;i++){
        P6OUT = storethis[i];
        P3OUT |= BIT0;          //enable to write
        P3OUT &= ~BIT0;         //disable to prepare next byte
        if(++P7OUT == 0)        //increment address
            if(++P9OUT == 0)
                P10OUT++;
    }

    //Reading the data back from SRAM
    SRAMdir('R', 0);            //read mode, address 0
    P3OUT |= BIT0;               //enabling chip
    for(i=0;i<200;i++){
        readback[i] = (uint8_t) P6IN;
        if(++P7OUT == 0)
            if(++P9OUT == 0)
                P10OUT++;
    }
    P3OUT &= ~BIT0;   //disable chip after finished

    //TEST1 RESULTS
    testpassed |= BIT0;
    for(i=0;i<200;i++)
        if(readback[i] != i){
            testpassed &= ~BIT0;
            break;
        }

    //TEST2
    SRAMdir('S', 131015);   //131015 = 0x1FFC7
    for(i=0;i<100;i++){
        P6OUT = (uint8_t) 0xAA;
        P3OUT |= BIT0;       //enable to write
        P3OUT &= ~BIT0;       //disable to prepare next byte
        if(++P7OUT == 0)    //increment address
            if(++P9OUT == 0)
                P10OUT++;
    }

    SRAMdir('R', 131015);        //read mode, address 0
    P3OUT |= BIT0;   //enabling chip
    for(i=0;i<100;i++){
        readback2[i] = (uint8_t) P6IN;
        if(++P7OUT == 0)
            if(++P9OUT == 0)
                P10OUT++;
    }
    P3OUT &= ~BIT0;   //disable chip after finished

    //TEST2 RESULTS
    //Test if data was stored correctly
    testpassed |= BIT1;
    for(i=0;i<100;i++)
        if(readback2[i] != 0xAA){
            testpassed &= ~BIT1;
            break;
        }
    //Test if address was incremented appropriately
    if(P9OUT > 0)
        testpassed &= ~BIT1;
    if(P10OUT < 2)
        testpassed &= ~BIT1;

    //TEST3
    SRAMdir('S', 180);
    for(i=0;i<100;i++){
        P6OUT = (uint8_t) 0xFF;
        P3OUT |= BIT0;       //enable to write
        P3OUT &= ~BIT0;       //disable to prepare next byte
        if(++P7OUT == 0)    //increment address
            if(++P9OUT == 0)
                P10OUT++;
    }

    SRAMdir('R', 0);        //read mode, address 0
    P3OUT |= BIT0;           //enabling chip
    for(i=0;i<280;i++){
        readback3[i] = (uint8_t) P6IN;
        if(++P7OUT == 0)
            if(++P9OUT == 0)
                P10OUT++;
    }
    P3OUT &= ~BIT0;   //disable chip after finished

    //Test if data from previous tests has remained
    testpassed |= BIT2;
    for(i=0;i<180;i++)
        if(readback3[i] != i){
            testpassed &= ~BIT2;
            break;
        }
    for(i=180;i<280;i++)
        if(readback3[i] != 0xFF){
            testpassed &= ~BIT2;
            break;
        }

   return testpassed;
}

/***************************************************
 *            CONTROL UNIT <-> DAQ                 *
 **************************************************/

//i2c setup instructions
void i2cinit(){
    EUSCI_B2->CTLW0 = (uint16_t) 0x0001;                //CTLW0[0] = UCSWRST = 1b       /set so we can modify the rest of the register
    EUSCI_B2->CTLW0 &= (uint16_t) 0x7FFF;               //CTLW0[15] = UCA10 = 0b        /set own address to 7bit
    EUSCI_B2->CTLW0 &= (uint16_t) 0xF7FF;               //CTLW0[11] = UCMST = 0b        /slave
    EUSCI_B2->CTLW0 |= (uint16_t) 0x0600;               //CLTW0[10:9] = UCMODE = 11b    /I2C mode
//    EUSCI_B2->BRW |= (uint16_t) 0xFF;                    //(int) clk/baud rate

    //EUSCI_B2 SDA
    P3SEL1 &= ~BIT6;
    P3SEL0 |= BIT6;
    //EUSCI_B2 SCL
    P3SEL1 &= ~BIT7;
    P3SEL0 |= BIT7;

    EUSCI_B2->I2COA0 |= 0x0460;                         //address enabled + slave module address = 110 0000

    EUSCI_B2->CTLW0 &= (uint16_t) 0xFFFE;               //UCSWRST = 0

    EUSCI_B2->IE = 0x000B;                                //interrupts enabled
    NVIC->ISER[0] = 1 << (EUSCIB2_IRQn);

    //master's I2C address
    EUSCI_B2->I2CSA = (uint16_t) 0x58;
}

void interpretInstruction(char* instruction){
    short i; //for loop counter
    if(instruction[0] == 0xCA){         //check header
        for(i = 1; i<count; i++){
            switch(instruction[i]){
            case 0x61:                                      //sampling frequency
                SRAMdir('S', 0);
                freq_Config(instruction[++i]);
                Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
//                samplingStandby = true;
                P3OUT |= BIT2;
                break;
            case 0x62:                                      //gain select
                GainSelect(instruction[++i]);
                break;
            case 0x63:                                      //selecting mux and adc channels
                MuxSelectors(instruction[++i]);
                ADCchannelselect(instruction[i]);
                break;
            case 0x64:                                      //cutoff frequency selection
                FilterFreq(instruction[++i]);
                break;
            case 0x65:
                datarequested = true;
                datastored = (P10OUT & 0x1F)<<16 | P9OUT<< 8 | P7OUT;
                break;
//            case 0x65:                                      //power down
//                toggleChannels(down, instruction[++i]);
//                break;
//            case 0x66:                                      //power up
//                toggleChannels(up, instruction[++i]);
//                break;
//            case 0x68:                                      //set output format
//                i++;
//                break;
//            case 0x9A:                                      //begin sampling
//                startSampling();
//                break;
//            case 0x9B:
//                SRAMtesting();                              //test ram write;
//                break;
//            case 0x6C:                                      //set clock frequency
//                i++;
//                break;
//            case 0x6E:                                      //set SClock ratio
//                i++;
//                break;
//            case 0x6F:                                      //data request
//                request = true;
//                i++;
//                break;
            default:
                i++;
            }
        }
    }
}


/***************************************************
 *                      MAIN                       *
 **************************************************/

void main(void)
{
    clockSystem(0);
    GPIOinit();
//    EUSCI_B2->I2COA0 = (uint16_t) 0x0451;
//    initialized = true;
//    i2cinit();
    filtertimersetup();
//    count = 9;
//    char setup[] = {0xCA, 0x63, 0xEE, 0x62, 0x01, 0x64, 0x08, 0x61, 0x08};
//    interpretInstruction(setup);

    for(;;);
}

/***************************************************
 *              INTERRUPT HANDLERS                 *
 **************************************************/

char rec1;
char rec2;
char rec3;
char data;
//Read sample from ADC
int testarray2[200];
void PORT5_IRQHandler(void)
{
    if (P5IFG & BIT1)
    {
        if(!ignore_sample_counter)
        {
            if(ignore_sample)
                ignore_sample_counter++;
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
//            storeByte(EUSCI_B0->RXBUF);                 // Store first byte of channel 1 in the array
//            testarray[testcount++] = EUSCI_B0->RXBUF;
            rec1 = EUSCI_B0->RXBUF;
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
//            storeByte(EUSCI_B0->RXBUF);                 // Store second byte of channel 1 in the array
//            testarray[testcount++] = EUSCI_B0->RXBUF;
            rec2 = EUSCI_B0->RXBUF;
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
//            storeByte(EUSCI_B0->RXBUF);                 // Store third byte of channel 1 in the array
//            testarray[testcount++] = EUSCI_B0->RXBUF;
            rec3 = EUSCI_B0->RXBUF;
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            //storebyte(eusci_B0->RXBUF);                 // Store first byte of channel 2 in the array
            //testarray[testcount++] = EUSCI_B0->RXBUF;
//            data = EUSCI_B0->RXBUF;
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            //storebyte(eusci_B0->RXBUF);                 // Store second byte of channel 2 in the array
            //testarray[testcount++] = EUSCI_B0->RXBUF;
//            data = EUSCI_B0->RXBUF;
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            //storebyte(eusci_B0->RXBUF);                 // Store third byte of channel 2 in the array
            //testarray[testcount++] = EUSCI_B0->RXBUF;
            data = EUSCI_B0->RXBUF;
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            //storebyte(eusci_B0->RXBUF);                 // Store first byte of channel 3 in the array
            //testarray[testcount++] = EUSCI_B0->RXBUF;
//            data = EUSCI_B0->RXBUF;
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            //storebyte(eusci_B0->RXBUF);                 // Store second byte of channel 3 in the array
            //testarray[testcount++] = EUSCI_B0->RXBUF;
//            data = EUSCI_B0->RXBUF;
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            //storebyte(eusci_B0->RXBUF);                 // Store third byte of channel 3 in the array
            //testarray[testcount++] = EUSCI_B0->RXBUF;
//            data = EUSCI_B0->RXBUF;
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            //storebyte(eusci_B0->RXBUF);                 // Store first byte of channel 4 in the array
            //testarray[testcount++] = EUSCI_B0->RXBUF;
//            data = EUSCI_B0->RXBUF;
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            //storebyte(eusci_B0->RXBUF);                 // Store second byte of channel 4 in the array
            //testarray[testcount++] = EUSCI_B0->RXBUF;
//            data = EUSCI_B0->RXBUF;
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            //storebyte(eusci_B0->RXBUF);                 // Store third byte of channel 4 in the array
            //testarray[testcount++] = EUSCI_B0->RXBUF;
//            data = EUSCI_B0->RXBUF;
        }
        else{
            if(ignore_sample_counter == ignore_sample)
                ignore_sample_counter = 0;
            else
                ignore_sample_counter++;
        }

        storeByte(rec1);
        storeByte(rec2);
        storeByte(rec3);
        testcount++;

        if(testcount == 200){
            P3OUT &= ~BIT2;
            SRAMdir('R', 0);
            P4DIR |= BIT4; //signal master data has been collected
            P4OUT |= BIT4;
//            P3OUT = 0x01;           //enabling chip
//            int i;
//            for(i=0;i<testcount;i++){
//                if(i%3==0) rec1 = P6IN;
//                else if(i%3 == 1) rec2 = P6IN;
//                else rec3 = P6IN;
//                testarray2[i] = (int) rec1<<16 | rec2<<8 | rec3;
//                if(++P7OUT == 0)
//                    if(++P9OUT == 0)
//                        P10OUT++;
//            }
//            testcount = 0;
        }

    }
    else if(P5IFG & BIT0){
        if(!initialized)            //initialize i2c module when receiving interrupt from master
            i2cinit();
        else if(samplingStandby)    //kick off adc operation
            P3OUT |= BIT2;
    }
    P5->IFG = 0;                                // Clear Port Interrupt Flag
}

int sentdata = 0;
void EUSCIB2_IRQHandler(void){

    if (EUSCI_B2->CTLW0 & BIT4 /*&& datarequested*/){
            EUSCI_B2->TXBUF = readByte();
            EUSCI_B2->IFG = 0;
    //        if(sentdata == 600){
    //            sentdata = 0;
    //            EUSCI_B2->IFG &= ~BIT1;
    //        }
    }
    else if(EUSCI_B2->IFG & BIT0){
        char received;
        received = EUSCI_B2->RXBUF;
        EUSCI_B2->IFG &= ~BIT0;
        if(initialized == false && ((received & 0xF0) == 0x50)){                               //set new address if not initialized yet
            EUSCI_B2->I2COA0 = 0x8400 + received;
            initialized =  true;
    //            EUSCI_B2->IFG &= ~BIT3;         //turning this flag off to make sure it doesn't try to interpret instructions
        }
        else{                                           //storing instruction byte
            instructions[count++] = received;
        }
        EUSCI_B2->IFG &= ~BIT0;
    }
    else if(EUSCI_B2->IFG & BIT3 && initialized){           //after instructions have all been stored
        interpretInstruction(instructions);
        count = 0;
        EUSCI_B2->IFG &= ~BIT3;
    }

    //status is being requested
//    else if(EUSCI_B2->IFG & BIT1){
//        EUSCI_B2->TXBUF = message;
//        EUSCI_B2->IFG &= ~BIT1;
//    }
}
