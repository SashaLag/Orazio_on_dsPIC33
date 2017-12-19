#include "orazio_hbridge_internal.h"
#include "digio.h"
#include "pwm.h"
#include "orazio_packets.h"

typedef void (*HBridgeFiniFn)(HBridge* bridge);
typedef void (*HBridgeSetSpeedFn)(HBridge* bridge, int16_t speed);

typedef struct HBridgeOps{
  HBridgeFiniFn fini_fn;
  HBridgeSetSpeedFn setSpeed_fn;
}  HBridgeOps;



void HBridgePWMDir_fini(HBridge* bridge);
void HBridgePWMDir_setSpeed(HBridge* bridge, int16_t speed);

void HBridgeDualPWM_fini(HBridge* bridge);
void HBridgeDualPWM_setSpeed(HBridge* bridge, int16_t speed);

void HBridgeHalfPWM_fini(HBridge* bridge);
void HBridgeHalfPWM_setSpeed(HBridge* bridge, int16_t speed);


static HBridgeOps h_bridge_ops[]=
  {
    // PWM and dir
    {
      .fini_fn=HBridgePWMDir_fini,
      .setSpeed_fn=HBridgePWMDir_setSpeed
    },
    // dual pwm
    {
      .fini_fn=HBridgeDualPWM_fini,
      .setSpeed_fn=HBridgeDualPWM_setSpeed
    },
    // half pwm
    {
      .fini_fn=HBridgeHalfPWM_fini,
      .setSpeed_fn=HBridgeHalfPWM_setSpeed
    }
  };

/* PWM+Dir Mode */


void HBridgePWMDir_init(HBridge* bridge,
			uint8_t pwm_pin,
			uint8_t dir_pin){
  if (bridge->ops)
    (*bridge->ops->fini_fn)(bridge);

  bridge->ops=h_bridge_ops+HBridgeTypePWMDir;
  bridge->params.pwmdir.pwm_pin=pwm_pin;
  bridge->params.pwmdir.dir_pin=dir_pin;
  DigIO_setDirection(dir_pin,Output);
  DigIO_setValue(dir_pin,0);
  PWM_setDutyCycle(pwm_pin,0);
  PWM_enable(pwm_pin,1);
}

void HBridgePWMDir_fini(HBridge* bridge){
  PWM_enable(bridge->params.pwmdir.pwm_pin,0);
  PWM_setDutyCycle(bridge->params.pwmdir.pwm_pin,0);
  DigIO_setDirection(bridge->params.pwmdir.dir_pin,Input);
  DigIO_setValue(bridge->params.pwmdir.dir_pin,0);
}

void HBridgePWMDir_setSpeed(HBridge* bridge, int16_t speed){
  const uint8_t dir_pin = bridge->params.pwmdir.dir_pin;
  const uint8_t pwm_pin = bridge->params.pwmdir.pwm_pin;
  uint16_t pwm=0;
  uint8_t dir=0;
  if(speed>=0){
    pwm=speed;
    dir=0;
  } else {
    pwm=-speed;
    dir=1;
  }
  if (pwm>255)
    pwm=255;
  DigIO_setValue(dir_pin, dir);
  PWM_setDutyCycle(pwm_pin, pwm);
}

/* Dual PWM */
void HBridgeDualPWM_init(HBridge* bridge,
			 uint8_t pwm_forward_pin,
			 uint8_t pwm_backward_pin){
  if (bridge->ops)
    (*bridge->ops->fini_fn)(bridge);

  bridge->ops=h_bridge_ops+HBridgeTypeDualPWM;
  bridge->params.dualpwm.pwm_forward_pin=pwm_forward_pin;
  bridge->params.dualpwm.pwm_backward_pin=pwm_backward_pin;
  PWM_setDutyCycle(pwm_forward_pin,0);
  PWM_setDutyCycle(pwm_backward_pin,0);
  PWM_enable(pwm_forward_pin,1);
  PWM_enable(pwm_backward_pin,1);
}

void HBridgeDualPWM_fini(HBridge* bridge){
  PWM_enable(bridge->params.dualpwm.pwm_forward_pin,0);
  PWM_enable(bridge->params.dualpwm.pwm_backward_pin,0);
}

void HBridgeDualPWM_setSpeed(HBridge* bridge, int16_t speed){
  const uint8_t fpwm_pin = bridge->params.dualpwm.pwm_forward_pin;
  const uint8_t bpwm_pin = bridge->params.dualpwm.pwm_backward_pin;
  if (speed>255)
    speed=255;
  if (speed<-255)
    speed=-255;
  if(speed>=0){
    PWM_setDutyCycle(fpwm_pin, speed);
    PWM_setDutyCycle(bpwm_pin, 0);
  } else {
    PWM_setDutyCycle(fpwm_pin, 0);
    PWM_setDutyCycle(bpwm_pin, -speed);
  }
}

/* Half PWM */
void HBridgeHalfPWM_init(HBridge* bridge,
			 uint8_t pwm_pin,
			 uint8_t enable_pin){
  if (bridge->ops)
    (*bridge->ops->fini_fn)(bridge);

  bridge->ops=h_bridge_ops+HBridgeTypeHalfCyclePWM;
  bridge->params.halfpwm.pwm_pin=pwm_pin;
  bridge->params.halfpwm.enable_pin=enable_pin;
  PWM_setDutyCycle(pwm_pin,0);
  DigIO_setValue(enable_pin, 1);
  PWM_enable(pwm_pin,1);
  DigIO_setDirection(enable_pin,1);
}

void HBridgeHalfPWM_fini(HBridge* bridge){
  uint8_t pwm_pin=bridge->params.halfpwm.pwm_pin;
  uint8_t enable_pin=bridge->params.halfpwm.enable_pin;
  PWM_setDutyCycle(pwm_pin,0);
  DigIO_setValue(enable_pin, 0);
  PWM_enable(pwm_pin,0);
  DigIO_setDirection(enable_pin,0);
}

void HBridgeHalfPWM_setSpeed(HBridge* bridge, int16_t speed){
  uint8_t pwm_pin=bridge->params.halfpwm.pwm_pin;
  if (speed>255)
    speed=255;
  if (speed<-255)
    speed=-255;
  speed=127+speed/2;
  PWM_setDutyCycle(pwm_pin, speed);
}

int8_t HBridge_setSpeed(HBridge* bridge, int16_t speed) {
  (*bridge->ops->setSpeed_fn)(bridge, speed);
  return 0;
}
