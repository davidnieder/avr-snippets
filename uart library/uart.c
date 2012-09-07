#include <avr/interrupt.h>

#include "uart.h"
#include <util/setbaud.h>



void uart_init()
{
    /* set baudrate */
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;

    /* double transmission speed? */
    #if USE_2X
    UCSRA |= (1<<U2X);
    #else
    UCSRA &= ~(1<<U2X);
    #endif

    /* frame format: asynchronous, 8 data bits, no parity, 1 stop bit */
    #ifdef URSEL
    UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0);
    #else
    UCSRC = (1<<UCSZ1) | (1<<UCSZ0);
    #endif

    /* enable receiver, transmitter and receive complete interrupt */
    UCSRB |= (1<<RXCIE) | (1<<RXEN) | (1<<TXEN);

    /* init ring buffer pointers */
    rxBuffer.start = rxBuffer.buffer;
    rxBuffer.end = rxBuffer.buffer;
    txBuffer.start = txBuffer.buffer;
    txBuffer.end = txBuffer.buffer;
}

/* sends single character
 * returns -1 on error
 */
uint8_t uart_putc(char c)
{
    /* is the buffer full? */
    if((txBuffer.end == &txBuffer.buffer[BUFFERSIZE-1] &&
        txBuffer.start == txBuffer.buffer) ||
        txBuffer.end == txBuffer.start-1) {

        return -1;
    }

    /* add element to buffer */
    *txBuffer.end = c;

    /* set pointer to next field */
    if(txBuffer.end == &txBuffer.buffer[BUFFERSIZE-1])
        txBuffer.end = txBuffer.buffer;
    else
        txBuffer.end++;

    /* enable tx register empty interrupt */
    UCSRB |= (1<<UDRIE);

    return 0;
}

/* sends null-terminated character string
 * returns number of sent characters
 */
uint8_t uart_puts(char *s)
{
    uint8_t count = 0;
    while(*s)   {

        if(uart_putc(*s) != -1) {
            count++;
            s++;
        }
        else
            break;
    }

    return count;
}

/* receives a character and stores it in 'dest'
 * returnes -1 if there is nothing in the buffer
 */
int8_t uart_getc(char *dest)
{
    /* is the buffer empty? */
    if(rxBuffer.start == rxBuffer.end)  {

        return -1;
    }

    /* get element from buffer */
    *dest = *rxBuffer.start;

    /* set pointer to next field */
    if(rxBuffer.start == &rxBuffer.buffer[BUFFERSIZE-1])
        rxBuffer.start = rxBuffer.buffer;
    else
        rxBuffer.start++;

    return 0;
}

/* uart receive complete interrupt */
ISR(RX_COMPL_INT)
{
    uint8_t rc;
    rc = UDR;

    /* store in buffer */
    if((rxBuffer.end == &rxBuffer.buffer[BUFFERSIZE-1] &&
        rxBuffer.start == rxBuffer.buffer) ||
        rxBuffer.end == rxBuffer.start-1)   {

        return;
    }
    else    {
        *rxBuffer.end = rc;

        if(rxBuffer.end == &rxBuffer.buffer[BUFFERSIZE-1])
            rxBuffer.end = rxBuffer.buffer;
        else
            rxBuffer.end++;
    }
}

/* uart transmit register empty interrupt */
ISR(TX_REG_EMPTY_INT)
{
    /* send byte */
    UDR = *txBuffer.start;

    /* set pointer to next field */
    if(txBuffer.start == &txBuffer.buffer[BUFFERSIZE-1])
        txBuffer.start = txBuffer.buffer;
    else
        txBuffer.start++;

    /* buffer empty? disable interrupt */
    if(txBuffer.start == txBuffer.end)
        UCSRB &= ~(1<<UDRIE);
}

