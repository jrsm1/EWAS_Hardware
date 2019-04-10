#include "msp.h"

void clockSystem(){
    CS->KEY = CS_KEY_VAL ;                  // Unlock CS module for register access
    CS->CTL0 = (uint32_t) 0x00000000;       // Reset tuning parameters
    CS->CTL2 = (uint32_t) 0x00610000;       //HFXT_EN, HFXTFREQ = 6h (40-48MHz), HFXTDRIVE = 1;
    PJSEL1 &= ~BIT3;                        //connecting launchpad 48MHz crystal
    PJSEL0 |= BIT3;
    CS->CTL1 = (uint32_t) 0x00000005;       //MCLK - HFXT
    CS->KEY = 0;                            // Lock CS module from unintended accesses
}

void main(void)
{

    PCM->CTL0 = (uint32_t) 0x695A0001;      //active mode request VCORE1
    short i;
    for(i=0;i<100;i++);
    P4DIR = BIT3;
    P4SEL1 &= ~BIT3;
    P4SEL0 |= BIT3;
    clockSystem();
    for(;;);

}
