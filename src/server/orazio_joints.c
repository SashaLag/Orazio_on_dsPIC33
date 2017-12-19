#include <string.h>
#include "orazio_globals.h"
#include "encoder.h"
#include "orazio_joints_internal.h"

void Orazio_jointsInit(void){
  for (int i=0; i<NUM_MOTORS; ++i){
    memset(joint_controllers+i, 0, sizeof(JointController));
    JointController* c=joint_controllers+i;
    c->params=joint_params.joints+i;
    c->status=joint_status.joints+i;
    c->control=joint_control.joints+i;
  }
}

void Orazio_jointsHandle(void){
  Encoder_sample();
  for (uint8_t i=0; i<NUM_MOTORS; ++i){
    JointController_handle(joint_controllers+i, i);
  }
}

void Orazio_jointsDisable(void){
  for (uint8_t i=0; i<NUM_MOTORS; ++i){
    joint_controllers[i].control->mode=JointDisabled;
    joint_controllers[i].output=0;
  }
}
