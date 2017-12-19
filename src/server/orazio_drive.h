#pragma once

// initiaizes the drive controller
void Orazio_driveInit(void);

// updates odometry in the status packet
void Orazio_driveUpdate(void);

// flushes to the joints velocities computed to move the base
// with translational and rotational velocity
void Orazio_driveControl(void);

// disable the drive control of the joints and allows
// direct joint control
void Orazio_driveDisable(void);
