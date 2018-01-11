#include "pwm.h"
#define NUM_CHANNELS 14

// here we use timers 0-4, in 8 bit mode
// regardless if they are 16 bit


// initializes the pwm subsystem
PWMError PWM_init(void){
  return PWMSuccess;
}

// how many pwm on this chip?
uint8_t PWM_numChannels(void){
  return NUM_CHANNELS;
}

// what was the period i set in the pwm subsystem
// might only require to adjust the prescaler
PWMError PWM_isEnabled(uint8_t c) {
  if (c>=NUM_CHANNELS)
    return PWMChannelOutOfBound;
  return PWMEnabled;
}

// sets the output on a pwm channel
PWMError PWM_enable(uint8_t c, uint8_t enable){
  if (c>=NUM_CHANNELS)
    return PWMChannelOutOfBound;
    return PWMSuccess;
}


// what was the duty cycle I last set?
uint8_t PWM_getDutyCycle(uint8_t c){
  if (c>=NUM_CHANNELS)
    return PWMChannelOutOfBound;
  return 0;
}

// sets the duty cycle
 PWMError PWM_setDutyCycle(uint8_t c, uint8_t duty_cycle){
  if (c>=NUM_CHANNELS)
    return PWMChannelOutOfBound;
  return PWMSuccess;
}

