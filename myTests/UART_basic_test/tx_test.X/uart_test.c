#include "uart.h"
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "p33FJ128MC802.h"


/*
#define FNOSC_FRC     0x0080  // Internal Fast RC (FRC) (16 MHz > 8 MHz, PLL not allowed)
#define FCKSM_CSECMD  0x0067  // Clock Switching enabled and Fail Safe Clock Monitor disabled
#define OSCIOFNC_OFF  0x00E7  // OSC2 Pin Function: OSC2 is Clock Output
#define POSCMD_NONE   0x00E7  // Primary Oscillator Mode: Disabled

*/

struct myUART* uart;
void printString(char* s){
  int l=strlen(s);
  for(int i=0; i<l; ++i, ++s)
    UART_putChar(uart, (uint8_t) *s);
}

int main(void){

/*    
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
*/    
    uart=UART_init("uart_0",115200);
//    uart=UART_init("uart_0", 57600);
  while(1) {
    char tx_message[300];
    char rx_message[300];
    //rx_message[0]=0;
    rx_message[0]=0x41;
    rx_message[1]=0x61;
    rx_message[2]=0;
    int size=0;
    while(1){
      delayMs(50);  
      sprintf(tx_message, "\nbuffer rx: %d message: ",
	      UART_rxBufferFull(uart));
	      //rx_message);
      printString(tx_message);
      printString(rx_message);
      
      //uint8_t c= UART_getChar(uart);
      //uint8_t c = 0x61;
      //rx_message[size]=c;
      ++size;
      rx_message[size]=0;
      //if (c=='\n' || c=='\r' || c==0)
      
	break;
    }
  }
}
