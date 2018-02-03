#define FCY 16000000
#include "uart.h"
#include "buffer_utils.h"
#include "delay.h"
#include <p33FJ128MC802.h>
#include <string.h>
#include <libpic30.h>

void setBaud57600(void) {
  #define BAUD 57600
  #define FREQ_SCALE 4
  #ifndef FCY
    #define FCY 16000000
  #endif
      U1BRG = (FCY / (FREQ_SCALE * BAUD)) - 1;
  #undef FREQ_SCALE
  #undef BAUD
}

void setBaud115200(void) {
  #define BAUD 115200
  #define FREQ_SCALE 4
  #ifndef FCY
    #define FCY 16000000
  #endif
      U1BRG = (FCY / (FREQ_SCALE * BAUD)) - 1;
  #undef FREQ_SCALE
  #undef BAUD
}

#define UART_BUFFER_SIZE 256

typedef struct myUART {
  int tx_buffer[UART_BUFFER_SIZE];
  volatile uint8_t tx_start;
  volatile uint8_t tx_end;
  volatile uint8_t tx_size;

  int rx_buffer[UART_BUFFER_SIZE];
  volatile uint8_t rx_start;
  volatile uint8_t rx_end;
  volatile uint8_t rx_size;

  int baud;
  int uart_num; // hardware uart;
} myUART;

static myUART uart_0;

struct myUART* UART_init(const char* device __attribute__((unused)), uint32_t baud) {
  myUART* uart = &uart_0;
  uart->uart_num = 0;

  switch (baud) {
    case 57600:  setBaud57600();  break;
    case 115200: setBaud115200(); break;
    default: return 0;
  }

  uart->tx_start = 0;
  uart->tx_end = 0;
  uart->tx_size = 0;
  uart->rx_start = 0;
  uart->rx_end = 0;
  uart->rx_size = 0;

  //UART Control Register Configuration
  U1MODEbits.UARTEN = 1;  // enables RX and TX
  U1MODEbits.USIDL = 0;   // Continue in Idle
  U1MODEbits.IREN = 0;    // No IR translation
  U1MODEbits.RTSMD = 1;   // Simplex Mode
  U1MODEbits.UEN = 0;     // U1TX and U1RX enabled, CTS,RTS not -- required?
  U1MODEbits.WAKE = 0;    // No Wake up (since we don't sleep here) -- required?
  U1MODEbits.LPBACK = 0;	// No Loop Back
  U1MODEbits.ABAUD = 0;   // No Autobaud (would require sending '55')
  U1MODEbits.URXINV = 0;	// IdleState = 1  (for dsPIC)
  U1MODEbits.BRGH = 1;	  // 4 clocks per bit period
  U1MODEbits.PDSEL = 0;   // mode 01: 8-bit data, even parity
  U1MODEbits.STSEL = 0;   // 1 stop bit
  
  //UART Status & Control Register Configuration
  U1STAbits.UTXINV = 0;   // U1TX Idle state is '1'
  U1STAbits.UTXBRK = 0;	  // Sync Break TX Disabled
  U1STAbits.UTXBF = 0;    // TX Buffer not full, one+ more char can be written
  U1STAbits.TRMT = 0;     // TX Shift Register not full, TX in progress or queued
  U1STAbits.ADDEN = 0;    // 8-bit data, Address Detect Disabled
  U1STAbits.RIDLE = 0;    // RX is active
  U1STAbits.PERR = 0;     // Parity Error not detected
  U1STAbits.FERR = 0;     // Framing Error not detected
  U1STAbits.OERR = 0;     // RX buffer not overflowed
  
  //Interrupt Configuration
  IPC2bits.U1RXIP = 0x4;  // RX Mid Range Interrupt Priority level (0100 => 4), no urgent reason
  IPC3bits.U1TXIP = 0x4;  // TX Mid Range Interrupt Priority level (0100 => 4), no urgent reason
  U1STAbits.URXISEL1 = 0;	// Interrupt when any char is received and transferred from the U1RSR to the receive buffer.
  U1STAbits.URXISEL0 = 0;	// Second part of configuration
  U1STAbits.UTXISEL1 = 0;	// interrupt when char is transferred to the TSR
  U1STAbits.UTXISEL0 = 0;	// Second part of configuration
  IFS0bits.U1TXIF = 0;    // Clear the Transmit Interrupt Flag
  IFS0bits.U1RXIF = 0;    // Clear the Receive Interrupt Flag
  //Fire the engine
  U1STAbits.UTXEN = 1;    // TX Enabled, TX pin controlled by UART
  
  
  asm volatile ( "mov #OSCCONL, w1 \n"
                "mov #0x45, w2 \n"
                "mov #0x57, w3 \n"
                "mov.b w2, [w1] \n"
                "mov.b w3, [w1] \n"
                "bclr OSCCON, #6 "); // CONTROL REGISTER UNLOCK: enable writes
   
