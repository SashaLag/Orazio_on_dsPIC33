#define FCY 40000000UL

#include "uart.h"
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "p33FJ128MC802.h"
#include <libpic30.h>

#pragma config FNOSC = FRC      // select internal FRC at POR
#pragma config FCKSM = CSECMD    // enable clock switching
#pragma config POSCMD = XT    // configure POSC for XT mode

struct myUART* uart;
void printString(char* s){
  int l=strlen(s);
  for(int i=0; i<l; ++i, ++s)
    UART_putChar(uart, (uint8_t) *s);
}

int main(void){


  // Configure Oscillator to operate the device at 16 Mhz, Fcy=Fosc/2
  // Fosc= Fin*M/(N1*N2) = 10M*64/(4*5)=32Mhz for 10M input clock
  PLLFBD=30;//62;             // M=64
  CLKDIVbits.PLLPOST=0;		// N1=4
  CLKDIVbits.PLLPRE=1;		//N2=5
  OSCTUN=0;					      // Tune FRC oscillator, if FRC is used

__builtin_write_OSCCONH(0x03);//__builtin_write_OSCCONH(0x01); //for FRC
__builtin_write_OSCCONL(OSCCON | 0x01);
// Wait for Clock switch to occur
while (OSCCONbits.COSC != 0b011); //while (OSCCONbits.COSC != 0b001); // for FRC
// Wait for PLL to lock
while(OSCCONbits.LOCK!=1) {};

    TRISB=0;
    LATB = 0x0;

    uart=UART_init("uart_0",9600);
//    uart=UART_init("uart_0", 57600);
  while(1) {
    char tx_message[300];
    char rx_message[300];
    //rx_message[0]=0;
    rx_message[0]=0x41;
    rx_message[1]=0x61;
    rx_message[2]=0; 
    //__delay_ms(1000);
      sprintf(tx_message, "ABCD");          
      printString(tx_message);
  }
}
