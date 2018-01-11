#include "uart.h"
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "p33FJ128MC802.h"

struct myUART* uart;
void printString(char* s){
  int l=strlen(s);
  for(int i=0; i<l; ++i, ++s)
    UART_putChar(uart, (uint8_t) *s);
}

int main(void){
  uart=UART_init("uart_0",115200);
  while(1) {
    char tx_message[300];
    char rx_message[300];
    uint32_t intero = 30000000;
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
