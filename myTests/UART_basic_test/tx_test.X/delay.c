#define FCY 16006719UL
//#define FCY 10000000UL
#include "delay.h"
#include <libpic30.h>
//#define scale FCY/1000000
void delayMs(uint16_t ms __attribute__((unused))){
  //ms = (uint16_t)1000*ms/scale;
  while (ms) {
    __delay_ms(1);
    --ms;
  }
}
