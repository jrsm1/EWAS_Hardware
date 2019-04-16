/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* Statics */
uint8_t TXData = 0;
uint8_t pwm_divider = 0x00;
uint8_t data_in[4][3];
/*
 * Configure ADC SPI with the given divider value
 */
void SPI_Config(uint16_t divider)
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

/*
 * Set DCO frequency and dividers for timer and SPI
 * depending on the sampling rate
 */
void freq_Config(int freq)
{
    switch(freq)
    {
    case 256:                                           //sampling rate of 256Hz
        PCM_setPowerState(PCM_AM_LF_VCORE0);
        CS_setDCOFrequency(16777216);
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_64;   // 8,388,608 / 64 = 131,072Hz
        SPI_Config(0x80);                               // 16,777,216/ 128= 131,072Hz
        break;
    case 512:                                           //sampling rate of 512Hz
        PCM_setPowerState(PCM_AM_LF_VCORE0);
        CS_setDCOFrequency(16777216);
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_32;   // 8,388,608 / 16 = 262,144Hz
        SPI_Config(0x40);                               // 16,777,216/ 64 = 262,144Hz
        break;
    case 1024:                                          //sampling rate of 1,024Hz
        PCM_setPowerState(PCM_AM_LF_VCORE0);
        CS_setDCOFrequency(16777216);
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_16;   // 8,388,608 / 16 = 524,288Hz
        SPI_Config(0x20);                               // 16,777,216/ 32 = 524,288Hz
        break;
    case 2048:                                          //sampling rate of 2,048Hz
        PCM_setPowerState(PCM_AM_LF_VCORE0);
        CS_setDCOFrequency(16777216);
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_8;    // 8,388,608 / 8 = 1,048,576Hz
        SPI_Config(0x10);                               // 16,777,216/ 16= 1,048,576Hz
        break;
    case 4096:                                          //sampling rate of 4,096Hz
        PCM_setPowerState(PCM_AM_LF_VCORE0);
        CS_setDCOFrequency(16777216);
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_4;    // 8,388,608 / 4 = 2,097,152Hz
        SPI_Config(0x08);                               // 16,777,216/ 8 = 2,097,152Hz
        break;
    case 8192:                                          //sampling rate of 8,192Hz
        PCM_setPowerState(PCM_AM_LF_VCORE0);
        CS_setDCOFrequency(16777216);
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_2;    // 8,388,608 / 2 = 4,194,304Hz
        SPI_Config(0x04);                               // 16,777,216/ 128=4,194,304Hz
        break;
    case 16384:                                         //sampling rate of 16,384Hz
        PCM_setPowerState(PCM_AM_LF_VCORE0);
        CS_setDCOFrequency(16777216);
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_1;    // 8,388,608 / 1 = 8,388,608Hz
        SPI_Config(0x02);                               // 16,777,216/ 2 = 8,388,608Hz
    case 20000:                                         //sampling rate of 20,000Hz
        PCM_setPowerState(PCM_AM_LF_VCORE1);
        CS_setDCOFrequency(24000000);
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_1;    // 12MHz / 1 = 12MHz
        SPI_Config(0x02);                               // 24MHz / 2 = 12MHz
        break;
    default:                                            // sampling rate of 2-4-8-16-32-64-128Hz
        PCM_setPowerState(PCM_AM_LF_VCORE0);
        CS_setDCOFrequency(16777216);
        pwm_divider = TIMER_A_CLOCKSOURCE_DIVIDER_64;   // 8,388,608 / 64 = 131,072Hz
        SPI_Config(0x80);                               // 16,777,216/ 128= 131,072Hz
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

int main(void)
{
    /* Halting WDT  */
    WDT_A_holdTimer();
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);  //Set MCLK to DCO
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1); //Set SMCLK to DCO
    /* Configuring GPIO2.4 as peripheral output for PWM */
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4,
            GPIO_PRIMARY_MODULE_FUNCTION);
    /* Configures Clock System, timer PWM divider, and SPI
     * divider according to the sampling rate specified by
     * the user. */
    freq_Config(20000);
    /* Configures PWM to pin 2.4 */
    Timer_A_PWMConfig pwmConfig =
    {
        TIMER_A_CLOCKSOURCE_SMCLK,
        pwm_divider,
        1,
        TIMER_A_CAPTURECOMPARE_REGISTER_1,
        TIMER_A_OUTPUTMODE_RESET_SET,
        1
    };
    TXData = 0x00;                                                      // Transmission Dummy Data to generate bit clock
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
    setDRDYPin();
    SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;                               // Wake up on exit from ISR
    __DSB();                                                            // Ensures SLEEPONEXIT takes effect immediately

    while (1)
    {
        __sleep();
        __no_operation();
        // For debug,Remain in LPM0
    }
}
/*
 * Pin Interrupt in Pin 5.4 to receive DRDY from the ADC
 * To start receiving sample data(96 bits)
 */
void PORT5_IRQHandler(void)
{
    if (P5IFG & BIT1)
    {
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
    P5->IFG = 0;                                // Clear Port Interrupt Flag
}
