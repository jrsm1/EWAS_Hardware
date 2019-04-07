#include "msp.h"

//data going to master
uint8_t sendthis;

void SRAMinit(){
    //////////Initialization of necessary ports
        //SRAM I/O: port 2  //single byte io mode
        P2DIR = 0xFF;
        P2OUT = 0x00;

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
    EUSCI_B2->BRW |= (uint16_t) 0xFF;                    //(int) clk/baud rate

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

void main(void){
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    SRAMinit();

    //preload ram

    //creating dummy batch of data to store
    uint8_t storethis[200];
    uint8_t readback[200];
    volatile int i;
    for(i=0;i<200;i++){
        storethis[i] = i;
    }
    //storing in SRAM
    P7OUT = 0xF0;   //initial address
    P9OUT = 0x00;
    P10OUT = 0x00;
    P4OUT = 0x06;   //write low byte
    for(i=0;i<200;i++){
        P2OUT = storethis[i];
        P3OUT = 0x01;       //enable to write
        P3OUT = 0x00;       //disable to prepare next byte
        if(++P7OUT == 0)    //increment address
            if(++P9OUT == 0)
                P10OUT++;
    }
    //prepare for reading
    P4OUT = 0x0A;   //output low byte
    P2OUT = 0x00;   //switching P2 to input
    P2DIR = 0x00;
    P7OUT = 0xF0;   //resetting address
    P9OUT = 0x00;
    P10OUT = 0x00;
    P3OUT = 0x01;   //enabling chip
    sendthis = (uint8_t) P2IN;

//    //Reading the data back from SRAM
//    P4OUT = 0x0A;   //output low byte
//    P2OUT = 0x00;   //switching P2 to input
//    P2DIR = 0x00;
//    P7OUT = 0xF0;   //resetting address
//    P9OUT = 0x00;
//    P10OUT = 0x00;
//    P3OUT = 0x01;   //enabling chip
//    for(i=0;i<200;i++){
////      P4OUT = 0x09; //(i%2==0) ? 0x0A : 0x09;
////      P3OUT = 0x01;
//        readback[i] = (uint8_t) P2IN;
//        if(++P7OUT == 0)
//                    if(++P9OUT == 0)
//                        P10OUT++;
//    }
//    P3OUT = 0x00;   //disable chip after finished

    i2cinit();

    for(;;);
}

//moving data to master
void EUSCIB2_IRQHandler(void){
    if(EUSCI_B2->IFG & BIT3){
        EUSCI_B2->IFG = 0;
    }
    else if(EUSCI_B2->CTLW0 & BIT4){
        EUSCI_B2->TXBUF = sendthis;
        if(++P7OUT == 0)
            if(++P9OUT == 0)
                P10OUT++;
        sendthis = (uint8_t) P2IN;
        EUSCI_B2->IFG = 0;
    }
}
