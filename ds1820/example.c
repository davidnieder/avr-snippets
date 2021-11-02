#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart.h"
#include "w1-master.h"
#include "ds1820.h"

#define MAX_SENSORS 4


int main()
{
	ds1820_t sensors[MAX_SENSORS];
	int8_t ret, count;
	char text_buf[32];

	uart_init();
	sei();

	/* search the bus for temperature sensors */
	count = ds1820_search_bus(sensors, MAX_SENSORS);
	uart_puts(itoa(count, text_buf, 10));
	uart_puts(" sensors found\n");

	while (1) {

		/* send convert_t to all sensors */
		ds1820_convert_t_all();

		/* wait for conversion */
		_delay_ms(DS1820_CONV_TIME_MAX);

		uart_puts("Readings:\n");

		for (uint8_t i=0; i<count; i++) {
			/* read sensor's scratchpad */
			ret = ds1820_read_scratchpad(&sensors[i]);
			if (ret != 0) {
				/* crc check failed */
				uart_puts("crc error\n");
				continue;
			}

			/* copy sensor's temperature to text_buf */
			ds1820_get_temperature_as_string(&sensors[i], text_buf);
			uart_puts(text_buf);
			uart_puts("\n");
		}

		uart_puts("\n");
		_delay_ms(2000);
	}
}
