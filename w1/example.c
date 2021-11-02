#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/crc16.h>

#include "uart/uart.h"
#include "w1-master.h"


#define BUFSIZE 4

int main()
{
	uint8_t device_count, crc;
	uint8_t device_id[BUFSIZE][8];
	char text_buf[6];

	uart_init();
	sei();

	while (1) {

		device_count = w1_find_devices(device_id, BUFSIZE);
		itoa(device_count, text_buf, 10);

		uart_puts("Found "); uart_puts(text_buf); uart_puts(" device(s)\n");

		for (uint8_t i=0; i<device_count; i++) {
			crc = 0;
			uart_puts(itoa(i+1, text_buf, 10));
			uart_puts(".  ");
			for (uint8_t j=0; j<8; j++)	{
				crc = _crc_ibutton_update(crc, device_id[i][j]);
				uart_puts(itoa(device_id[i][j], text_buf, 16));
				uart_puts(" ");
			}
			uart_puts(" (crc ");
			if (crc == 0) uart_puts("ok)\n");
			else uart_puts("failed)\n");

		}

		_delay_ms(2000);
	}
}
