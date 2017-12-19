#pragma once
#include <stdint.h>
#include "orazio_packets.h"

#ifdef __cplusplus
extern "C" {
#endif

  struct HBridgeOps;

  typedef struct HBridge{
    struct HBridgeOps* ops;
    union {
      struct {
	uint8_t pwm_pin;
	uint8_t dir_pin;
      } pwmdir;
      struct {
	uint8_t pwm_forward_pin;
	uint8_t pwm_backward_pin;
      } dualpwm;
      struct {
	uint8_t pwm_pin;
	uint8_t enable_pin;
      } halfpwm;
    } params;
  } HBridge;


  int8_t HBridge_setSpeed(struct HBridge* bridge, int16_t speed);

  void HBridgePWMDir_init(HBridge* bridge,
			  uint8_t pwm_pin,
			  uint8_t dir_pin);

  void HBridgeDualPWM_init(HBridge* bridge,
			   uint8_t pwm_forward_pin,
			   uint8_t pwm_backward_pin);

  void HBridgeHalfPWM_init(HBridge* bridge,
			   uint8_t pwm_pin,
			   uint8_t enable_pin);

#ifdef __cplusplus
    }
#endif
