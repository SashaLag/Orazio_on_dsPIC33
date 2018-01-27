#define FCY 16000000UL
//#define FCY 10859375UL

#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "p33FJ128MC802.h"
#include <libpic30.h>

/* OLD WAY:
_FOSCSEL(FNOSC_FRC & IESO_OFF); // Internal FRC start-up without PLL,
// no Two Speed Start-up
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF & POSCMD_XT); // Clock switch enabled,
// Primarly Oscillator XT
_FWDT(FWDTEN_OFF); // Watchdog Timer disabled
//_FPOR(PPWRT_PWR128); // Power-up Timer enabled 128 ms
_FICD(JTAGEN_OFF); // Disable JTAG
*/

#pragma config FNOSC = FRC      // select internal FRC at POR
#pragma config FCKSM = CSECMD    // enable clock switching
#pragma config POSCMD = XT    // configure POSC for XT mode

int main(void){
  // Configure Oscillator to operate the device at 16 Mhz, Fcy=Fosc/2
  // Fosc= Fin*M/(N1*N2) = 10M*64/(4*5)=32Mhz for 10M input clock
  PLLFBD=62;             // M=64
  CLKDIVbits.PLLPOST=1;		// N1=4
  CLKDIVbits.PLLPRE=3;		// N2=5
  OSCTUN=0;					      // Tune FRC oscillator, if FRC is used

__builtin_write_OSCCONH(0x03);//__builtin_write_OSCCONH(0x01); //for FRC
__builtin_write_OSCCONL(OSCCON | 0x01);
// Wait for Clock switch to occur
while (OSCCONbits.COSC != 0b011); //while (OSCCONbits.COSC != 0b001); // for FRC
// Wait for PLL to lock
while(OSCCONbits.LOCK!=1) {};

  TRISB=0;
  LATB = 0xFFFF;
  
  while(1) {
    LATB = ~LATB;
    __delay_ms(1000);
    //delayMs(625); // ~1 sec and something for some reason 
      
  }
}
