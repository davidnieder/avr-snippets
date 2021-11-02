#ifndef UART_H
#define UART_H

#ifndef F_CPU
#error "F_CPU is not defined"
#endif

#include <stdint.h>

#ifndef BAUD
#ifdef UART_BAUD_RATE
#define BAUD UART_BAUD_RATE
#else
#define BAUD 9600
#endif
#endif

#ifndef RX_BUFFERSIZE
#define RX_BUFFERSIZE 16
#endif

#ifndef TX_BUFFERSIZE
#define TX_BUFFERSIZE 32
#endif

/* if set to 1 sending will block execution until all data
 * is written to the output buffer */
#ifndef TX_BLOCK_ON_FULL_BUFFER
#define TX_BLOCK_ON_FULL_BUFFER 1
#endif

#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega32__)
    #define RX_COMPL_INT USART_RXC_vect
    #define TX_REG_EMPTY_INT USART_UDRE_vect

#elif defined(__AVR_ATtiny2313__) || defined(__AVR_ATmega8535__)
    #define RX_COMPL_INT USART_RX_vect
    #define TX_REG_EMPTY_INT USART_UDRE_vect

#endif


/*
 * initializes uart hardware and library - call once at start.
 * enable interrupts before sending data.
 */
void uart_init();

/*
 * sends a single character over the uart.
 * returns non-zero on error
 */
uint8_t uart_putc(char c);

/*
 * sends null-terminated character string
 * return number of sent characters
 */
uint8_t uart_puts(char *s);

/*
 * sends the 2-digit hex representation of byte
 * (eg (dec)42 becomes "2A")
 *
 * returns non-zero on error
 */
uint8_t uart_putb(char byte);

/*
 * reads one character from the uart and stores it in 'c'
 * returns non-zero if the input buffer is empty (nothing received)
 */
uint8_t uart_getc(char *c);

#endif