  //UART PINS
  RPINR18bits.U1RXR = 7; //INPUT  RPINR18 means U1RX -> MAPPED TO RP7 -> UART1 (RX)
  RPOR4bits.RP8R = 3; //OUTPUT RP2R means U1TX -> MAPPED TO RP8 -> UART1 (TX)
  
  asm volatile ( "mov #OSCCONL, w1 \n"
                "mov #0x45, w2 \n"
                "mov #0x57, w3 \n"
                "mov.b w2, [w1] \n"
                "mov.b w3, [w1] \n"
                "bset OSCCON, #6"); // CONTROL REGISTER LOCK: disable writes
  
  AD1PCFGL = 0xFFFF; //FONDAMENTALE PER LA UART, DEVO CONFIGURARE QUEI PIN COME DIGITALI, DI BASE SONO ANALOGICI
   
  return &uart_0;
}

// returns the free space in the buffer
int UART_rxbufferSize(struct myUART* uart __attribute__((unused))) {
  return UART_BUFFER_SIZE;
}

// returns the free occupied space in the buffer
int UART_txBufferSize(struct myUART* uart __attribute__((unused))) {
  return UART_BUFFER_SIZE;
}

// number of chars in rx buffer
int UART_rxBufferFull(myUART* uart) {
  return uart->rx_size;
}

// number of chars in rx buffer
int UART_txBufferFull(myUART* uart) {
  return uart->tx_size;
}

// number of free chars in tx buffer
int UART_txBufferFree(myUART* uart) {
  return UART_BUFFER_SIZE - uart->tx_size;
}

void UART_putChar(struct myUART* uart, uint8_t c) {
  // loops until there is some space in the buffer
  while (uart->tx_size >= UART_BUFFER_SIZE);
  //ATOMIC Execution of following Code
  asm volatile ("disi #0x3FFF"); // disable all user interrupts (atomically)
    uart->tx_buffer[uart->tx_end] = c;
    BUFFER_PUT(uart->tx, UART_BUFFER_SIZE);
  asm volatile ("disi #0"); //enable all user interrupts (atomically)
  IEC0bits.U1TXIE = 1;   // Enable Transmit Interrupt
  //delayMs(1); // more than enough
  __delay_us(100);
  }

uint8_t UART_getChar(struct myUART* uart) {
  while (uart->rx_size == 0);
  uint8_t c;
  //ATOMIC Execution of following Code
  asm volatile ("disi #0x3FFF"); // disable all user interrupts (atomically)
    c = uart->rx_buffer[uart->rx_start];
    BUFFER_GET(uart->rx, UART_BUFFER_SIZE);
  asm volatile ("disi #0"); //enable all user interrupts (atomically)
  return c;
}


void __attribute__ ((interrupt, no_auto_psv)) _U1RXInterrupt() {
  uint8_t c=U1RXREG;
  if (uart_0.rx_size<UART_BUFFER_SIZE) {
    uart_0.rx_buffer[uart_0.rx_end] = c;
    BUFFER_PUT(uart_0.rx, UART_BUFFER_SIZE);
  }
}

void __attribute__ ((interrupt, no_auto_psv)) _U1TXInterrupt(){
  //IFS0bits.U1TXIF = 0;    // Clear the Transmit Interrupt Flag
    if (! uart_0.tx_size) {
    IEC0bits.U1TXIE = 0;   // Disable Transmit Interrupts
  } else {
    U1TXREG = uart_0.tx_buffer[uart_0.tx_start];
    BUFFER_GET(uart_0.tx, UART_BUFFER_SIZE);
    LATB = ~LATB;  // just for debug
  }
}