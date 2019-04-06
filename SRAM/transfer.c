#include "msp.h"

uint8_t data[200];

//i2c setup instructions
void i2cinit(){
    EUSCI_B2->CTLW0 = (uint16_t) 0x0001;                //CTLW0[0] = UCSWRST = 1b       /set so we can modify the rest of the register
    EUSCI_B2->CTLW0 &= (uint16_t) 0x7FFF;               //CTLW0[15] = UCA10 = 0b        /set own address to 7bit
    EUSCI_B2->CTLW0 &= (uint16_t) 0xF7FF;               //CTLW0[11] = UCMST = 0b        /slave
    EUSCI_B2->CTLW0 |= (uint16_t) 0x0600;               //CLTW0[10:9] = UCMODE = 11b    /I2C mode
    EUSCI_B2->BRW |= (uint16_t) 0x2;                    //(int) clk/baud rate           /baud rate=10000

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

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
    short i;
    for(i=0;i<200;i++){
        data[i] = i;
    }
    i2cinit();

    for(;;);
}

//moving dummy data to master
int datacount = 0;
void EUSCIB2_IRQHandler(void){
    if(EUSCI_B2->CTLW0 & BIT4){
        EUSCI_B2->TXBUF = data[datacount++];
        EUSCI_B2->IFG = 0;
    }
}

////moving data from sram to master
//int datacount = 0;
//void EUSCIA2_IRQHandler(void){
//    if(EUSCI_B2->CTLW0 & BIT4){
//        if(datacount == 0){
//            P7OUT = (uint8_t) 0x0; P9OUT = (uint8_t) 0x0; P10OUT = (uint8_t) 0x0;   //address reset
//            P2OUT = (uint8_t) 0x0; P2DIR = (uint8_t) 0x0;                           //ready io
//            P4OUT = (uint8_t) 0xA;                                                  //read enable
//            P3OUT = (uint8_t) 0x1;                                                  //chip enable
//            EUSCI_B2->TXBUF = (uint8_t) P2IN;                                       //start moving data
//            datacount++;
//        }
//        else{ //if(datacount!=200){
//            if(++P7OUT == 0)
//                if(++P9OUT == 0)
//                    P10OUT++;
//            EUSCI_B2->TXBUF = P2IN;
//            datacount++;
//        }
//    }
//}
