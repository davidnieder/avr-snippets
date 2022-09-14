#include "usi_uart.h"


int main()
{
	char c;

	usi_uart_init();
	usi_uart_sends("usi uart initialized\n");

	while (1) {
		if (usi_uart_data_available()) {
			usi_uart_recvc(&c);
			usi_uart_sendc(c);
		}
	}
}
