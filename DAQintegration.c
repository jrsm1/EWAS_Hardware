#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
//#include "msp.h"
#include <stdint.h>

/* Globals */
uint8_t TXData = 0;
uint8_t pwm_divider = 0x00;
short ignore_sample = 0;
short ignore_sample_counter = 0;

/***************************************************
 *                   DIAGNOSTICS                   *
 **************************************************/

char sramtest(){

    //creating array of 200 bytes
    uint8_t storethis[200];
    uint8_t readback[200];
    uint8_t readback2[100];
    uint8_t readback3[280];
    char testpassed 0x00;
    volatile int i;
    for(i=0;i<200;i++){
        storethis[i] = i;
    }

    //storing in SRAM
    SRAMdir('S', 0);            //store mode, address 0
    for(i=0;i<200;i++){
        P2OUT = storethis[i];
        P3OUT = 0x01;           //enable to write
        P3OUT = 0x00;           //disable to prepare next byte
        if(++P7OUT == 0)        //increment address
            if(++P9OUT == 0)
                P10OUT++;
    }

    //Reading the data back from SRAM
    SRAMdir('R', 0);            //read mode, address 0
    P3OUT = 0x01;               //enabling chip
    for(i=0;i<200;i++){
        readback[i] = (uint8_t) P2IN;
        if(++P7OUT == 0)
            if(++P9OUT == 0)
                P10OUT++;
    }
    P3OUT = 0x00;   //disable chip after finished

    //Test if data was stored correctly
    testpassed |= BIT0;
    for(i=0;i<200;i++)
        if(readback[i] != i){
            testpassed &= ~BIT0;
            break;
        }

    SRAMdir('S', 131015);   //131015 = 0x1FFC7
    for(i=0;i<100;i++){
        P2OUT = (uint8_t) 0xAA;
        P3OUT = 0x01;       //enable to write
        P3OUT = 0x00;       //disable to prepare next byte
        if(++P7OUT == 0)    //increment address
            if(++P9OUT == 0)
                P10OUT++;
    }

    SRAMdir('R', 131015);        //read mode, address 0
    P3OUT = 0x01;   //enabling chip
    for(i=0;i<100;i++){
        readback2[i] = (uint8_t) P2IN;
        if(++P7OUT == 0)
            if(++P9OUT == 0)
                P10OUT++;
    }
    P3OUT = 0x00;   //disable chip after finished


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

    SRAMdir('S', 180);
    for(i=0;i<100;i++){
        P2OUT = (uint8_t) 0xFF;
        P3OUT = 0x01;       //enable to write
        P3OUT = 0x00;       //disable to prepare next byte
        if(++P7OUT == 0)    //increment address
            if(++P9OUT == 0)
                P10OUT++;
    }

    SRAMdir('R', 0);        //read mode, address 0
    P3OUT = 0x01;           //enabling chip
    for(i=0;i<280;i++){
        readback3[i] = (uint8_t) P2IN;
        if(++P7OUT == 0)
            if(++P9OUT == 0)
                P10OUT++;
    }
    P3OUT = 0x00;   //disable chip after finished

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

void freq_Config(int freq, Timer_A_PWMConfig *pwmConfig)
{
    ignore_sample = 0;
    ignore_sample_counter = 0;
    switch(freq)
    {
    case 256:                                           //sampling rate of 256Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_64;   // 8,388,608 / 64 = 131,072Hz
        ADC_SPI_Config(0x80);                               // 16,777,216/ 128= 131,072Hz
        break;
    case 512:                                           //sampling rate of 512Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_32;   // 8,388,608 / 16 = 262,144Hz
        ADC_SPI_Config(0x40);                               // 16,777,216/ 64 = 262,144Hz
        break;
    case 1024:                                          //sampling rate of 1,024Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_16;   // 8,388,608 / 16 = 524,288Hz
        ADC_SPI_Config(0x20);                               // 16,777,216/ 32 = 524,288Hz
        break;
    case 2048:                                          //sampling rate of 2,048Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_8;    // 8,388,608 / 8 = 1,048,576Hz
        ADC_SPI_Config(0x10);                               // 16,777,216/ 16= 1,048,576Hz
        break;
    case 4096:                                          //sampling rate of 4,096Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_4;    // 8,388,608 / 4 = 2,097,152Hz
        ADC_SPI_Config(0x08);                               // 16,777,216/ 8 = 2,097,152Hz
        break;
    case 8192:                                          //sampling rate of 8,192Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_2;    // 8,388,608 / 2 = 4,194,304Hz
        ADC_SPI_Config(0x04);                               // 16,777,216/ 128=4,194,304Hz
        break;
    case 16384:                                         //sampling rate of 16,384Hz
        clockSystem(0);                                //Set DCO frequency to 16MHz
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_1;    // 8,388,608 / 1 = 8,388,608Hz
        ADC_SPI_Config(0x02);                               // 16,777,216/ 2 = 8,388,608Hz
        break;
    case 20000:                                         //sampling rate of 20,000Hz
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
        case 2:
            ignore_sample = 127;
            break;
        case 4:
            ignore_sample = 63;
            break;
        case 8:
            ignore_sample = 31;
            break;
        case 16:
            ignore_sample = 15;
            break;
        case 32:
            ignore_sample = 7;
            break;
        case 64:
            ignore_sample = 3;
            break;
        case 128:
            ignore_sample = 1;
            break;
        default:
            ignore_sample = 0;
        }
    }
    *pwmConfig = (Timer_A_PWMConfig) {TIMER_A_CLOCKSOURCE_SMCLK, pwm_divider, 1, TIMER_A_CAPTURECOMPARE_REGISTER_1, TIMER_A_OUTPUTMODE_RESET_SET, 1};
}


