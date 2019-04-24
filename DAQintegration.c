#include "msp.h"
#include <stdint.h>

void clockSystem(short mode){
    //mode = 0 sets system to 12MHz, mode = 1 sets to 24
    if(mode==1){
        PCM->CTL0 = (uint32_t) 0x695A0001;      //active mode request VCORE1
        while(PCM->CTL1 & BIT8);
        CS->KEY = CS_KEY_VAL ;                  // Unlock CS module for register access
        CS->CTL0 = (uint32_t) 0x08040000;       // Reset tuning parameters
        CS->CTL1 = (uint32_t) 0x10100033;
        CS->KEY = 0;                            // Lock CS module from unintended accesses
    }
//    else{
//        PCM->CTL0 = (uint32_t) 0x695A0000;      //active mode request VCORE0
//        while(PCM->CTL1 & BIT8);
//        CS->KEY = CS_KEY_VAL ;                  // Unlock CS module for register access
//        CS->CTL0 = (uint32_t) 0x08040000;       // DCO enabled and set to 24MHz
//        CS->CTL1 = (uint32_t) 0x10100033;       //MCLK and SMCLK set to DCO, SMCLK and HSMCLK divider set to 2
//        CS->KEY = 0;                            // Lock CS module from unintended accesses
//    }
}

void timersetup(){
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
    TIMER_A2->CTL |= (uint16_t) 0x0200;             //CTL[9:8]=TASSEL=10b - TA takes input from SCLK
    TIMER_A2->CTL |= (uint16_t) 0x0010;             //CTL[5:4]=MC=01b - timer in count up mode (resets when it reaches CCR0 value)
    TIMER_A2->CCTL[1] &= (uint16_t) 0xFEFF;         //CCTL1[8]=CAP=0b set CCTL1 to compare mode
    TIMER_A2->CCTL[1] |= (uint16_t) 0x00E0;         //CCTL1[7:5]=OUTMOD=111b OUT1 is reset when counter reaches CCR1, it is set after reaching CCR0;
}

void FilterFreq(short mode){
    //Using 16MHz system clocks for all but the highest frequency, which uses 12MHz
    if(mode == 8)
        clockSystem(1);
    else
        clockSystem(0);
    //format: timer output - cutoff frequency
    switch(mode){
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
void MuxSelectors(char channels){
    //least significant 4 bits are used as the selectors for channels 4, 3, 2, and 1 respectively
    P8OUT = (channels & 0x0F) << 2;
}

//PGA
void GainSelect(short gain){
    uint8_t option = 0x01;
    switch(gain){
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

void SPIport(){
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

void GPIOinit(){
    //multiplexer selectors
    P8DIR = (uint8_t) 0x3C;

    //pga
    P2DIR = (uint8_t) 0x0B;
    SPIport();

    //TIMER_A2 OUT1
    P5DIR |= BIT6;
    P5SEL1 &= ~BIT6;
    P5SEL0 |= BIT6;
}

void main(void){
   clockSystem(1);
   GPIOinit();
//   MuxSelectors(0x00);
//   GainSelect(1);
   timersetup();
   FilterFreq(8);
}
