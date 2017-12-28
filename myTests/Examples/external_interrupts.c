/**********************************************************************
* © 2005 Microchip Technology Inc.
*
* FileName:        external_interrupts.c
* Dependencies:    Header (.h) files if applicable, see below
* Processor:       dsPIC30Fxxxx
* Compiler:        MPLAB® C30 v1.33.00 or higher
* IDE:             MPLAB® IDE v7.20.01 or later
* Dev. Board Used: dsPICDEM 1.1 Development Board
* Hardware Dependencies: None
*
* SOFTWARE LICENSE AGREEMENT:
* Microchip Technology Inc. (“Microchip”) licenses this software to you
* solely for use with Microchip dsPIC® digital signal controller
* products. The software is owned by Microchip and is protected under
* applicable copyright laws.  All rights reserved.
*
* SOFTWARE IS PROVIDED “AS IS.”  MICROCHIP EXPRESSLY DISCLAIMS ANY
* WARRANTY OF ANY KIND, WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE, OR NON-INFRINGEMENT. IN NO EVENT SHALL MICROCHIP
* BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR CONSEQUENTIAL
* DAMAGES, LOST PROFITS OR LOST DATA, HARM TO YOUR EQUIPMENT, COST OF
* PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
* BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF),
* ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER SIMILAR COSTS.
*
* REVISION HISTORY:
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Author            Date      Comments on this revision
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* HV               09/30/05  First release of source file
*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
* ADDITIONAL NOTES:
*
**********************************************************************/

#include "p30fxxxx.h"

/* Macros for Configuration Fuse Registers (copied from device header file):*/
_FOSC(CSW_FSCM_OFF & XT_PLL8);  /* Set up for Crystal multiplied by 8x PLL */
_FWDT(WDT_OFF);                 /* Turn off the Watch-Dog Timer.  */
_FBORPOR(MCLR_EN & PWRT_OFF);   /* Enable MCLR reset pin and turn off the power-up timers. */
_FGS(CODE_PROT_OFF);            /* Disable Code Protection */

/* Global Variables and Functions */
int main (void);
void INTx_IO_Init(void);
void __attribute__((__interrupt__)) _INT0Interrupt(void); /*Declare external interrupt ISRs*/
void __attribute__((__interrupt__)) _INT1Interrupt(void);
void __attribute__((__interrupt__)) _INT2Interrupt(void);
void __attribute__((__interrupt__)) _INT3Interrupt(void);
void __attribute__((__interrupt__)) _INT4Interrupt(void);

int main (void)
{
        ADPCFG = 0xFFFF;        /* Configure ANx pins used by ICD for digital i/o*/
        TRISD = 0x0000;         /* LEDs on dsPICDEM 1.1 board are connected to RD0-RD3 */
                                /* We will configure Port D to be output so we can use */
                                /* use LEDs as an indicator of the occurrence of external */
                                /* interrupts */

        INTx_IO_Init();         /* Call function to initialize the External Interrupts */
        while (1);              /* Loop endlessly...anytime an interrupt occurs */
                                /* the processor will vector to the interrupt and */
                                /* return back to the while(1) loop */
}
/*
Functions:
INTx_IO_Init() sets up the INT0, INT1, INT2, INT3 & INT4 pins.
INT1 - INT4 pins on the 30F6014A device are connected to
switches S1 - S4, on the dsPICDEM1.1 board.
*/
void INTx_IO_Init(void)
{
        INTCON2 = 0x001E;       /*Setup INT1, INT2, INT3 & INT4 pins to interupt */
                                /*on falling edge and set up INT0 pin to interupt */
                                /*on rising edge */
        IFS0bits.INT0IF = 0;    /*Reset INT0 interrupt flag */
        IEC0bits.INT0IE = 1;    /*Enable INT0 Interrupt Service Routine */

        IFS1bits.INT1IF = 0;    /*Reset INT1 interrupt flag */
        IEC1bits.INT1IE = 1;    /*Enable INT1 Interrupt Service Routine */

        IFS1bits.INT2IF = 0;    /*Reset INT0 interrupt flag */
        IEC1bits.INT2IE = 1;    /*Enable INT0 Interrupt Service Routine */

        IFS2bits.INT3IF = 0;    /*Reset INT1 interrupt flag */
        IEC2bits.INT3IE = 1;    /*Enable INT1 Interrupt Service Routine */

        IFS2bits.INT4IF = 0;    /*Reset INT1 interrupt flag */
        IEC2bits.INT4IE = 1;    /*Enable INT1 Interrupt Service Routine */
}

/*
_INT0Interrupt() is the INT0 interrupt service routine (ISR).
The routine must have global scope in order to be an ISR.
The ISR name is chosen from the device linker script.
*/
void __attribute__((__interrupt__)) _INT0Interrupt(void)
{

        IFS0bits.INT0IF = 0;    //Clear the INT0 interrupt flag or else
                                //the CPU will keep vectoring back to the ISR
}

/*
_INT1Interrupt() is the INT1 interrupt service routine (ISR).
The routine must have global scope in order to be an ISR.
The ISR name is chosen from the device linker script.
*/
void __attribute__((__interrupt__)) _INT1Interrupt(void)
{
        LATDbits.LATD0 = ~LATDbits.LATD0;
        IFS1bits.INT1IF = 0;    //Clear the INT1 interrupt flag or else
                                //the CPU will keep vectoring back to the ISR
}

/*
_INT2Interrupt() is the INT2 interrupt service routine (ISR).
The routine must have global scope in order to be an ISR.
The ISR name is chosen from the device linker script.
*/
void __attribute__((__interrupt__)) _INT2Interrupt(void)
{
        LATDbits.LATD1 = ~LATDbits.LATD1;
        IFS1bits.INT2IF = 0;    //Clear the INT2 interrupt flag or else
                                //the CPU will keep vectoring back to the ISR
}

/*
_INT3Interrupt() is the INT3 interrupt service routine (ISR).
The routine must have global scope in order to be an ISR.
The ISR name is chosen from the device linker script.
*/
void __attribute__((__interrupt__)) _INT3Interrupt(void)
{
        LATDbits.LATD2 = ~LATDbits.LATD2;
        IFS2bits.INT3IF = 0;    //Clear the INT3 interrupt flag or else
                                //the CPU will keep vectoring back to the ISR
}

/*
_INT4Interrupt() is the INT4 interrupt service routine (ISR).
The routine must have global scope in order to be an ISR.
The ISR name is chosen from the device linker script.
*/
void __attribute__((__interrupt__)) _INT4Interrupt(void)
{
        LATDbits.LATD3 = ~LATDbits.LATD3;
        IFS2bits.INT4IF = 0;    //Clear the INT4 interrupt flag or else
                                //the CPU will keep vectoring back to the ISR
}

/*
EOF
*/


