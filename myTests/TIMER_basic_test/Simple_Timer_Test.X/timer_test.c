#include "timer.h"
#include "uart.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>
#include "p33FJ128MC802.h"

#define FNOSC_FRC     0x0080  // Internal Fast RC (FRC) (16 MHz > 8 MHz, PLL not allowed)
#define FCKSM_CSECMD  0x0067  // Clock Switching enabled and Fail Safe Clock Monitor disabled
#define OSCIOFNC_OFF  0x00E7  // OSC2 Pin Function: OSC2 is Clock Output
#define POSCMD_NONE   0x00E7  // Primary Oscillator Mode: Disabled

struct myUART* uart;
void printString(char* s){
  int l=strlen(s);
  for(int i=0; i<l; ++i, ++s)
    UART_putChar(uart, (uint8_t) *s);
}

void timerFn(void* args){
  uint16_t* argint=(uint16_t*)args;
  *argint=1;
}

int main(void){
	
	  // Configure Oscillator to operate the device at 16 Mhz, Fcy=Fosc/2
  // Fosc= Fin*M/(N1*N2) = 7.37M*139/(8*8)=32.013Mhz for 7.37M input clock
  PLLFBD=137;             // M=139
  CLKDIVbits.PLLPOST=3;		// N1=8
  CLKDIVbits.PLLPRE=6;		// N2=8
  OSCTUN=0;					      // Tune FRC oscillator, if FRC is used

  // Clock switch to incorporate PLL (NOSC=0b001)
  __builtin_write_OSCCONH(0x01);	 // Initiate Clock Switch to FRC w/ PLL
  __builtin_write_OSCCONL(0x01);   // Start clock switching
//  while (OSCCONbits.COSC!=0x01);	 // Wait for Clock switch to occur
//  while(OSCCONbits.LOCK!=1);       // Wait for PLL to lock

	
	
  uart=UART_init("uart_0",115200);
  Timers_init();
  volatile uint16_t do_stuff;
  struct Timer* timer=Timer_create("timer_0", 5, timerFn, (void*) &do_stuff); 
  Timer_start(timer);
  uint16_t tick=0;
  while(1) {
    if (do_stuff){
      char buffer[20];
      ++tick;
      sprintf(buffer, "Tick: [ %05d]\n ", tick);
      printString(buffer);
      char reg[50];
      sprintf(reg, "PR3 value: %d\n", PR3);
      printString(reg);
      do_stuff=0;
    }
  }
}
