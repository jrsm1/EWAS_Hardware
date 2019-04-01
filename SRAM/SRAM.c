#include "msp.h"

//P10   , P9   , P7
//A19-16, A15-8, A7-0
//High Byte, Low Byte
//P4.2     , P4.1
//Write Enable, Output Enable, Chip Enable 2
//P4.3        , P4.2         , P3.0


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

void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	SRAMinit();

	//SRAM testing

	    //creating array of 200 bytes
	uint8_t storethis[200];
	uint8_t readback[200];
	volatile int i;
	for(i=0;i<200;i++){
	    storethis[i] = i;
	}

	    //Storing the data in SRAM
	P7OUT = 0x00;   //initial address 0
	P4OUT = 0x06;   //write low byte
	for(i=0;i<200;i++){
	    readback[i] = 0;    //clearing array that will receive the data later
	    P2OUT = storethis[i];
	    P3OUT = 0x01;       //enable to write
	    P3OUT = 0x00;       //disable to prepare next byte
	    P7OUT++;            //increment address
	}

	    //Reading the data back from SRAM
	P4OUT = 0x0A;   //output low byte
	P2OUT = 0x00;   //switching P2 to input
	P2DIR = 0x00;
	P7OUT = 0x00;   //resetting address to 0
	P3OUT = 0x01;   //enabling chip
	for(i=0;i<200;i++){
//	    P4OUT = 0x09; //(i%2==0) ? 0x0A : 0x09;
//	    P3OUT = 0x01;
	    readback[i] = (uint8_t) P2IN;
	    P7OUT++;
	}
	P3OUT = 0x00;   //disable chip after finished

//	short size = 5;
//	//mapping memory
//	uint8_t premem2[5], premem6[5], postmem2[5], postmem6[5];
//	P2OUT = 0x00; P6OUT = 0x00;
//    P2DIR = 0x00; P6DIR = 0x00;  //data port cleared and turned to input for reading
//    P4OUT = 0x09;   //write disabled, output enabled
//    P3OUT = 0x01;   //chip enabled
//	for(i=0;i<size;i++){
//	    premem2[i] = P2IN;
//	    premem6[i] = P6IN;
//	    P7OUT++;
//	}
//
//	//writing to random location
//	P2DIR = 0xFF; P6DIR = 0xFF;
//	P2OUT = 0x74;   //data to write
//	P6OUT = 0xAA;
//	P7OUT = 0x00;   //address to write to
//	P4OUT = 0x06;   //write low
//	P3OUT |= 0x01;  //chip enable
//	P3OUT &= 0x00;  //chip disabled
//
//	//remapping to verify changes
//	P7OUT = 0x00;
//	P2OUT = 0x00; P6OUT = 0x00;
//    P2DIR = 0x00; P6DIR = 0x00;
//    P4OUT = 0x09;   //write disabled, output enabled
//    P3OUT = 0x01;
//	for(i=0;i<size;i++){
//        postmem2[i] = P2IN;
//        postmem6[i] = P6IN;
//        P7OUT++;
//    }
}
