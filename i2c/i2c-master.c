#include "i2c-master.h"


static uint8_t last_error = 0;

void
i2c_master_init()
{
	/* no clock prescaler */
	TWSR = 0;
	/* set SCL frequency */
	TWBR = (F_CPU/F_SCL - 16)/2;
}

static uint8_t
i2c_master_start(uint8_t slave_addr, uint8_t data_direction)
{
	/* send start condition */
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	/* wait for transmission */
	while (!(TWCR & (1<<TWINT))) ;

	/* return on error */
	if (TW_STATUS != TW_START && TW_STATUS != TW_REP_START) {
		last_error = TW_STATUS;
		return 1;
	}

	/* send slave address and data direction */
	TWDR = (slave_addr<<1) | data_direction;
	TWCR = (1<<TWINT) | (1<<TWEN);
	/* wait for transmission */
	while (!(TWCR & (1<<TWINT))) ;

	/* return on error */
	if (TW_STATUS != TW_MT_SLA_ACK && TW_STATUS != TW_MR_SLA_ACK) {
		last_error = TW_STATUS;
		return 1;
	}

	/* success, we are now in master-tx/master-rx mode */
	return 0;
}

static void
i2c_master_stop()
{
	/* send stop contition */
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
	/* wait for transmission (TWI clears TWSTO bit) */
	while (TWCR & (1<<TWSTO)) ;
}

uint8_t
i2c_master_send(uint8_t slave_addr, uint8_t *data, uint8_t len)
{
	int8_t ret;

	ret = i2c_master_start(slave_addr, I2C_WRITE);
	if (ret != 0) {
		i2c_master_stop();
		return 1;
	}

	while (len--) {
		TWDR = *data++;
		TWCR = (1<<TWINT) | (1<<TWEN);

		while (!(TWCR & (1<<TWINT))) ;
		if (TW_STATUS != TW_MT_DATA_ACK) {
			last_error = TW_STATUS;
			i2c_master_stop();
			return 1;
		}
	}

	i2c_master_stop();
	return 0;
}

uint8_t
i2c_master_recv(uint8_t slave_addr, uint8_t *buffer, uint8_t len)
{
	int8_t ret;

	ret = i2c_master_start(slave_addr, I2C_READ);
	if (ret != 0) {
		i2c_master_stop();
		return 1;
	}

	while (len-- > 1) {
		/* receive one byte, respond with ack */
		TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
		/* wait for receiver */
		while (!(TWCR & (1<<TWINT))) ;
		/* write to buffer */
		*buffer++ = TWDR;
	}

	/* receive last byte, respond with nack */
	TWCR = (1<<TWINT) | (1<<TWEN);
	/* wait for receiver */
	while (!(TWCR & (1<<TWINT))) ;
	/* write to buffer */
	*buffer = TWDR;

	i2c_master_stop();

	return 0;
}

uint8_t
i2c_master_last_error()
{
	return last_error;
}
