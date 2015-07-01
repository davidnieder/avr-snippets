#ifndef _74HC595_H
#define _74HC595_H

#include <avr/io.h>

#ifndef USE_SPI
#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega32__)
    #define USE_SPI 1
#else
    #define USE_SPI 0
#endif
#endif

#define SHIFTREGISTER DDRB
#define SHIFTPORT PORTB

#define LATCH PB2   /* SS (RCK) (not necessarily on SS) */
#define DATA  PB3   /* MOSI (SI) */
#define CLOCK PB5   /* SCK (SCK) */
#define OUTEN PB0   /* OE - output enable (active low) */


void shift_init(void);
inline void shift_enable(void);
inline void shift_disable(void);
void shift_out(uint8_t);
void shift_out_array(uint8_t*, uint8_t);

#endif
