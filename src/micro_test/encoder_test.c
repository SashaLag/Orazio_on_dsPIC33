#include "encoder.h"
#include "delay.h"
#include "uart.h"
#include <p33FJ128MC802.h>
#include <stdio.h>
#include <string.h>

#pragma config FNOSC = FRC      // select internal FRC at POR
#pragma config FCKSM = CSECMD   // enable clock switching
#pragma config POSCMD = XT      // configure POSC for XT mode

struct myUART* uart;
void printString(char* s){
  int l=strlen(s);
  for(int i=0; i<l; ++i, ++s)
    UART_putChar(uart, (uint8_t) *s);
}

int main(void){
  // Configure Oscillator to operate the device at 16 Mhz, Fcy=Fosc/2
  // Fosc= Fin*M/(N1*N2) = 10M*64/(4*5)=32Mhz for 10M input clock
  PLLFBD=62;              // M=64
  CLKDIVbits.PLLPOST=1;		// N1=4
  CLKDIVbits.PLLPRE=3;		// N2=5
  __builtin_write_OSCCONH(0x03);//__builtin_write_OSCCONH(0x01); //for FRC
  __builtin_write_OSCCONL(OSCCON | 0x01);
  while (OSCCONbits.COSC != 0b011); // Wait for Clock switch to occur
  while(OSCCONbits.LOCK!=1) {};     // Wait for PLL to lock  
    
  Encoder_init();
  
  uart=UART_init("uart_0",115200);
  while(1) {
    Encoder_sample();
    char buffer[300];
    printString("encoders: [");
    for (int i=0; i<Encoder_numEncoders(); ++i){
      sprintf(buffer, " %05d ", Encoder_getValue(i));
      printString(buffer);
    }
    printString("]\n");
    delayMs(100);
  }
}
