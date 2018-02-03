#define FP 16000000
#define BAUDRATE 9600
#define BRGVAL ((FP/BAUDRATE)/16)-1
unsigned int i;
#define DELAY_105uS asm volatile ("REPEAT, #4201"); Nop(); // 105uS delay

#include "uart.h"
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include <p33FJ128MC802.h>
#include <libpic30.h>

#pragma config FNOSC = FRC      // select internal FRC at POR
#pragma config FCKSM = CSECMD    // enable clock switching
#pragma config POSCMD = XT    // configure POSC for XT mode

int main(void){

    //UART
  RPINR18bits.U1RXR = 3; //INPUT
  RPOR1bits.RP2R = 3; //OUTPUT
  AD1PCFGL = 0xFFFF; //FONDAMENTALE PER LA UART, DEVO CONFIGURARE QUEI PIN COME DIGITALI, DI BASE SONO ANALOGICI
    
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

  U1MODEbits.STSEL = 0; // 1-Stop bit
  U1MODEbits.PDSEL = 0; // No Parity, 8-Data bits
  U1MODEbits.ABAUD = 0; // Auto-Baud disabled
  U1MODEbits.BRGH = 0; // Standard-Speed mode
  U1BRG = BRGVAL; // Baud Rate setting for 9600
  U1STAbits.UTXISEL0 = 0; // Interrupt after one TX character is transmitted
  U1STAbits.UTXISEL1 = 0;
  IEC0bits.U1TXIE = 1; // Enable UART TX interrupt
  U1MODEbits.UARTEN = 1; // Enable UART
  U1STAbits.UTXEN = 1; // Enable UART TX
  /* Wait at least 105 microseconds (1/9600) before sending first char */
  DELAY_105uS
  U1TXREG = 'a'; // Transmit one character
  while(1)
  {
      IEC0bits.U1TXIE = 1;
      
      //IFS0bits.U1TXIF = 0;
  }
}
void __attribute__ ((interrupt, no_auto_psv)) _U1TXInterrupt()
{
IFS0bits.U1TXIF = 0; // Clear TX Interrupt flag
U1TXREG = 'a'; // Transmit one character
IEC0bits.U1TXIE = 0; 
}