//last 4 bits of input are the power selectors for channels 4, 3, 2, and 1
void channelselect(char channels){
    P5OUT &= ~BIT2 & ~BIT4 & ~BIT5 & ~BIT7;
    P5OUT |= (channels & 0x01) << 2;
    P5OUT |= (channels & 0x06) << 3;
    P5OUT |= (channels & 0x08) << 4;
}

/***************************************************
 *                     FILTER                      *
 **************************************************/

void filtertimersetup()
{
    //Timer_A2
    TIMER_A2->CTL |= (uint16_t) 0x0200; //CTL[9:8]=TASSEL=10b - TA takes input from SCLK
    TIMER_A2->CTL |= (uint16_t) 0x0010; //CTL[5:4]=MC=01b - timer in count up mode (resets when it reaches CCR0 value)
    TIMER_A2->CCTL[1] &= (uint16_t) 0xFEFF; //CCTL1[8]=CAP=0b set CCTL1 to compare mode
    TIMER_A2->CCTL[1] |= (uint16_t) 0x00E0; //CCTL1[7:5]=OUTMOD=111b OUT1 is reset when counter reaches CCR1, it is set after reaching CCR0;
}

void FilterFreq(short mode)
{
    //Using 16MHz system clocks for all but the highest frequency, which uses 12MHz
    if (mode == 8)
        clockSystem(1);
    else
        clockSystem(0);
    //format: timer output - cutoff frequency
    switch (mode)
    {
    case 0:     //100Hz - 1Hz
        TIMER_A2->CCR[0] = (uint16_t) 4095;
        TIMER_A2->CCR[1] = (uint16_t) 2048;
        break;
    case 1:     //5kHz - 50Hz
        TIMER_A2->CCR[0] = (uint16_t) 2047;
        TIMER_A2->CCR[1] = (uint16_t) 1024;
        break;
    case 2:     //10kHz - 100Hz
        TIMER_A2->CCR[0] = (uint16_t) 1023;
        TIMER_A2->CCR[1] = (uint16_t) 512;
        break;
    case 3:     //25kHz - 250Hz
        TIMER_A2->CCR[0] = (uint16_t) 511;
        TIMER_A2->CCR[1] = (uint16_t) 256;
        break;
    case 4:     //50kHz - 500Hz
        TIMER_A2->CCR[0] = (uint16_t) 255;
        TIMER_A2->CCR[1] = (uint16_t) 128;
        break;
    case 5:     //100kHz - 1kHz
        TIMER_A2->CCR[0] = (uint16_t) 127;
        TIMER_A2->CCR[1] = (uint16_t) 64;
        break;
    case 6:     //250kHz - 2.5kHz
        TIMER_A2->CCR[0] = (uint16_t) 63;
        TIMER_A2->CCR[1] = (uint16_t) 32;
        break;
    case 7:     //500kHz - 5kHz
        TIMER_A2->CCR[0] = (uint16_t) 31;
        TIMER_A2->CCR[1] = (uint16_t) 16;
        break;
    case 8:     //1MHz - 10kHz
        TIMER_A2->CCR[0] = (uint16_t) 11;
        TIMER_A2->CCR[1] = (uint16_t) 6;
        break;
    default:
        TIMER_A2->CCR[0] = (uint16_t) 511;
        TIMER_A2->CCR[1] = (uint16_t) 256;
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
void SRAMdir(char mode, int address){
    //I/O: port 2       //single byte io mode
    if(mode == 'S'){
        P2DIR = 0xFF;
        P2OUT = 0x00;
    }
    else{
        P2OUT = 0x00;
        P2DIR = 0x00;
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

void storeByte((uint8_t) data){
    P2OUT = data;
    P3OUT = 0x01;           //enable to write
    P3OUT = 0x00;           //disable to prepare next byte
    if(++P7OUT == 0)        //increment address
        if(++P9OUT == 0)
            P10OUT++;
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

    EUSCI_B2->IE = 0x000F;                                //interrupts enabled
    NVIC->ISER[0] = 1 << (EUSCIB2_IRQn);

    //master's I2C address
    EUSCI_B2->I2CSA = (uint16_t) 0x58;
}

//void interpretInstruction(char* instruction){
//    short i; //for loop counter
//    if(instruction[0] == 0xCA){         //check header
//        for(i = 1; i<count; i++){
//            switch(instruction[i]){
//            case 0x61:
//                setSamplingPeriod(instruction[++i]);        //sample period
//                break;
//            case 0x62:                                      //gain select
//                setGainFactor(instruction[++i]);
//                changePGAAMP(settings.gain);
//                break;
//            case 0x63:                                      //diagnostics
//                muxstate = (int) instruction[++i];
//                changeMUX(muxstate);
//                SRAMinit();
//                break;
//            case 0x64:                                      //sync
//                sync = true;
//                i++;
//                break;
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
//            default:
//                i++;
//            }
//        }
//    }
//}


/***************************************************
 *                    GENERAL                      *
 **************************************************/

void clockSystem(short mode)
{
    //mode = 0 sets system to 16MHz, mode = 1 sets to 24
    if (mode == 1)
    {
        PCM->CTL0 = (uint32_t) 0x695A0001;      //active mode request VCORE1
        while (PCM->CTL1 & BIT8 )
            ;
        CS->KEY = CS_KEY_VAL;            // Unlock CS module for register access
        CS->CTL0 = (uint32_t) 0x08040000;       // Reset tuning parameters
        CS->CTL1 = (uint32_t) 0x10100033;
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
    P2DIR = (uint8_t) 0x0B;
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

    //I2C initialization
    P8DIR &= ~BIT7;
    P8IE |= BIT7;
    P8IFG = 0;
    NVIC->ISER[1] |= 1 << ((PORT8_IRQn) & 31);

    //ADC powerdown selectors
    P5DIR |= BIT4 | BIT5 | BIT6 | BIT7;
}


/***************************************************
 *                      MAIN                       *
 **************************************************/

void main(void)
{
    GPIOinit();
    channelselect(0x01);
//    Timer_A_PWMConfig pwmConfig;
//    freq_Config(256, &pwmConfig);                                                 // Transmission Dummy Data to generate bit clock
//    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
}

/***************************************************
 *              INTERRUPT HANDLERS                 *
 **************************************************/


///////TODO figure out where to increment samplecount variable
//Read sample from ADC
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
            storeByte(EUSCI_B0->RXBUF);                 // Store first byte of channel 1 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            storeByte(EUSCI_B0->RXBUF);                 // Store second byte of channel 1 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            storeByte(EUSCI_B0->RXBUF);                 // Store third byte of channel 1 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            storeByte(EUSCI_B0->RXBUF);                 // Store first byte of channel 2 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            storeByte(EUSCI_B0->RXBUF);                 // Store second byte of channel 2 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            storeByte(EUSCI_B0->RXBUF);                 // Store third byte of channel 2 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            storeByte(EUSCI_B0->RXBUF);                 // Store first byte of channel 3 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            storeByte(EUSCI_B0->RXBUF);                 // Store second byte of channel 3 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            storeByte(EUSCI_B0->RXBUF);                 // Store third byte of channel 3 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            storeByte(EUSCI_B0->RXBUF);                 // Store first byte of channel 4 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            storeByte(EUSCI_B0->RXBUF);                 // Store second byte of channel 4 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            storeByte(EUSCI_B0->RXBUF);                 // Store third byte of channel 4 in the array
        }
        else{
            if(ignore_sample_counter == ignore_sample)
                ignore_sample_counter = 0;
            else
                ignore_sample_counter++;
        }
    }
    P5->IFG = 0;                                // Clear Port Interrupt Flag
}

//initializing i2c module
void PORT8_IRQHandler(void){
    i2cinit();
}
