#include "orazio_globals.h"
#include "orazio_drive_internal.h"

static DifferentialDriveController drive_controller;
volatile static uint16_t last_drive_control_seq=65535; // last param seq receiveds
static uint8_t drive_enable=0;

void Orazio_driveInit(void){
  uint8_t left_idx=drive_params.left_joint_index;
  uint8_t right_idx=drive_params.right_joint_index;
  JointInfo* status_left=joint_status.joints+left_idx;
  JointInfo* status_right=joint_status.joints+right_idx;
  JointControl* control_left=joint_control.joints+left_idx;
  JointControl* control_right=joint_control.joints+right_idx;
  DifferentialDriveController_init(&drive_controller,
				   &drive_params,
				   &drive_control,
				   &drive_status,
				   status_left,
				   status_right,
				   control_left,
				   control_right,
				   0.001*system_params.timer_period_ms*system_params.comm_cycles,
				   0.001*system_params.timer_period_ms);
  last_drive_control_seq=drive_control.header.seq;
}

void Orazio_driveUpdate(void){
  DifferentialDriveController_update(&drive_controller);
}

void Orazio_driveDisable(void){
  drive_enable=0;
}

void Orazio_driveControl(void){
  if(last_drive_control_seq!=drive_control.header.seq) {
    drive_enable=1;
    last_drive_control_seq=drive_control.header.seq;
  }
  if (drive_enable)
    DifferentialDriveController_control(&drive_controller);
  
}
