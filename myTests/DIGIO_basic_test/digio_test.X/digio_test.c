#include "digio.h"
#include "uart.h"
#include "delay.h"
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

void printDigioStatus(void){
  uint8_t num_channels=DigIO_numChannels();
  char buffer[128];
  char* bend=buffer+sprintf(buffer, "direction=[");
  for (int i=0; i<num_channels; ++i)
    bend+=sprintf(bend,"%d", DigIO_getDirection(i));
  bend+=sprintf(bend,"]\n");
  bend+=sprintf(bend, "value=[");
  for (int i=0; i<num_channels; ++i)
    bend+=sprintf(bend,"%d", DigIO_getValue(i));
  bend+=sprintf(bend,"]\n");
  printString(buffer);
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

  // all pins as inputs with pull up resistors
  DigIO_init();

  // all pins as output
  for (uint8_t i=0; i<DigIO_numChannels(); ++i)
    DigIO_setDirection(i, 1);


  uart=UART_init("uart_0",115200);
  uint8_t start_pin=0;
  while(1) {
    for (int k=0; k<DigIO_numChannels(); ++k){
      uint8_t pin=(start_pin+k)%DigIO_numChannels();
      DigIO_setValue(pin,k%2);
    }
    printDigioStatus();
    ++start_pin;
    if (start_pin>=DigIO_numChannels())
      start_pin=0;
    delayMs(1000);
  }
}
