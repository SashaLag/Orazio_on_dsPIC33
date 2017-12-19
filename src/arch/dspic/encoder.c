#include "encoder.h"

#define NUM_ENCODERS 2


// initializes the encoder subsystem
void Encoder_init(void){
  // the dspic has 2 QEI hardware decoders, configure them
}

// samples the encoders, saving the respective values in a temporary storage
void Encoder_sample(void){
}

// returns the number of the encoder
uint8_t Encoder_numEncoders(void){
  return NUM_ENCODERS;
}

// returns the value of an encoder, when sampled with the Encoder_sample();
uint16_t Encoder_getValue(uint8_t num_encoder){
  return 0;
}
 
