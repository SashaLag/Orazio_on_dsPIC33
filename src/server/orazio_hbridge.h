#pragma once

// initializes the h bridge subsystem
// - it applyes system_params.h_bridge_type to all bridges
//   in the system and assigns the platform dependant PWM/Digital pins
//   depending to the platform
void Orazio_hBridgeInit(void);

// copies the joint_controllers[i]->output to the bridges
// so that the motor actually spins
void Orazio_hBridgeApplyControl(void);
