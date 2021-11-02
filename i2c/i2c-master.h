#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <util/twi.h>

/* i2c clock frequency in Hz, normal mode: 100kHz */
#define F_SCL 100000

#define I2C_READ TW_READ
#define I2C_WRITE TW_WRITE

/*
 * setup hardware/library, call once before send/recv.
 */
void i2c_master_init();

/*
 * send 'len' bytes from 'data' to 'slave_addr'
 *
 * returns 0 on success, 1 on error
 */
uint8_t i2c_master_send(uint8_t slave_addr, uint8_t *data, uint8_t len);

/*
 * receive 'len' bytes from 'slave_addr', store in 'buffer'
 *
 * return 0 on success, 1 on error
 */
uint8_t i2c_master_recv(uint8_t slave_addr, uint8_t *buffer, uint8_t len);

/*
 * if send/recv returned unsuccessful, this gives the reason
 * the error code is one the status codes defined in <util/twi.h>
 */
uint8_t i2c_master_last_error();

#endif
