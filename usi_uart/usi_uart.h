/*
 * Implementation of an half duplex uart on the ATtiny25/45/85 using the
 * universal serial interface (usi) as outlined in Atmel app note AVR307.
 *
 *   http://ww1.microchip.com/downloads/en/Appnotes/doc4300.pdf
 *
 *
 * The frame format is fixed: eight data bits, one stop bit, no parity (8N1).
 * The baudrate can be configured below.
 *
 */
#ifndef USI_UART_H
#define USI_UART_H

#include <stdint.h>

/*
 * Possible baudrates depend on the system clock speed,
 * see table 1 on page 5 in app note avr307 for details.
 *
 * When using the 8MHz internal clock of the tiny25/45/85
 * possible values are 9600, 14400, 19200, 28800
 * (with TIMER_PRESCALER set to 8).
 *
 * When running at 1MHz (eg internal clock divided by 8)
 * use a baudrate of 9600 and set TIMER_PRESCALER to 1.
 *
 * F_CPU needs to be set accordingly.
 */
#define BAUDRATE 19200
#define TIMER_PRESCALER 8

/* If set to 1, the sendc/sends function will not return
 * until all data was written to the output buffer.
 */
#define BLOCKING_WRITE 1

/* buffer sizes must be a power of 2 */
#define USI_UART_RX_BUFFER_SIZE 16
#define USI_UART_TX_BUFFER_SIZE 16


/*
 * call once at start-up
 */
void usi_uart_init();

/*
 * Send the byte c.
 *
 * IF BLOCKING_WRITE is set, the funktion does not return
 * until there is space available in the output buffer.
 *
 * Returns zero on success, one on error (output buffer full).
 */
uint8_t usi_uart_sendc(char c);

/*
 * Send the string s
 * (the terminating null byte is not transmitted).
 *
 * If BLOCKING_WRITE is set, the funktion does not return
 * until the whole string was written to the output buffer.
 *
 * Returns the number bytes transmitted.
 */
uint8_t usi_uart_sends(const char *s);

/*
 * Returns the number of bytes available in the input buffer.
 */
uint8_t usi_uart_data_available();

/*
 * Move one byte from the input buffer to c.
 *
 * Returns zero on success, one on error (input buffer empty).
 */
uint8_t usi_uart_recvc(char *c);

/*
 * Move up to n bytes from the input buffer to 'buf'.
 *
 * Returns the number of bytes written to 'buf'.
 */
uint8_t usi_uart_recvn(char *buf, uint8_t n);

/*
 * Read from the input buffer and store into 'buf' until the character in
 * 'delimiter' is enountered or 'size' bytes were copied.
 *
 * Returns the number of bytes written to 'buf'.
 */
uint8_t usi_uart_recv_until(char *buf, uint8_t size, char delimiter);

/*
 * Discard everything currently in the reveive buffer.
 */
void usi_uart_rx_buffer_clear();

#endif
