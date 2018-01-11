#define FNOSC_FRC     0x0080  // Internal Fast RC (FRC) (16 MHz > 8 MHz, PLL not allowed)
#define FCKSM_CSECMD  0x0067  // Clock Switching enabled and Fail Safe Clock Monitor disabled
#define OSCIOFNC_OFF  0x00E7  // OSC2 Pin Function: OSC2 is Clock Output
#define POSCMD_NONE   0x00E7  // Primary Oscillator Mode: Disabled
//#include <xc.h>
#include "eeprom.h"
#include "p33FJ128MC802.h"
#include "DEE Emulation 16-bit.h"

void EEPROM_init(void){
  // Configure Oscillator to operate the device at 16 Mhz, Fcy=Fosc/2
  // Fosc= Fin*M/(N1*N2) = 7.37M*139/(8*8)=32.013Mhz for 7.37M input clock
  PLLFBD=137;             // M=139
  CLKDIVbits.PLLPOST=3;		// N1=8
  CLKDIVbits.PLLPRE=6;		// N2=8
  OSCTUN=0;					      // Tune FRC oscillator, if FRC is used

  // Clock switch to incorporate PLL (NOSC=0b001)
  __builtin_write_OSCCONH(0x01);	 // Initiate Clock Switch to FRC w/ PLL
  __builtin_write_OSCCONL(0x01);   // Start clock switching
  while(OSCCONbits.COSC!=0x01); 	 // Wait for Clock switch to occur
  while(OSCCONbits.LOCK!=1);       // Wait for PLL to lock

  DataEEInit();
  dataEEFlags.val = 0;
}

// There are 2 Nop(); after Read and Write. But there is 2 increment. May not be useful
void EEPROM_read(void* dest_, const uint16_t src, uint16_t size){
  unsigned int *dest=(unsigned int*)dest_;
  unsigned int s=src;
  unsigned int s_end=(unsigned int)src+(unsigned int)size;
  while( s<s_end ){
    *dest=DataEERead(s);
    ++s;
    ++dest;
  }
}

void EEPROM_write(uint16_t dest, const void* src_,  uint16_t size){
  unsigned int *s=(unsigned int*)src_;
  unsigned int s_end=(unsigned int)s+(unsigned int)size;
  while( s<s_end ){
    DataEEWrite(*s, (unsigned int)dest);
    ++s;
    ++dest;
  }
}
