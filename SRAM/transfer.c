#include "msp.h"

void SRAMinit(){
    //////////Initialization of necessary ports
        //SRAM I/O: ports 2 & 6
        P2DIR = 0xFF; //LOW
    //  P6DIR = 0xFF; //HIGH

        P2OUT = 0x00;
    //  P6OUT = 0x00;

        //SRAM address ports
        P10DIR = 0xFF; //MSB
        P9DIR = 0xFF;
        P7DIR = 0xFF;   //LSB

        P7OUT = 0x00;
        P9OUT = 0x00;
        P10OUT = 0x00;

        //chip select CE2 (CE1 goes to ground since we only have 1 IC)
        P3DIR |= 0x01;
        P3OUT &= 0x00;

        //write, output, byte high, and byte low enable signals
        P4DIR |= 0x0F;
        P4OUT |= 0x0F;
}

//i2c setup instructions
void i2cinit(){
    EUSCI_B2->CTLW0 = (uint16_t) 0x0001;                //CTLW0[0] = UCSWRST = 1b       /set so we can modify the rest of the register
    EUSCI_B2->CTLW0 &= (uint16_t) 0x7FFF;               //CTLW0[15] = UCA10 = 0b        /set own address to 7bit
    EUSCI_B2->CTLW0 &= (uint16_t) 0xF7FF;               //CTLW0[11] = UCMST = 0b        /slave
    EUSCI_B2->CTLW0 |= (uint16_t) 0x0600;               //CLTW0[10:9] = UCMODE = 11b    /I2C mode
    EUSCI_B2->BRW |= (uint16_t) 0xD;                    //(int) clk/baud rate           /baud rate=10000

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

void clockSystem(){
    CS->KEY = CS_KEY_VAL ;                  // Unlock CS module for register access
    CS->CTL0 = 0;                           // Reset tuning parameters
    CS->CTL0 = CS_CTL0_DCORSEL_3;           // Set DCO to 12MHz
    // Select ACLK = REFO, SMCLK = MCLK = DCO
    CS->CTL1 = CS_CTL1_SELA_2 | CS_CTL1_SELS_3 | CS_CTL1_SELM__DCOCLK;
    CS->KEY = 0;                            // Lock CS module from unintended accesses
}

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    clockSystem();
    i2cinit();
    SRAMinit();
}

int datacount = 0;
void EUSCIA2_IRQHandler(void){
    if(EUSCI_B2->CTLW0 & BIT4){
        if(datacount == 0){
            P7OUT = (uint8_t) 0x0;
            P9OUT = (uint8_t) 0x0;
            P10OUT = (uint8_t) 0x0;
            P2OUT = (uint8_t) 0x0;
            P2DIR = (uint8_t) 0x0;
            P4OUT = (uint8_t) 0xA;
            P3OUT = (uint8_t) 0x1;
            EUSCI_B2->TXBUF = (uint8_t) P2IN;
        }
        if(datacount!=200){
            if(++P7OUT == 0)
                if(++P9OUT == 0)
                    P10OUT++;
            EUSCI_B2->TXBUF = P2IN;
        }
    }
}
