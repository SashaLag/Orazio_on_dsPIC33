//#include "digio.h"
#include "pwm.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>
#include "uart.h"

#pragma config FNOSC = FRC      // select internal FRC at POR
#pragma config FCKSM = CSECMD    // enable clock switching
#pragma config POSCMD = XT    // configure POSC for XT mode

struct myUART* uart;
void printString(char* s){
  int l=strlen(s);
  for(int i=0; i<l; ++i, ++s)
    UART_putChar(uart, (uint8_t) *s);
}

void printPWMStatus(void){
  uint8_t num_channels=PWM_numChannels();
  char buffer[128];
  char* bend=buffer+sprintf(buffer, "enable=[");
  for (int i=0; i<num_channels; ++i)
    bend+=sprintf(bend,"%d ", PWM_isEnabled(i));
  bend+=sprintf(bend,"]\n");
  bend+=sprintf(bend, "value=[");
  for (int i=0; i<num_channels; ++i)
    bend+=sprintf(bend,"%d ", PWM_getDutyCycle(i));
  bend+=sprintf(bend,"]\n");
  printString(buffer);
}

int main(void){
  
  // Configure Oscillator to operate the device at 16 Mhz, Fcy=Fosc/2
  // Fosc= Fin*M/(N1*N2) = 10M*64/(4*5)=32Mhz for 10M input clock
  PLLFBD=62;             // M=64
  CLKDIVbits.PLLPOST=1;		// N1=4
  CLKDIVbits.PLLPRE=3;		//N2=5
  OSCTUN=0;					      // Tune FRC oscillator, if FRC is used

  __builtin_write_OSCCONH(0x03);//__builtin_write_OSCCONH(0x01); //for FRC
  __builtin_write_OSCCONL(OSCCON | 0x01);
  // Wait for Clock switch to occur
  while (OSCCONbits.COSC != 0b011); //while (OSCCONbits.COSC != 0b001); // for FRC
  // Wait for PLL to lock
  while(OSCCONbits.LOCK!=1) {};
    
    // all pins as inputs with pull up resistors
  PWM_init();

  // we enable all pwm outputs
  for (uint8_t i=0; i<PWM_numChannels(); ++i)
    PWM_enable(i, 1);


  uart=UART_init("uart_0",115200);
  printPWMStatus();
  uint8_t start_pin=0;
  while(1) {
    for (int k=0; k<PWM_numChannels(); ++k){
      uint8_t pin=(start_pin+k)%PWM_numChannels();
      PWM_setDutyCycle(pin,(k*255)/PWM_numChannels());
      //PWM_setDutyCycle(pin,20);
    }
    printPWMStatus();
    ++start_pin;
    if (start_pin>=PWM_numChannels())
      start_pin=0;
    delayMs(1000);
  }
}
