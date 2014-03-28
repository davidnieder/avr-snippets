/* echos every character using the uart library */

#include <avr/interrupt.h>
#include "uart.h"


int main()
{
    char c;

    uart_init();
    sei();

    while(1)   {

        if(uart_getc(&c) == 0)
            uart_putc(c);
    }
}

