#include "orazio_globals.h"

//these packets are global
//variables that contain the state of our system
//and of the parameters
//they are updated automatically by the communication routines
//the status packets are updated and sent by various subsystems

SystemParamPacket system_params = {
  {.type=SYSTEM_PARAM_PACKET_ID,
   .size=sizeof(SystemParamPacket),
   .seq=0
  },
  .protocol_version=ORAZIO_PROTOCOL_VERSION,
  .firmware_version=ORAZIO_FIRMWARE_VERSION,
  .timer_period_ms=10,
  .comm_speed=115200,
  .comm_cycles=2,
  .periodic_packet_mask=(PSystemStatusFlag|PJointStatusFlag|PDriveStatusFlag),
  .watchdog_cycles=0,
  .num_motors=NUM_MOTORS,
  .h_bridge_type=0
};

JointParamPacket joint_params = {
  {.type=JOINT_PARAM_PACKET_ID,
   .size=sizeof(JointParamPacket),
   .seq=0
  },
  {
    {
      .kp=255,
      .ki=32,
      .kd=0,
      .max_i=255,
      .min_pwm=0,
      .max_pwm=255,
      .max_speed=100,
      .slope=10
    },
    {
      .kp=255,
      .ki=32,
      .kd=0,
      .max_i=255,
      .min_pwm=0,
      .max_pwm=255,
      .max_speed=100,
      .slope=10
    }
  }
};

DifferentialDriveParamPacket drive_params={
  {.type=DIFFERENTIAL_DRIVE_PARAM_PACKET_ID,
   .size=sizeof(DifferentialDriveParamPacket),
   .seq=0
  },
  .kr=0.001,
  .kl=0.001,
  .baseline=0.4,
  .right_joint_index=0,
  .left_joint_index=1,
  .max_translational_velocity=2.,
  .max_translational_acceleration=2.,
  .max_translational_brake=4.0,
  .max_rotational_velocity=2.,
  .max_rotational_acceleration=20
};

SystemStatusPacket system_status = {
  {.type=SYSTEM_STATUS_PACKET_ID,
   .size=sizeof(SystemStatusPacket),
   .seq=0
  },
  .rx_seq=0,
  .rx_packet_queue=0,
  .idle_cycles=0
};

JointStatusPacket joint_status = {
  {.type=JOINT_STATUS_PACKET_ID,
   .size=sizeof(JointStatusPacket),
   .seq=0
  }
};

DifferentialDriveStatusPacket drive_status = {
  {.type=DIFFERENTIAL_DRIVE_STATUS_PACKET_ID,
   .size=sizeof(DifferentialDriveStatusPacket),
   .seq=0
  }
};

JointControlPacket joint_control = {
  {.type=JOINT_CONTROL_PACKET_ID,
   .size=sizeof(JointControlPacket),
   .seq=0
  },
  {
    {
      .mode=JointDisabled,
      .speed=0
    },
    {
      .mode=JointDisabled,
      .speed=0
    },
  }
};

DifferentialDriveControlPacket drive_control = {
  {.type=DIFFERENTIAL_DRIVE_CONTROL_PACKET_ID,
   .size=sizeof(DifferentialDriveControlPacket),
   .seq=0
  },
  .translational_velocity=0.,
  .rotational_velocity=0.
};


 
