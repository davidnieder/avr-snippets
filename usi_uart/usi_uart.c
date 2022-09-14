#include <avr/io.h>
#include <avr/interrupt.h>

#include "usi_uart.h"


#ifndef F_CPU
 #error "F_CPU is not defined."
#endif

#define TIMER0_SEED (256 - ((F_CPU/BAUDRATE)/TIMER_PRESCALER))
#define TIMER0_INITIAL_SEED (256 - (((F_CPU)/BAUDRATE)/TIMER_PRESCALER) * 3/2)

#if TIMER0_SEED>240 || (TIMER0_SEED>180 && TIMER_PRESCALER==1)
 #warning "TIMER0_SEED has high value. Lower baudrate if communication is unstable."
#endif
#if TIMER0_INITIAL_SEED<5
 #warning "TIMER0_INITIAL_SEED has critical value. Check baudrate/prescaler settings."
#endif

#define USI_COUNTER_SEED_TX 11
#define USI_COUNTER_SEED_RX 8

#if TIMER_PRESCALER == 1
 #define TIMER0_CLOCK_SELECT (1<<CS00)
#elif TIMER_PRESCALER == 8
 #define TIMER0_CLOCK_SELECT (1<<CS01)
#else
 #error "Unsupported TIMER_PRESCALER value."
#endif


#define RX_BUFFER_MASK (USI_UART_RX_BUFFER_SIZE-1)
#define TX_BUFFER_MASK (USI_UART_TX_BUFFER_SIZE-1)

static unsigned char rx_buffer[USI_UART_RX_BUFFER_SIZE];
static volatile uint8_t rx_head;
static volatile uint8_t rx_tail;

static unsigned char tx_buffer[USI_UART_TX_BUFFER_SIZE];
static volatile uint8_t tx_head;
static volatile uint8_t tx_tail;

static volatile uint8_t tx_current_byte;

#define STATE_IDLE			0
#define STATE_RX_ACTIVE		1
#define STATE_RX_WAIT_NEXT	2
#define STATE_TX_ACTIVE		3
#define STATE_TX_MID_BYTE	4
static volatile uint8_t state;


static void
initialize_transmitter()
{
	cli();

	/* setup timer0 to generate an overflow interrupt every time
	 * the usi should shift out the next bit */
	TCNT0 = TIMER0_SEED;
	TCCR0B = TIMER0_CLOCK_SELECT;
	TIFR = (1<<TOV0);
	TIMSK |= (1<<TOIE0);

	/* enable usi overflow interrupt, three wire mode */
	USICR = (1<<USIOIE) | (1<<USIWM0);
	/* load high bits into the shift register */
	USIDR = 0xff;
	/* clear usi interrupt flags */
	USISR |= (1<<USISIF) | (1<<USIOIF) | (1<<USIPF);
	/* set usi counter to top value, overflow interrupt on next shift */
	USISR |= 0x0f;

	/* configure usi do as output */
	DDRB |= (1<<PB1);

	state = STATE_TX_ACTIVE;

	sei();
}

static void
initialize_receiver()
{
	cli();

	/* disconnect do from usi data register */
	DDRB &= ~(1<<PB1);

	/* disable usi module */
	USICR = 0;

	/* enable pin-change interrupt on pcint0 (usi di) */
	PCMSK |= (1<<PCINT0);
	/* enable pin-change interrupt */
	GIMSK |= (1<<PCIE);
	/* clear pin-change interrupt flag */
	GIFR = (1<<PCIF);

	state = STATE_IDLE;

	sei();
}

/* pin change interrupt - detect the falling edge of the start bit */
ISR(PCINT0_vect)
{

	if (PINB & (1<<PB0)) {
		/* di is high, this was the transition
		 * from last data bit to stop bit */
		return;
	}

	/* initial timer seed */
	TCNT0 = TIMER0_INITIAL_SEED + 2;
	/* start timer0 */
	TCCR0B = TIMER0_CLOCK_SELECT;
	/* clear overflow interrupt flag */
	TIFR = (1<<TOV0);
	/* enable overflow interrupt */
	TIMSK |= (1<<TOIE0);

	/* enable usi overflow interrupt, three wire mode, no clock source */
	USICR = (1<<USIOIE) | (1<<USIWM0);
	/* clear interrupt flags, set usi counter value */
	USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) | USI_COUNTER_SEED_RX;

	/* disable pin-change interrupt */
	GIMSK &= ~(1<<PCIE);

	state = STATE_RX_ACTIVE;
}

