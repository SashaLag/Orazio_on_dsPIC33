#include "pwm.h"
#include <p33FJ128MC802.h>
#include <pwm12.h>
#define NUM_CHANNELS 14

// here we use timers 0-4, in 8 bit mode
// regardless if they are 16 bit


// initializes the pwm subsystem
PWMError PWM_init(void){
  asm volatile ("disi #0x3FFF"); // disable all user interrupts (atomically)
/* not here, it should go to enable function
  unsigned int sptime = 0x0;
  unsigned int config1 = PWM1_EN             //
                         & PWM1_IDLE_CON
                         & PWM1_OP_SCALE1
                         & PWM1_IPCLK_SCALE1
                         & PWM1_MOD_UPDN;

  unsigned int config2 = PWM1_MOD1_IND
                         & PWM1_PEN1L
                         & PWM1_PDIS1H
                         & PWM1_MOD2_IND
                         & PWM1_PEN2L
                         & PWM1_PDIS2H 
                         & PWM1_PDIS3H
                         & PWM1_PDIS3L;

  unsigned int config3 = PWM1_SEVOPS1
                         & PWM1_OSYNC_PWM
                         & PWM1_UEN;
 OpenMCPWM1(period, sptime, config1, config2, config3);
*/
  uint16_t period;
  P1TPERbits.PTPER = period; // PWM Time Base Period Value
  P1SECMPbits.SEVTCMP = 0; // Special Event Compare Value - default
  P1TCONbits.PTSIDL = 0; // PWM time base runs in CPU idle mode
  P1TCONbits.PTOPS = 0; // PWM time base output postscale 1:1
  P1TCONbits.PTCKPS = 0; // PWM time base input clock period is TCY (1:1 prescale)
  //P1TCONbits.PTMOD = 2; // PWM time base operates in Continuous Up/Down Count mode
  P1TCONbits.PTMOD = 0; // PWM time base operates in Free Running mode


/* #include ConfigIntMCPWM.c, maybe not needed
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
