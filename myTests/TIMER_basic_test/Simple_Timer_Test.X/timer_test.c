#include "timer.h"
#include "uart.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>
#include "p33FJ128MC802.h"

#pragma config FNOSC = FRC      // select internal FRC at POR
#pragma config FCKSM = CSECMD    // enable clock switching
#pragma config POSCMD = XT    // configure POSC for XT mode

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
LATB = 0x0;


  uart=UART_init("uart_0",115200);
  Timers_init();
  volatile uint16_t do_stuff;
  struct Timer* timer=Timer_create("timer_0", 100, timerFn, (void*) &do_stuff); 
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
