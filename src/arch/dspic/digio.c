#include "digio.h"
#include <p33FJ128MC802.h>
#define NUM_CHANNELS 14

// initializes the digital io pins of the chip
void DigIO_init(void) {
    AD1PCFGL = 0xFFFF;
}

// returns the number of digital io pins on the chip
uint8_t  DigIO_numChannels(void){
  return NUM_CHANNELS;
}

uint8_t DigIO_setDirection(uint8_t pin, PinDirection dir) {
  if (pin>=NUM_CHANNELS)
    return 1;
  return 0;
}

PinDirection DigIO_getDirection(uint8_t pin){
  if (pin>=NUM_CHANNELS)
    return 0;
  return 1;
}

void DigIO_setValue(uint8_t pin, uint8_t value) {
  if (pin>=NUM_CHANNELS)
    return;
  return;
}

uint8_t DigIO_getValue(uint8_t pin){
  if (pin>=NUM_CHANNELS)
    return 0;
  return 0;
}
