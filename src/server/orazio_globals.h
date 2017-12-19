#pragma once
#include "uart.h"
#include "deferred_packet_handler.h"
#include "orazio_packets.h"
#define ORAZIO_FIRMWARE_VERSION 0x20171125

//these global variables store the configuration
//the state and the control of each subsystem

extern  SystemParamPacket system_params;
extern  JointParamPacket joint_params;
extern  DifferentialDriveParamPacket drive_params;

extern  SystemStatusPacket system_status;
extern  JointStatusPacket joint_status;
extern  DifferentialDriveStatusPacket drive_status;

extern  JointControlPacket joint_control;
extern  DifferentialDriveControlPacket drive_control;


