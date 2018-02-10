#include "pins.h"
//#include "digio.h"
#include <p33FJ128MC802.h>
/* I need to implement 14 RB pins.
 RB 7 and 8 are excluded (they are needed for uart)
 PWM on:
  PWM1L1 -> PR 15
  PWM1H1 -> PR 14
  PWM1L2 -> PR 13
  PWM1H2 -> PR 12
  PWM1L3 -> PR 11
  PWM1H3 -> PR 10

 to select TRISBbits.TRISB15 we will use:
 c = 2
 const Pin* pin = pins+c;
 *pin->dir_register |= (1<<pin->bit)

*/

const Pin pins[] =
  {
    //0
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=0,
      .tcc_register=0,
      .oc_register=0,
      .com_mask=0
    },
    //1
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=1,
      .tcc_register=0,
      .oc_register=0,
      .com_mask=0
    },
    //2 PWM1l1
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=15,
      .tcc_register=&TCCR3A,
      .oc_register=&P1DC1,     // here is stored the duty cycle
      .com_mask=(1<<COM3B0)|(1<<COM3B1)
    },
    //3 PWM1H1  (SAME dutycycle as PWM1L1)
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=14,
      .tcc_register=&TCCR3A,
      .oc_register=&P1DC1,
      .com_mask=(1<<COM3C0)|(1<<COM3C1)
    },
    //4 PWM1L2
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=13,
      .tcc_register=&TCCR0A,
      .oc_register=&P1DC2,
      .com_mask=(1<<COM0B0)|(1<<COM0B1)
    },
    //5 PWM1H2  (SAME dutycycle as PWM1L2)
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=12,
      .tcc_register=&TCCR3A,
      .oc_register=&P1DC2,
      .com_mask=(1<<COM3A0)|(1<<COM3A1)
    },
    //6 PWM1L3
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=11,
      .tcc_register=&TCCR4A,
      .oc_register=&P1DC3,
      .com_mask=(1<<COM4A0)|(1<<COM4A1)
    },
    //7 PWM1H3  (SAME dutycycle as PWM1L3)
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=10,
      .tcc_register=&TCCR4A,
      .oc_register=&P1DC3,
      .com_mask=(1<<COM3B0)|(1<<COM3B1)
    },
    //8 PWM (not in dspic)
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=9,
      .tcc_register=0,
      .oc_register=0,
      .com_mask=0
    },
    //9 PWM (not in dspic)
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=6,
      .tcc_register=0,
      .oc_register=0,
      .com_mask=0
    },
    //10 PWM (not in dspic)
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=5,
      .tcc_register=0,
      .oc_register=0,
      .com_mask=0
    },
    //11 PWM (not in dspic)
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=4,
      .tcc_register=0,
      .oc_register=0,
      .com_mask=0
    },
    //12 PWM (not in dspic)
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=3,
      .tcc_register=0,
      .oc_register=0,
      .com_mask=0
    },
    //13 PWM (not in dspic)
    {
      .in_register=&PORTB,
      .out_register=&LATB,
      .dir_register=&TRISB,
      .bit=2,
      .tcc_register=0,
      .oc_register=0,
      .com_mask=0
    }
  };
