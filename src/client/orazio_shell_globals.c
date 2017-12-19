#include <string.h>
#include "orazio_shell_globals.h"
#include "orazio_client.h"

struct OrazioClient* client=0;
int run=1;
PacketStatus op_result=Success;

sem_t state_sem;
int state_sem_init=0;

sem_t param_sem;
int param_sem_init=0;


SystemStatusPacket system_status;
SystemParamPacket system_params;
JointStatusPacket joint_status;
JointParamPacket joint_params;
DifferentialDriveStatusPacket drive_status;
DifferentialDriveParamPacket drive_params;
EndEpochPacket end_epoch;
ResponsePacket response;

ParamControlPacket param_control={
  {.type=PARAM_CONTROL_PACKET_ID,
   .size=sizeof(ParamControlPacket),
   .seq=0
  },
  .action=ParamRequest,
  .param_type=ParamSystem
};

JointControlPacket joint_control={
  {.type=JOINT_CONTROL_PACKET_ID,
   .size=sizeof(JointControlPacket),
   .seq=0
  },
  {
    {0,0},
    {0,0}
  }
};
DifferentialDriveControlPacket drive_control={
  {.type=DIFFERENTIAL_DRIVE_CONTROL_PACKET_ID,
   .size=sizeof(DifferentialDriveControlPacket),
   .seq=0
  },
  .translational_velocity=0.f,
  .rotational_velocity= 0.f
};

VEntry variable_entries[]={
  // system status
  RV(system_status, TypePacket, VarRead),
  RV(system_status.header.seq, TypeUInt16, VarRead),
  RV(system_status.rx_buffer_size, TypeUInt16, VarRead),
  RV(system_status.rx_packets, TypeUInt16, VarRead),
  RV(system_status.rx_packet_errors, TypeUInt16, VarRead),
  RV(system_status.tx_buffer_size, TypeUInt16, VarRead),
  RV(system_status.tx_packets, TypeUInt16, VarRead),
  RV(system_status.tx_packet_errors, TypeUInt16, VarRead),
  RV(system_status.battery_level, TypeUInt16, VarRead),
  RV(system_status.watchdog_count, TypeUInt16, VarRead),
  RV(system_status.idle_cycles, TypeUInt32, VarRead),
  RV(system_status.rx_seq, TypeUInt16, VarRead),
  //system params
  RV(system_params, TypePacket, VarRead),
  RV(system_params.header.seq, TypeUInt16, VarRead),
  RV(system_params.protocol_version, TypeInt32Hex,VarRead),
  RV(system_params.firmware_version, TypeInt32Hex,VarRead),
  RV(system_params.timer_period_ms, TypeUInt16,VarRead|VarWrite),
  RV(system_params.comm_speed, TypeInt32,VarRead),
  RV(system_params.comm_cycles, TypeUInt16,VarRead|VarWrite),
  RV(system_params.periodic_packet_mask, TypeUInt8,VarRead|VarWrite),
  RV(system_params.watchdog_cycles, TypeUInt16,VarRead|VarWrite),
  RV(system_params.num_motors, TypeUInt8,VarRead),
  RV(system_params.h_bridge_type, TypeUInt8,VarRead|VarWrite),
  // joint status
  RV(joint_status, TypePacket, VarRead),
  RV(joint_status.header.seq, TypeUInt16, VarRead),
  RV(joint_status.joints[0].mode, TypeInt8, VarRead),
  RV(joint_status.joints[0].encoder_position, TypeUInt16, VarRead),
  RV(joint_status.joints[0].encoder_speed, TypeInt16, VarRead),
  RV(joint_status.joints[0].desired_speed, TypeInt16, VarRead),
  RV(joint_status.joints[0].pwm, TypeInt16, VarRead),
  RV(joint_status.joints[0].sensed_current, TypeInt16, VarRead),
  RV(joint_status.joints[1].mode, TypeInt8, VarRead),
  RV(joint_status.joints[1].encoder_position, TypeUInt16, VarRead),
  RV(joint_status.joints[1].encoder_speed, TypeInt16, VarRead),
  RV(joint_status.joints[1].desired_speed, TypeInt16, VarRead),
  RV(joint_status.joints[1].pwm, TypeInt16, VarRead),
  RV(joint_status.joints[1].sensed_current, TypeInt16, VarRead),
  //joint params
  RV(joint_params, TypePacket, VarRead),
  RV(joint_params.header.seq, TypeUInt16, VarRead),
  RV(joint_params.joints[0].kp, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[0].ki, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[0].kd, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[0].max_i, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[0].max_pwm, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[0].min_pwm, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[0].max_speed, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[0].slope, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[1].kp, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[1].ki, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[1].kd, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[1].max_i, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[1].max_pwm, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[1].min_pwm, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[1].max_speed, TypeInt16,VarRead|VarWrite),
  RV(joint_params.joints[1].slope, TypeInt16,VarRead|VarWrite),
  // drive status
  RV(drive_status, TypePacket, VarRead),
  RV(drive_status.header.seq, TypeUInt16, VarRead),
  RV(drive_status.odom_x, TypeFloat, VarRead),
  RV(drive_status.odom_y, TypeFloat, VarRead),
  RV(drive_status.odom_theta, TypeFloat, VarRead),
  RV(drive_status.translational_velocity_measured, TypeFloat, VarRead),
  RV(drive_status.translational_velocity_desired, TypeFloat, VarRead),
  RV(drive_status.translational_velocity_adjusted, TypeFloat, VarRead),
  RV(drive_status.rotational_velocity_measured, TypeFloat, VarRead),
  RV(drive_status.rotational_velocity_desired, TypeFloat, VarRead),
  RV(drive_status.rotational_velocity_adjusted, TypeFloat, VarRead),
  //drive params
  RV(drive_params, TypePacket, VarRead),
  RV(drive_params.header.seq, TypeUInt16, VarRead),
  RV(drive_params.kr, TypeFloat,VarRead|VarWrite),
  RV(drive_params.kl, TypeFloat,VarRead|VarWrite),
  RV(drive_params.baseline, TypeFloat,VarRead|VarWrite),
  RV(drive_params.right_joint_index, TypeUInt8,VarRead|VarWrite),
  RV(drive_params.left_joint_index,  TypeUInt8,VarRead|VarWrite),
  RV(drive_params.max_translational_velocity, TypeFloat,VarRead|VarWrite),
  RV(drive_params.max_translational_acceleration, TypeFloat,VarRead|VarWrite),
  RV(drive_params.max_translational_brake, TypeFloat,VarRead|VarWrite),
  RV(drive_params.max_rotational_velocity, TypeFloat,VarRead|VarWrite),
  RV(drive_params.max_rotational_acceleration, TypeFloat,VarRead|VarWrite),

  //param control
  RV(param_control,TypePacket, VarRead),
  RV(param_control.header.seq, TypeInt16, VarRead),
  RV(param_control.action, TypeUInt8,VarRead|VarWrite),
  RV(param_control.param_type, TypeUInt8,VarRead|VarWrite),

  //joint control
  RV(joint_control,TypePacket, VarRead),
  RV(joint_control.header.seq, TypeInt16, VarRead),
  RV(joint_control.joints[0].mode, TypeUInt8,VarRead|VarWrite),
  RV(joint_control.joints[0].speed, TypeInt16,VarRead|VarWrite),
  RV(joint_control.joints[1].mode, TypeUInt8,VarRead|VarWrite),
  RV(joint_control.joints[1].speed, TypeInt16,VarRead|VarWrite),

  //drive control
  RV(drive_control,TypePacket, VarRead),
  RV(drive_control.header.seq, TypeInt16, VarRead),
  RV(drive_control.translational_velocity, TypeFloat,VarRead|VarWrite),
  RV(drive_control.rotational_velocity, TypeFloat,VarRead|VarWrite)
};

