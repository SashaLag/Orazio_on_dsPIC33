#pragma once
#include "ventry.h"
#include "orazio_packets.h"
#include "packet_operations.h"
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

  // the client
  extern struct OrazioClient* client;

  //semaphore_update
  extern sem_t state_sem;
  extern int state_sem_init;

  extern sem_t param_sem;
  extern int param_sem_init;

  // is the system running
  extern int run;

  // result of the last packet operation
  extern PacketStatus op_result;

  // variables used by the shell thread to read values
  extern SystemStatusPacket system_status;
  extern SystemParamPacket system_params;
  extern JointStatusPacket joint_status;
  extern JointParamPacket joint_params;
  extern DifferentialDriveStatusPacket drive_status;
  extern DifferentialDriveParamPacket drive_params;
  extern JointControlPacket joint_control;
  extern ParamControlPacket param_control;
  extern DifferentialDriveControlPacket drive_control;
  extern ResponsePacket response;
  extern EndEpochPacket end_epoch;

  // variables in the system
  extern VEntry variable_entries[];
  extern const int num_variables;

  int isParamPacket(const char* name);

  int isControlPacket(const char* name);

  // find a variable by name
  VEntry* findVar(const char* name);

  // find all variables whose name start with prefix
  int getVarsByPrefix(VEntry** entries, const char* prefix);

  // refreshes all state packets
  PacketStatus refreshState(void);

  // refreshes all param packets
  PacketStatus refreshParams(ParamType type);

  

#ifdef __cplusplus
}
#endif
