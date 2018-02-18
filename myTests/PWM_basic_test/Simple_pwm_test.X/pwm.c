#include "pwm.h"
#include <p33FJ128MC802.h>
#include "pins.h"
#define NUM_CHANNELS 14

// here we use timers 0-4, in 8 bit mode
// regardless if they are 16 bit


// initializes the pwm subsystem
PWMError PWM_init(void){
  asm volatile ("disi #0x3FFF"); // disable all user interrupts (atomically)
  // for updown count mode: P1TPER = FCY/(Fpwm*P1TMRprescaler*2)-1
  // FCY = 16000000; Fpwm = 20000; P1TMR = 1 => P1TPER = 499
  uint16_t period = 499;
  P1TPERbits.PTPER = period; // PWM Time Base Period Value
  P1TMR = 0; // start counting from 0
  P1SECMPbits.SEVTCMP = 0; // Special Event Compare Value - default
  P1TCONbits.PTSIDL = 0; // PWM time base runs in CPU idle mode
  P1TCONbits.PTOPS = 0; // PWM time base output postscale 1:1
  P1TCONbits.PTCKPS = 0; // PWM time base input clock period is TCY (1:1 prescale)
  P1TCONbits.PTMOD = 2; // PWM time base operates in Continuous Up/Down Count mode
  //P1TCONbits.PTMOD = 0; // PWM time base operates in Free Running mode
  PWM1CON1bits.PMOD1 = 1; //PWM1 in indipendent mode
  PWM1CON1bits.PMOD2 = 1; //PWM2 in indipendent mode
  PWM1CON1bits.PMOD3 = 1; //PWM3 in indipendent mode
  P1TCONbits.PTEN = 1; //PWM1 Time Base Timer Enable bit
  PWM1CON2bits.IUE = 1; // immediate update of PWM enabled
  P1OVDCON = 0x3F00; // no override. Override disables PWM
/*
  uint16_t config = PWM_INT_EN
                    & PWM_INT_PR4
                    & PWM_FLTA_DIS_INT
                    &PWM_FLTB_DIS_INT
  ConfigIntMCPWM1(config);
*/
  asm volatile ("disi #0"); //enable all user interrupts (atomically)
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
