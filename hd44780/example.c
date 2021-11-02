#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"


int main()
{
	uint8_t i = 0;
	char strbuf[16], tmp[16];

	lcd_init();
	lcd_home();

	lcd_puts("Hello LCD!");
	_delay_ms(2000);


	while (1) {
		lcd_clear();

		itoa(i, strbuf, 10);
		strcat(strbuf, " to ");
		itoa(i+7, tmp, 10);
		strcat(strbuf, tmp);
		strcat(strbuf, ":");

		lcd_set_cursor(0, 0);
		lcd_puts(strbuf);

		for (uint8_t j=0; j<8; j++) {
			lcd_set_cursor(j*2, 1);
			lcd_putc(i+j);
		}

		i += 8;
		_delay_ms(2500);
	}
}
