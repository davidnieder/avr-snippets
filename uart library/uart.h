#ifndef UART_H
#define UART_H

#ifndef F_CPU
#error "F_CPU is not defined"
#endif

#include <stdint.h>

#ifndef BAUD
#define BAUD 9600
#endif

#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega32__)
    #define RX_COMPL_INT USART_RXC_vect
    #define TX_REG_EMPTY_INT USART_UDRE_vect
    
#elif defined(__AVR_ATtiny2313__) || defined(__AVR_ATmega8535__)
    #define RX_COMPL_INT USART_RX_vect
    #define TX_REG_EMPTY_INT USART_UDRE_vect

#endif


#define BUFFERSIZE 32

struct ringBuffer   {
    uint8_t *start;
    uint8_t *end;
    uint8_t buffer[BUFFERSIZE];

} rxBuffer, txBuffer;


/* prototypes */
void uart_init();
uint8_t uart_putc(char);
uint8_t uart_puts(char*);
int8_t uart_getc(char*);


#endif
