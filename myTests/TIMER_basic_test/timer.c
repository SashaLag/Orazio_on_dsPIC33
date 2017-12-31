#include "p33FJ128MC802.h"
#include <string.h>

// here we hook to timer 3 which is of type C 
// its 16 bit and allows us a decent resolution

#include "timer.h"
// just 1 timer for the user
// we hook to timer 3
#define  NUM_TIMERS 1

// Configuration settings -- should go in main function??
_FOSC(CSW_FSCM_OFF & FRC_PLL16); // Fosc=16x7.5MHz, Fcy=30MHz
_FWDT(WDT_OFF);                  // Watchdog timer off
_FBORPOR(MCLR_DIS);              // Disable reset pin


typedef struct Timer{
  int timer_num;
  uint16_t duration_ms;
  TimerFn fn;
  void* args;
} Timer;

static Timer timers[NUM_TIMERS];

void Timers_init(void){
  memset(timers, 0, sizeof(timers));
  for (int i=0; i<NUM_TIMERS; ++i)
    timers[i].timer_num=i;
	 // Configure Timer 3.
    // PR3 and TCKPS are set to call interrupt every 500ms.
    // Period = PR3 * prescaler * Tcy = 58594 * 256 * 33.33ns = 500ms
    T3CON = 0;            // Clear Timer 3 configuration
    T3CONbits.TCKPS = 3;  // Set timer 3 prescaler (0=1:1, 1=1:8, 2=1:64, 3=1:256)
    PR3 = 58594;          // Set Timer 3 period (max value is 65535)
    IPC2bits.T3IP = 0x01;            // Set Timer 3 interrupt priority (1)
    IFS0bits.T3IF = 0; // Clear Timer 3 interrupt flag
	IEC0bits.T3IE = 1; // Enable Timer 3 interrupt
//    T3CONbits.TON = 1;    // Turn on Timer 3 -- should go in start function
}

// creates a timer that has a duration of ms milliseconds
// bound to the device device
// every duration_ms the function timer_fn will be called
// with arguments timer args
Timer* Timer_create(char* device,
		    uint16_t duration_ms,
		    TimerFn timer_fn,
		    void* timer_args){
  Timer* timer=0;
  if (!strcmp(device,"timer_0"))
    timer=timers;
  else
    return 0;
  timer->duration_ms=duration_ms;
  timer->timer_num=0;
  timer->fn=timer_fn;
  timer->args=timer_args;
  return timer;
}

// stops and destroys a timer
void Timer_destroy(struct Timer* timer){
  Timer_stop(timer);
  asm volatile ("disi #0x3FFF"); // ********** disable all user interrupts (atomically)
    int timer_num=timer->timer_num;
    memset(timer, 0, sizeof(Timer));
    timer->timer_num=timer_num;
  asm volatile ("disi #0x0"); // ************ enable all user interrupts (atomically)
}

void _timer0_start(struct Timer* timer){
  uint16_t ocrval=(uint16_t)(15.62*timer->duration_ms);
  //TCCR5A = 0;
  //TCCR5B = 0;
  //OCR5A = ocrval;
  //TCCR5B |= (1 << WGM52) | (1 << CS50) | (1 << CS52);
  //TIMSK5 |= (1 << OCIE5A);
}


// starts a timer
void Timer_start(struct Timer* timer){
  asm volatile ("disi #0x3FFF"); // ********** disable all user interrupts (atomically)
  if (timer->timer_num==0)
    _timer0_start(timer);
  asm volatile ("disi #0x0"); // ********** enable all user interrupts (atomically)
}


// stops a timer
void Timer_stop(struct Timer* timer){
  if (timer->timer_num!=0)
    return;
  asm volatile ("disi #0x3FFF"); // *********** disable all user interrupts (atomically)
  //TIMSK5 &= ~(1 << OCIE5A);
  asm volatile ("disi #0x0"); // ************** enable all user interrupts (atomically)
}


void __attribute__ ((interrupt, no_auto_psv)) _T3Interrupt() {
  IFS0bits.T3IF = 0; //clear Timer3 interrupt flag -- in atmega: TCNT5 = 0;
  if(timers[0].fn)
    (*timers[0].fn)(timers[0].args);
}