const int num_variables=sizeof(variable_entries)/sizeof(VEntry);

const char *param_names[]={
  "system_params",
  "joint_params",
  "drive_params",
  0
};

const char *control_names[]={
  "param_control",
  "joint_control",
  "drive_control",
  0
};

int stringInSet(const char** set, const char* s){
  const char**base=set;
  while(*base){
    if (!strcmp(*base, s))
      return 1;
    ++base;
  }
  return 0;
}

int isParamPacket(const char* name){
  return stringInSet(param_names, name);
}

int isControlPacket(const char* name){
  return stringInSet(control_names, name);
}


VEntry* findVar(const char* name){
  int var_idx=0;
  while(var_idx<num_variables){
    if (! strcmp(variable_entries[var_idx].name, name))
      return &variable_entries[var_idx];
    var_idx++;
  }
  return 0;
}

int getVarsByPrefix(VEntry** entries, const char* prefix){
  int k=0;
  int l=strlen(prefix);
  for (int i=0; i<num_variables; ++i){
    VEntry* var=variable_entries+i;
    if (! strncmp(prefix, var->name, l)){
      entries[k++]=var;
    }
  }
  entries[k]=0;
  return k;
}

PacketStatus refreshState(void){
  op_result = OrazioClient_get(client, (PacketHeader*)&system_status, SYSTEM_STATUS_PACKET_ID);
  if (op_result!=Success) return op_result;

  op_result=OrazioClient_get(client, (PacketHeader*)&drive_status, DIFFERENTIAL_DRIVE_STATUS_PACKET_ID);
  if (op_result!=Success) return op_result;

  op_result=OrazioClient_get(client, (PacketHeader*)&joint_status, JOINT_STATUS_PACKET_ID);
  if (op_result!=Success) return op_result;

  op_result=OrazioClient_get(client, (PacketHeader*)&end_epoch, END_EPOCH_PACKET_ID);
  OrazioClient_get(client, (PacketHeader*)&response, RESPONSE_PACKET_ID);
  
  return op_result;
}

PacketStatus refreshParams(ParamType type){
  switch(type){
  case ParamSystem:
    return OrazioClient_get(client, (PacketHeader*)&system_params, SYSTEM_PARAM_PACKET_ID);
  case ParamJoints:
    return OrazioClient_get(client, (PacketHeader*)&joint_params, JOINT_PARAM_PACKET_ID);
  case ParamDrive:
    return OrazioClient_get(client, (PacketHeader*)&drive_params, DIFFERENTIAL_DRIVE_PARAM_PACKET_ID);
  default:
    return GenericError;
  }
}

sem_t state_sem;
