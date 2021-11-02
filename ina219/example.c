#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart.h"
#include "i2c-master.h"
#include "ina219.h"


void print16(uint16_t word)
{
	uart_putb(word>>8);
	uart_putb(word);
}

void print_config(ina219_t *ina219)
{
	uint16_t cfg;

	if (ina219_read_config(ina219, &cfg) == 0) {
		uart_puts("config: 0x");
		print16(cfg);
		uart_putc('\n');
	} else {
		uart_puts("read_config error\n");
	}
}

int main()
{
	ina219_t *ina219;
	uint16_t bus_mv, power_mw;
	int16_t shunt_mv, current_ma;

	DDRB |= (1<<PB0);

	uart_init();
	sei();

	/* initialize i2c lib */
	i2c_master_init();

	/* one device with the default address of 0x40 */
	ina219 = ina219_new(0x40);

	/* let's try to reset the device */
	if (ina219_reset(ina219) != 0) {
		/* sth went wrong, we could investigate with
		 * i2c_master_last_error() */
		uart_puts("error\n");
	}

	/* to measure current and power, we need to calibrate the device:
	 * let's say we expect an absolut maximum current of 1A. the next
	 * bigger calculated value is for 2A, so let's use that one.
	 * (the predefined values are only valid for shunt resistors of 0.1 Ohm!) */
	if (ina219_calibrate(ina219, INA219_CALIBRATION_MAX_2A) == 0) {
		/* calibration value was set */
	} else {
		uart_puts("error\n");
	}

	/* if we have a supply voltage of eg 12V, we can lower the voltage range
	 * from 32V (default) to 16V */
	ina219_set_voltage_range(ina219, INA219_CONFIG_VRANGE_16);
	/* skipped error checking here! */

	/* a maximum current of 1A and a shunt resistor of 0.1 Ohm leads to maximum
	 * voltage drop of 100mV over the shunt. we can adjust the gain and lower
	 * the range of shunt voltage measurements to 160mV */
	ina219_set_gain(ina219, INA219_CONFIG_GAIN_160);
	/* skipped error checking! */


	while (1) {
		PORTB ^= (1<<PB0);

		/* by default the ina219 measures both shunt and bus voltages
		 * continuously and calculates current and power accordingly.
		 * let's read all these values in a loop */

		if (ina219_get_bus_voltage(ina219, &bus_mv) == 0) {
			/* bus_mv holds a successful reading: the voltage at V- in mV */
			uart_puts("bv: "); print16(bus_mv); uart_putc('\n');
		}

		if (ina219_get_shunt_voltage(ina219, &shunt_mv) == 0) {
			/* shunt_mv holds the voltage drop over the shunt in mV */
			uart_puts("sv: "); print16(shunt_mv); uart_putc('\n');
		}

		if (ina219_get_current(ina219, &current_ma) == 0) {
			/* current_ma holds the current through the shunt in mA */
			uart_puts("c:  "); print16(current_ma); uart_putc('\n');
		}

		_delay_ms(50);

		if (ina219_get_power(ina219, &power_mw) == 0) {
			/* power_mw holds the calculated power in mW */
			uart_puts("p:  "); print16(power_mw); uart_putc('\n');
		}

		uart_putc('\n');
		_delay_ms(2000);
	}
}
