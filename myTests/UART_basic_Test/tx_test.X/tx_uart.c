#include "uart.h"
#include <string.h>
#include <stdio.h>
#include "p33FJ128MC802.h"

struct myUART* uart;

int main ()
{
   int count =4;
   uart=UART_init("uart_0",115200);
   while(1)
   {
	  //printf("Cycle %d\n", count);
      uint8_t tx = 0x9;
	  UART_putChar(uart, tx);
	  count--;
	  if(!count)
         break;
   }
  return 0;
}