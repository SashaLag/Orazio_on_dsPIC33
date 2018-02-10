#include "eeprom.h"
#include "delay.h"
#include "uart.h"
#include <p33FJ128MC802.h>
#include <string.h>
#include <stdio.h>

#pragma config FNOSC = FRC      // select internal FRC at POR
#pragma config FCKSM = CSECMD   // enable clock switching
#pragma config POSCMD = XT      // configure POSC for XT mode

struct myUART* uart;
void printString(char* s){
  int l=strlen(s);
  for(int i=0; i<l; ++i, ++s)
    UART_putChar(uart, (uint8_t) *s);
}

char* message="I am a string you stored in the EEPROM";

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
  
  EEPROM_init();
  uart=UART_init("uart_0",115200);
  // we write the string on eeprom at address 0
  int size=strlen(message)+1;
  EEPROM_write(0, message, size);

  while(1) {
    char eeprom_buffer[size];
    memset(eeprom_buffer, 0, size);
    // we read the string from the eeprom, in a buffer we clear each time
    EEPROM_read(eeprom_buffer, 0, size); 
    eeprom_buffer[size-1]=0;

    // we wrap it in a nice thing
    char tx_message[128];
    sprintf(tx_message, "buffer string:[%s]\n", eeprom_buffer);
    printString(tx_message);
    delayMs(100);
  }
}
