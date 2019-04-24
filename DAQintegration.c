#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
//#include "msp.h"
#include <stdint.h>

/* Statics */
uint8_t TXData = 0;
uint8_t pwm_divider = 0x00;
uint8_t data_in[4][3];
short ignore_sample = 0;
short ignore_sample_counter = 0;
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

void freq_Config(int freq, Timer_A_PWMConfig *pwmConfig)
{
    ignore_sample = 0;
    ignore_sample_counter = 0;
    *pwmConfig = (Timer_A_PWMConfig) {TIMER_A_CLOCKSOURCE_SMCLK, pwm_divider, 1, TIMER_A_CAPTURECOMPARE_REGISTER_1, TIMER_A_OUTPUTMODE_RESET_SET, 1};
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

}

void setDRDYPin(void)
{
    P5->IES |= BIT1;                            // Enable Interrupt on Falling Edge of DRDY
    P5->IE |= BIT1;                             // Port Interrupt Enable
    P5->IFG = 0;                                // Clear Port Interrupt Flag
    P5->DIR &= ~BIT1;                           // Set Pin as Input
    NVIC->ISER[1] |= 1 << ((PORT5_IRQn) & 31);  // Enable PORT5 interrupt in NVIC module
}

void timersetup()
{
//    //Timer_A0
//    TIMER_A0->CTL |= (uint16_t) 0x0200;             //CTL[9:8]=TASSEL=01b - TA takes input from SCLK
//    TIMER_A0->CTL |= (uint16_t) 0x0010;             //CTL[5:4]=MC=01b - timer in count up mode (resets when it reaches CCR0 value)
//    TIMER_A0->CCTL[1] &= (uint16_t) 0xFEFF;         //CCTL1[8]=CAP=0b set CCTL1 to compare mode
//    TIMER_A0->CCTL[1] |= (uint16_t) 0x00E0;         //CCTL1[7:5]=OUTMOD=111b OUT1 is reset when counter reaches CCR1, it is set after reaching CCR0;
//
//    //TA0 OUT1 exits through P5.6 (device specified)
//    //this puts CCR1's output on a pin so we can measure it easily
//    P2DIR |= BIT4;                                  //P2.4 set to output
//    P2SEL1 &= ~BIT4;                                //P2.4 SEL = 01B
//    P2SEL0 |= BIT4;

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

//Multiplexers
void MuxSelectors(char channels)
{
    //least significant 4 bits are used as the selectors for channels 4, 3, 2, and 1 respectively
    P8OUT = (channels & 0x0F) << 2;
}

//PGA
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

    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4,
                GPIO_PRIMARY_MODULE_FUNCTION);
    setDRDYPin();
}



void main(void)
{
//    clockSystem(1);
//    GPIOinit();
//   MuxSelectors(0x00);
//   GainSelect(1);
//    timersetup();
//    FilterFreq(8);

    /* Configures Clock System, timer PWM divider, and SPI
     * divider according to the sampling rate specified by
     * the user. */
//    freq_Config(20000);
    /* Configures PWM to pin 2.4 */
    Timer_A_PWMConfig pwmConfig;// =
//    {
//        TIMER_A_CLOCKSOURCE_SMCLK,
//        pwm_divider,
//        1,
//        TIMER_A_CAPTURECOMPARE_REGISTER_1,
//        TIMER_A_OUTPUTMODE_RESET_SET,
//        1
//    };

    freq_Config(256, &pwmConfig);
//    TXData = 0x00;                                                      // Transmission Dummy Data to generate bit clock
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
    //setDRDYPin();
}

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
            data_in[0][0] = EUSCI_B0->RXBUF;            // Store first byte of channel 1 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            data_in[0][1] = EUSCI_B0->RXBUF;            // Store second byte of channel 1 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            data_in[0][2] = EUSCI_B0->RXBUF;            // Store third byte of channel 1 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            data_in[1][0] = EUSCI_B0->RXBUF;            // Store first byte of channel 2 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            data_in[1][1] = EUSCI_B0->RXBUF;            // Store second byte of channel 2 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            data_in[1][2] = EUSCI_B0->RXBUF;            // Store third byte of channel 2 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            data_in[2][0] = EUSCI_B0->RXBUF;            // Store first byte of channel 3 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            data_in[2][1] = EUSCI_B0->RXBUF;            // Store second byte of channel 3 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            data_in[2][2] = EUSCI_B0->RXBUF;            // Store third byte of channel 3 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            data_in[3][0] = EUSCI_B0->RXBUF;            // Store first byte of channel 4 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            data_in[3][1] = EUSCI_B0->RXBUF;            // Store second byte of channel 4 in the array
            EUSCI_B0->TXBUF = TXData;                   // Transmit dummy data to generate bit clock
            while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));// Wait until we receive entire byte to read
            data_in[3][2] = EUSCI_B0->RXBUF;            // Store third byte of channel 4 in the array
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
