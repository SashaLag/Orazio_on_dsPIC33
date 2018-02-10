//#define OSCIOFNC_OFF  0x00E7  // OSC2 Pin Function: OSC2 is Clock Output
//#include <xc.h>
#include "eeprom.h"
#include "p33FJ128MC802.h"
#include "DEE Emulation 16-bit.h"
#include <stdint.h>

void EEPROM_init(void){
  DataEEInit();
  dataEEFlags.val = 0;
}

// There are 2 Nop(); after Read and Write. But there is 2 increment. May not be useful
void EEPROM_read(void* dest_, const uint16_t src, uint16_t size){
  uint8_t *dest=(uint8_t*)dest_;
  uint16_t s=src;
  uint16_t s_end=src+size;
  while( s<s_end ){
    *dest=(uint8_t)DataEERead((unsigned int)s);
    ++s;
    ++dest;
  } //there should be two nop() after read. there are incrementations instead.
}

void EEPROM_write(uint16_t dest, const void* src_,  uint16_t size){
  const uint8_t *s=(uint8_t*)src_;
  const uint8_t *s_end=s+size;
  while( s<s_end ){
    DataEEWrite((unsigned int)*s, (unsigned int)dest);
    ++s;
    ++dest;
  }
}
