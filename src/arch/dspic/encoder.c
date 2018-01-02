#include <dsp.h>
#include "encoder.h"
#include "p33FJ128MC802.h"

#define NUM_ENCODERS 2
uint16_t EncodersValue[NUM_ENCODERS] = { 0 };

// initializes the encoder subsystem
void Encoder_init(void){
  AD1PCFGL |= 0x0038;     // Configure QEI pins as digital inputs
  QEICONbits.CNTERR = 0;  // Clear any count errors
  QEICONbits.QEISIDL = 0; // Continue operation during sleep
  QEICONbits.QEIM = 0;    // Disable QEI Module
  QEICONbits.SWPAB = 0;   // QEA and QEB not swapped
  QEICONbits.PCDOUT = 0;  // Normal I/O pin operation
  QEICONbits.POSRES = 1;  // Index pulse resets position counter
  POS1CNT = 0;            // Reset position first counter
  POS2CNT = 0;            // Reset position second counter
  DFLTCONbits.CEID = 1;   // Count error interrupts disabled
  DFLTCONbits.QEOUT = 1;  // Digital filters output enabled for QEn pins
  DFLTCONbits.QECK = 5;   // 1:64 clock divide for digital filter for QEn
  QEICONbits.QEIM = 6;    // X4 mode with position counter reset by Index
}

// samples the encoders, saving the respective values in a temporary storage
void Encoder_sample(void){
  asm volatile ("disi #0x3FFF");  // disable all user interrupts (atomically)
    EncodersValue[0]=POS1CNT;
    EncodersValue[1]=POS2CNT;
  asm volatile ("disi #0");       //enable all user interrupts (atomically)
}

// returns the number of the encoder
uint8_t Encoder_numEncoders(void){
  return NUM_ENCODERS;
}

// returns the value of an encoder, when sampled with the Encoder_sample();
uint16_t Encoder_getValue(uint8_t num_encoder){
  return EncodersValue[num_encoder];
}
