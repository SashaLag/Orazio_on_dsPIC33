#include <assert.h>
#include <string.h>
#include <math.h>
#include "timer.h"
#include "eeprom.h"
#include "encoder.h"
#include "digio.h"
#include "pwm.h"

#include "orazio_globals.h"
#include "orazio_param.h"
#include "orazio_comm.h"
#include "orazio_joints.h"
#include "orazio_drive.h"
#include "orazio_hbridge.h"

// used to handle communication
volatile uint8_t tick_counter;           //incremented at each timer tick, reset on comm
volatile uint8_t comm_handle=0;

// this is to remember how many packets we received since the last update
volatile uint16_t rx_packets_before=0; 

void onTimerTick(void* args __attribute__((unused))){
  Orazio_jointsHandle();
  Orazio_hBridgeApplyControl();
  --tick_counter;
  if (!tick_counter){
    tick_counter=system_params.comm_cycles;
    comm_handle=1;
  }
}

int main(int argc, char** argv){
  
  // initialize devices
  EEPROM_init();
  DigIO_init();
  PWM_init();
  Encoder_init();
  Timers_init();
  
  // initialize subsystems
  Orazio_paramInit();
  Orazio_jointsInit();
  Orazio_hBridgeInit();
  Orazio_driveInit();
  Orazio_commInit();

  //start a timer
  struct Timer* timer=Timer_create("timer_0",system_params.timer_period_ms,onTimerTick,0);
  Timer_start(timer);

  // we want to get the first data whe the platform starts
  tick_counter=system_params.comm_cycles;
  system_status.watchdog_count=system_params.watchdog_cycles;

  // loop foreva
  while(1){
    ++system_status.idle_cycles; // count how long we spend doing nofin
    if (comm_handle){
      Orazio_driveUpdate();
      Orazio_driveControl();
      Orazio_commHandle();
      
      // decrease the watchdog if you did not get anything new
      // otherwise reset
      if(system_params.watchdog_cycles) {
	if (rx_packets_before==system_status.rx_packets)
	  --system_status.watchdog_count;
	else
	  system_status.watchdog_count=system_params.watchdog_cycles;
      
	// disable motors if did not receive stuff for long enough
	if (! system_status.watchdog_count && system_params.watchdog_cycles){
	  Orazio_jointsDisable();
	  Orazio_driveDisable();
	}
      }
      
      rx_packets_before=system_status.rx_packets;
      comm_handle=0;
      system_status.idle_cycles=0;
    }
  }
}
