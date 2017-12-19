#include "pwm.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "pins.h"

// disabled PWM from all pins that use timer 1
// that is needed by the timer.c

// initializes the pwm subsystem
PWMError PWM_init(void){
  cli();

  // disable interrupts

  
  TIMSK0=0;
  TIMSK1=0;
  TIMSK2=0;

  //8 bit timers manual init
  TCCR0A=(1<<WGM00)|(1<<WGM01);
  TCCR0B=(1<<CS00);
  TCCR2A=TCCR0A;
  TCCR2B=TCCR0B;
   sei();

  return PWMSuccess;
}

// how many pwm on this chip?
uint8_t PWM_numChannels(void){
  return PINS_NUM;
}

// what was the period i set in the pwm subsystem
// might only require to adjust the prescaler
PWMError PWM_isEnabled(uint8_t c) {
  if (c>=PINS_NUM)
    return PWMChannelOutOfBound;
  const Pin* pin = pins+c;
  if (!pin->tcc_register)
    return PWMChannelOutOfBound;
  if ((*pin->tcc_register & pin->com_mask)==0)
    return 0;
  return PWMEnabled;
}

// sets the output on a pwm channel
PWMError PWM_enable(uint8_t c, uint8_t enable){
  if (c>=PINS_NUM)
    return PWMChannelOutOfBound;
  const Pin* pin = pins+c;
  if (!pin->tcc_register)
    return PWMChannelOutOfBound;
  *pin->oc_register=0;
  if (enable){
    *pin->tcc_register |= pin->com_mask;
    *pin->dir_register    |= (1<<pin->bit);
  } else {
    *pin->tcc_register &= ~pin->com_mask;
    *pin->dir_register    &= ~(1<<pin->bit);
  }
  return PWMSuccess;
}


// what was the duty cycle I last set?
uint8_t PWM_getDutyCycle(uint8_t c){
  if (c>=PINS_NUM)
    return PWMChannelOutOfBound;
  const Pin* pin = pins+c;
  if (!pin->tcc_register)
    return PWMChannelOutOfBound;
  return 255-*pin->oc_register;
}

// sets the duty cycle
 PWMError PWM_setDutyCycle(uint8_t c, uint8_t duty_cycle){
  if (c>=PINS_NUM)
    return PWMChannelOutOfBound;
  const Pin* pin = pins+c;
  if (!pin->tcc_register)
    return PWMChannelOutOfBound;
  *pin->oc_register = 255-duty_cycle;
  return PWMSuccess;
}