/* usi shift counter overflow interrupt */
ISR(USI_OVF_vect)
{
	uint8_t new_tail;

	switch (state) {
		case STATE_RX_ACTIVE:
			/* at this point the eight data bits of the uart frame
			 * should be in the usi data register */

			new_tail = (rx_tail+1) & RX_BUFFER_MASK;
			if (new_tail != rx_head) {
				/* store received byte */
				rx_buffer[rx_tail] = USIDR;
				rx_tail = new_tail;
			} else {
				/* rx buffer is full, discard for now */
			}

			/* get ready for next start condition */
			TCNT0 = 0;
			initialize_receiver();
			state = STATE_RX_WAIT_NEXT;
			break;

		case STATE_TX_ACTIVE:
			/* transmit the next byte from the tx buffer */
			if (tx_head != tx_tail) {
				tx_current_byte = tx_buffer[tx_head];
				tx_head = (tx_head+1) & TX_BUFFER_MASK;

				/* clear usi interrupt flags, set usi counter */
				USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) | USI_COUNTER_SEED_TX;
				/* load high bit, start bit, and payload */
				USIDR = 0x80 | (tx_current_byte >> 2);

				/* next interrupt continue with second "half" of the byte */
				state = STATE_TX_MID_BYTE;

			} else {
				/* tx buffer empty, disable timer0, get ready to receive */
				TCCR0B = 0;
				initialize_receiver();
			}
			break;

		case STATE_TX_MID_BYTE:
			/* clear interrupt flags, set usi counter */
			USISR = (1<<USISIF) | (1<<USIOIF) | (1<<USIPF) | USI_COUNTER_SEED_TX;;
			/* load rest of payload, high bits */
			USIDR = (tx_current_byte << 3) | 0x07;

			/* next interrupt continue with next byte from buffer */
			state = STATE_TX_ACTIVE;
			break;
	}
}

/* timer0 overflow interrupt - timing for the usi */
ISR(TIM0_OVF_vect)
{
	if (state == STATE_RX_WAIT_NEXT) {
		/* after receiving the last frame we waited some time
		 * to see if the opposite side wanted to send more but
		 * did not detect a new start condition */
		if (tx_head != tx_tail) {
			/* there is data in the tx buffer, start transmitting */
			initialize_transmitter();
		} else {
			/* nothing to do for now */
			TCCR0B = 0;
			state = STATE_IDLE;
		}
	} else {
		/* time until next interrupt */
		TCNT0 += TIMER0_SEED;
		/* usi clock strobe, shift data in/out */
		USICR |= (1<<USICLK);
	}
}

/* reverse the bit order in given byte, data is transmitted lsb first */
static uint8_t
reverse_byte(uint8_t byte)
{
	byte = ((byte >> 1) & 0x55) | ((byte << 1) & 0xaa);
	byte = ((byte >> 2) & 0x33) | ((byte << 2) & 0xcc);
	byte = ((byte >> 4) & 0x0f) | ((byte << 4) & 0xf0);
	return byte;
}

void
usi_uart_init()
{
	/* pull-up on usi di/do */
	PORTB |= (1<<PB1) | (1<<PB0);
	/* configure usi di/do as input */
	DDRB &= ~((1<<PB1) | (1<<PB0));

	initialize_receiver();
}

uint8_t
usi_uart_sendc(char c)
{
	uint8_t next_tail;

	next_tail = (tx_tail+1) & TX_BUFFER_MASK;
#if BLOCKING_WRITE
	while (next_tail == tx_head)
		; /* wait for space */
#else
	if (next_tail == tx_head) {
		/* return unsuccessfully */
		return 1;
	}
#endif

	/* write byte to tx buffer, lsb first */
	tx_buffer[tx_tail] = reverse_byte(c);
	tx_tail = next_tail;

	if (state == STATE_IDLE) {
		initialize_transmitter();
	}

	return 0;
}

uint8_t
usi_uart_sends(const char *s)
{
	uint8_t count = 0;

	while (*s && usi_uart_sendc(*s++) == 0) {
		count++;
	}

	return count;
}

uint8_t
usi_uart_data_available()
{
	uint8_t tail = rx_tail; /* read once */
	return tail >= rx_head ? tail-rx_head : USI_UART_RX_BUFFER_SIZE-rx_head+tail;
}

uint8_t
usi_uart_recvc(char *c)
{
	if (rx_head == rx_tail) {
		/* no data in rx buffer */
		return 1;
	}

	*c = reverse_byte(rx_buffer[rx_head]);
	rx_head = (rx_head+1) & RX_BUFFER_MASK;

	return 0;
}

uint8_t
usi_uart_recvn(char *buf, uint8_t n)
{
	uint8_t c = 0;

	while (rx_head != rx_tail && c++ < n) {
		*buf++ = reverse_byte(rx_buffer[rx_head]);
		rx_head = (rx_head+1) & RX_BUFFER_MASK;
	}

	return c;
}

uint8_t
usi_uart_recv_until(char *buf, uint8_t size, char delimiter)
{
	uint8_t c = 0;

	while (c++ < size) {
		while (rx_head == rx_tail) ;

		*buf = reverse_byte(rx_buffer[rx_head]);
		rx_head = (rx_head+1) & RX_BUFFER_MASK;

		if (*buf == delimiter) break;
		buf++;
	}

	return c;
}

void
usi_uart_rx_buffer_clear()
{
	rx_head = rx_tail = 0;
}
