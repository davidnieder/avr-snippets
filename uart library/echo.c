/* echos every character using the uart library */

#define F_CPU 8000000
#include "uart.c"

#include <util/delay.h>


int main()
{
    uart_init();
    sei();

    while (1)   {

        uint8_t c;
        if(uart_getc(&c) == 0)
            uart_putc(c);
    }
}

