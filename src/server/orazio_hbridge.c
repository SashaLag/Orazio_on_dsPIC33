#include "orazio_hbridge.h"
#include "orazio_joints_internal.h"
#include "orazio_globals.h"
#include "orazio_hbridge_internal.h"

static HBridge bridges[NUM_MOTORS]={
  {
    .ops=0
  },
  {
    .ops=0
  }
};

//the more I look at the following the more i think it sucks
//portability 0% 
//for now I had to hardcode the pin numbers for typical configurations here
//in an ideal world we should parameterize that and allow to connect pins after flashing uc
//will be blasted, just thinkin how to make it elegant

// SUCKS?
//vvvvvvvvvvvvvvvvvv

void Orazio_hBridgeInit() {
  switch(system_params.h_bridge_type){
  case HBridgeTypePWMDir:
    HBridgePWMDir_init(bridges, 3, 12);               //< sucks
    HBridgePWMDir_init(bridges+1, 11, 13);            //< sucks 
    return;
  case HBridgeTypeDualPWM:                             
    HBridgeDualPWM_init(bridges, 3, 2);               //< sucks
    HBridgeDualPWM_init(bridges+1, 5, 4);             //< sucks
    return;
  case HBridgeTypeHalfCyclePWM:
    HBridgeHalfPWM_init(bridges, 2, 3);               //< sucks
    HBridgeHalfPWM_init(bridges+1, 4, 5);             //< sucks
    return;
  default:
    return;
  }
}

// ^^^^^^^^^^^^^^
// SUCKS

void Orazio_hBridgeApplyControl(){
  for (int i=0; i<NUM_MOTORS; ++i){
    HBridge* bridge=bridges+i;
    JointController* controller=joint_controllers+i;
    HBridge_setSpeed(bridge,controller->output);
  }
}
