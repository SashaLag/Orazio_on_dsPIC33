#include "orazio_param.h"
#include "eeprom.h"
#include "delay.h"
#include "orazio_drive.h"
#include "orazio_comm.h"
#include "orazio_joints.h"
#include "orazio_hbridge.h"


// these are the physical location
// in the eeprom where we store the parameters

#define PROTOCOL_VERSION_OFFSET (32)
#define FIRMWARE_VERSION_OFFSET (PROTOCOL_VERSION_OFFSET+sizeof(uint32_t))
#define SYSTEM_PARAM_OFFSET     (FIRMWARE_VERSION_OFFSET+sizeof(uint32_t))
#define JOINT_PARAM_OFFSET      (SYSTEM_PARAM_OFFSET+sizeof(SystemParamPacket))
#define DRIVE_PARAM_OFFSET      (JOINT_PARAM_OFFSET+sizeof(JointParamPacket))

  
// read from eeprom the firmware revision

// if they don't match with the current firmware
//    fill the parameters with default values
//    and save them to the eeprom
// otherwise  
//    just read
//    read all parameters

void Orazio_paramInit(void) {
  // read from the first two dwords of eeprom the protocol revision
  uint32_t cpv;
  uint32_t cfv;
  EEPROM_read(&cpv, PROTOCOL_VERSION_OFFSET, sizeof(uint32_t));
  EEPROM_read(&cfv, FIRMWARE_VERSION_OFFSET, sizeof(uint32_t));
  
  if (cpv!=system_params.protocol_version){
    EEPROM_write(PROTOCOL_VERSION_OFFSET, &system_params.protocol_version, sizeof(uint32_t));
    EEPROM_write(FIRMWARE_VERSION_OFFSET, &system_params.firmware_version, sizeof(uint32_t));
    Orazio_paramSave(ParamSystem);
    Orazio_paramSave(ParamJoints);
    Orazio_paramSave(ParamDrive);
  } else {
    Orazio_paramLoad(ParamSystem);
    Orazio_paramLoad(ParamJoints);
    Orazio_paramLoad(ParamDrive);
  }
  // for debug
  system_params.protocol_version=cpv;
  system_params.firmware_version=cfv;
  return;
}

PacketStatus Orazio_paramLoad(uint8_t param_type){
  switch(param_type){
  case ParamSystem:
    EEPROM_read(&system_params, SYSTEM_PARAM_OFFSET, sizeof(system_params));
    break;
  case ParamJoints:
    EEPROM_read(&joint_params, JOINT_PARAM_OFFSET, sizeof(joint_params));
    break;
  case ParamDrive:
    EEPROM_read(&drive_params, DRIVE_PARAM_OFFSET, sizeof(drive_params));
    break;
  default:
    return GenericError;
  }
  Orazio_paramSet(param_type);
  return Success;
}

PacketStatus Orazio_paramSave(uint8_t param_type){
  switch(param_type){
  case ParamSystem:
    EEPROM_write(SYSTEM_PARAM_OFFSET, &system_params, sizeof(system_params));
    return Success;
  case ParamJoints:
    EEPROM_write(JOINT_PARAM_OFFSET, &joint_params, sizeof(joint_params));
    return Success;
  case ParamDrive:
    EEPROM_write(DRIVE_PARAM_OFFSET, &drive_params, sizeof(drive_params));
    return Success;
  default:
    return GenericError;
  }
}

PacketStatus Orazio_paramSet(uint8_t param_type){
  switch(param_type){
  case ParamSystem:
    //Orazio_commInit();
    Orazio_hBridgeInit();
    Orazio_driveInit();
    return Success;
  case ParamJoints:
    //Orazio_jointsInit();
    return Success;
  case ParamDrive:
    Orazio_driveInit();
    return Success;
  default:
    return GenericError;
  }
}

