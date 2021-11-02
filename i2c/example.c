#include <util/delay.h>
#include "i2c-master.h"


int main()
{
	int8_t ret;
	uint8_t data[] = { 0x00, 0x01, 0x02 };

	i2c_master_init();
	while (1) {

		/* write 0 bytes (ie nothing) to device 0x00
		 * this addresses every slave and if at least
		 * one is present, we expect an ACK */
		ret = i2c_master_send(0x00, data, 0);
		if (ret != 0) {
			/* some error (eg no ACK) see i2c_master_last_error() */
		}

		/* write 1 byte to slave 0x40 */
		ret = i2c_master_send(0x40, data, 1);
		if (ret != 0) {
			/* error case */
		}

		/* read 2 bytes from slave 0x40 */
		ret = i2c_master_recv(0x40, data+1, 2);
		if (ret != 0) {
			/* error case */
		}

		_delay_ms(2000);
	}
}
