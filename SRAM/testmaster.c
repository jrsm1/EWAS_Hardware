#include "msp.h"
#include <stdio.h>

//sets the slave address for i2c
void setSlave(int address){
    EUSCI_B0->I2CSA = (uint16_t) address;
}

//all the i2c setup instructions put into a function because it needs to be set up more than once
void i2cinit(){
    EUSCI_B0->CTLW0 = (uint16_t) 0x0001;               //CTLW0[0] = UCSWRST = 1b       /set so we can modify the rest of the register
    EUSCI_B0->CTLW0 &= (uint16_t) 0x7FFF;               //CTLW0[15] = UCA10 = 0b        /set own address to 7bit
    EUSCI_B0->CTLW0 |= (uint16_t) 0x0800;               //CTLW0[11] = UCMST = 1b        /master mode
    EUSCI_B0->CTLW0 |= (uint16_t) 0x0600;               //CLTW0[10:9] = UCMODE = 11b    /I2C mode
    EUSCI_B0->CTLW0 |= (uint16_t) 0x0040;               //CTLW0[7:6] = UCSSEL = 01b     /ACLK source
    EUSCI_B0->CTLW1 |= (uint16_t) 0x0008;               //CTLW1[3:2] = UCASTP = 10b     /automatic stop with byte count

    EUSCI_B0->BRW |= (uint16_t) 0x1;                    //baud rate

    //EUSCI_B0 SDA
    P1SEL1 &= ~BIT6;
    P1SEL0 |= BIT6;
    //EUSCI_B0 SCL
    P1SEL1 &= ~BIT7;
    P1SEL0 |= BIT7;

    EUSCI_B0->CTLW0 &= (uint16_t) 0xFFFE;               //UCSWRST = 0

    EUSCI_B0->IE = 0x000F;                                //interrupts enabled
    NVIC->ISER[0] = 1 << (EUSCIB0_IRQn);
}

void readslave(char n){
    EUSCI_B0->TBCNT = (uint16_t) n;            //byte counter
    EUSCI_B0->CTLW0 &= (uint16_t) 0xFFEF;       //CTLW0[4] = UCTR = 0b  /receiver
    EUSCI_B0->CTLW0 |= BIT1;                    //start
}

void main(void){

    //Clock system
    CS->KEY = CS_KEY_VAL;                               //setting key to make clock registers editable
    CS->CTL1 |= (uint32_t) 0x00000200;                  //CTL1[10:8]=SELA=010b - ACLK source set to REFOCLK
    CS->CTL1 |= (uint32_t) 0x00000000;                  //CTL1[26:24]=DIVA=111b - ACLK div = 128
    CS->CLKEN |= (uint32_t) 0x00008000;                 //CLKEN[15]=REFOFSEL=1b - REFOCLK set to 128kHz
    CS->KEY = 0;

    i2cinit();

    NVIC->ISER[0] = 1 << (EUSCIB0_IRQn);

    setSlave(0x60);
    readslave(5);

    for(;;);
}

uint8_t data[200];
int datacount = 0;
void EUSCIB0_IRQHandler(void){
    if(EUSCI_B0->IFG & BIT0){
        data[datacount++] = EUSCI_B0->RXBUF;
        EUSCI_B0->IFG = 0;
    }
    else EUSCI_B0->IFG = 0;
}
