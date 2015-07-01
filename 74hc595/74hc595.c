/*
 *  write bytes to 74hc595 shift registers
 *  using spi or pure software (see 74hc595.h)
 *
 *  tested on atmega8
 *
 *  TODO support for reset line
 */

#ifndef _74HC595_C
#define _74HC595_C

#include "74hc595.h"


void shift_init()
{
    /* set pins as outputs */
    SHIFTREGISTER |= (1<<LATCH) | (1<<DATA) | (1<<CLOCK) | (1<<OUTEN);
    /* set pins to low */
    SHIFTPORT &= ~((1<<LATCH) | (1<<DATA) | (1<<CLOCK));
    /* output enable is 'active low' */
    SHIFTPORT |= (1<<OUTEN);

#if USE_SPI
    /* start spi as master (MSB first, no interrupt) */
    SPCR = (1<<SPE) | (1<<MSTR);
    /* max transmission speed (f_cpu/2 for one bit) */
    SPSR |= (1<<SPI2X);
#endif
}

inline void shift_enable()
{
    SHIFTPORT &= ~(1<<OUTEN);
}

inline void shift_disable()
{
    SHIFTPORT |= (1<<OUTEN);
}

/* shift out one character */
void shift_out(uint8_t c)
{
#if USE_SPI
    /* write spi data register */
    SPDR = c;

    /* wait until transmission is completed */
    while (!(SPSR & (1<<SPIF)))
        ;
#else
    uint8_t i;
    for (i=8; i; --i,c<<=1)  {
        if (c & 0x80)
            /* shift a one */
            SHIFTPORT |= (1<<DATA);
        else
            /* shift a zero */
            SHIFTPORT &= ~(1<<DATA);

        /* clock pulse */
        SHIFTPORT |= (1<<CLOCK);
        SHIFTPORT &= ~(1<<CLOCK);
    }
#endif

    /* latch pulse */
    SHIFTPORT |= (1<<LATCH);
    SHIFTPORT &= ~(1<<LATCH);
}

/* shift out byte array */
void shift_out_array(uint8_t *bytes, uint8_t length)
{
    uint8_t i;
    for (i=0; i<length; i++)    {
        shift_out(bytes[i]);
    }
}

#endif
