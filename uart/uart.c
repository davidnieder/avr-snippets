#include "uart.h"

#include <avr/interrupt.h>
#include <util/setbaud.h>


static uint8_t *volatile rxBufStart, *volatile rxBufEnd;
static uint8_t *volatile txBufStart, *volatile txBufEnd;
static uint8_t rxBuf[RX_BUFFERSIZE], txBuf[TX_BUFFERSIZE];

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
    rxBufStart = rxBuf;
    rxBufEnd = rxBuf;
    txBufStart = txBuf;
    txBufEnd = txBuf;
}

/* sends single character
 * returns 1 on error
 */
uint8_t uart_putc(char c)
{
    /* is the buffer full? */
#if TX_BLOCK_ON_FULL_BUFFER
	while((txBufEnd == &txBuf[TX_BUFFERSIZE-1] && txBufStart == txBuf) ||
        txBufEnd == txBufStart-1) {}
#else
    if((txBufEnd == &txBuf[TX_BUFFERSIZE-1] && txBufStart == txBuf) ||
        txBufEnd == txBufStart-1) {
        return 1;
    }
#endif

    /* add element to buffer */
    *txBufEnd = c;

    /* set pointer to next field */
    if(txBufEnd == &txBuf[TX_BUFFERSIZE-1])
        txBufEnd = txBuf;
    else
        txBufEnd++;

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
		if (uart_putc(*s) != 0)
			break;
		else {
			count++;
			s++;
		}
    }

    return count;
}

/* converts the given byte into a 2-digit hex representation
 * and sends the two characters
 */
uint8_t uart_putb(char byte)
{
	uint8_t hn, ln;

	hn = ((byte & 0xF0) >> 4);
	ln = (byte & 0x0F);
	hn = hn < 10 ? hn + '0' : hn + 'A'-10;
	ln = ln < 10 ? ln + '0' : ln + 'A'-10;

	if (uart_putc(hn) != 0)
		return 1;
	if (uart_putc(ln) != 0)
		return 1;

	return 0;
}

/* receives a character and stores it in 'dest'
 * returnes 1 if there is nothing in the buffer
 */
uint8_t uart_getc(char *dest)
{
    /* is the buffer empty? */
    if(rxBufStart == rxBufEnd)  {
        return 1;
    }

    /* get element from buffer */
    *dest = *rxBufStart;

    /* set pointer to next field */
    if(rxBufStart == &rxBuf[RX_BUFFERSIZE-1])
        rxBufStart = rxBuf;
    else
        rxBufStart++;

    return 0;
}

/* uart receive complete interrupt */
ISR(RX_COMPL_INT)
{
    uint8_t rc;
    rc = UDR;

    /* store in buffer */
    if((rxBufEnd == &rxBuf[RX_BUFFERSIZE-1] &&
        rxBufStart == rxBuf) ||
        rxBufEnd == rxBufStart-1)   {

        return;
    }
    else    {
        *rxBufEnd = rc;

        if(rxBufEnd == &rxBuf[RX_BUFFERSIZE-1])
            rxBufEnd = rxBuf;
        else
            rxBufEnd++;
    }
}

/* uart transmit register empty interrupt */
ISR(TX_REG_EMPTY_INT)
{
    /* send byte */
    UDR = *txBufStart;

    /* set pointer to next field */
    if(txBufStart == &txBuf[TX_BUFFERSIZE-1])
        txBufStart = txBuf;
    else
        txBufStart++;

    /* buffer empty? disable interrupt */
    if(txBufStart == txBufEnd)
        UCSRB &= ~(1<<UDRIE);
}
