#include <stddef.h>

#include "ina219.h"
#include "i2c-master.h"


static ina219_t devices[INA219_MAX_DEVICES];

static uint8_t
ina219_read_register(ina219_t *ina219, uint8_t reg, uint16_t *dest)
{
	uint8_t ret, buf[2];

	/* write device register pointer */
	ret = i2c_master_send(ina219->addr, &reg, 1);
	if (ret != 0) {
		return ret;
	}

	/* read content of 2-byte register */
	ret = i2c_master_recv(ina219->addr, buf, 2);
	if (ret != 0) {
		return ret;
	}

	*dest = ((uint16_t)buf[0]<<8) | buf[1];
	return 0;
}

static uint8_t
ina219_write_register(ina219_t *ina219, uint8_t reg, uint16_t value)
{
	uint8_t msg[3] = { reg, value>>8, value };
	return i2c_master_send(ina219->addr, msg, 3);
}

static uint8_t
ina219_write_single_setting(ina219_t *ina219, uint16_t mask, uint16_t value)
{
	uint8_t ret;
	uint16_t config;

	ret = ina219_read_config(ina219, &config);
	if (ret != 0) {
		return ret;
	}

	config = (config & ~mask) | value;
	return ina219_configure(ina219, config);
}

ina219_t *
ina219_new(uint8_t i2c_addr)
{
	static uint8_t count = 0;

	if (count >= INA219_MAX_DEVICES)
		return NULL;

	devices[count].addr = i2c_addr;
	devices[count].current_lsb = 0;

	//devices[count].config = INA219_REG_CONFIG_DEFAULT;
	//devices[count].shunt_voltage = 0;
	//devices[count].bus_voltage = 0;
	//devices[count].power = 0;
	//devices[count].current = 0;
	//devices[count].calibration = 0;

	return &devices[count++];
}

//uint8_t
//ina219_test_connection(ina219_t *ina219)
//{
//	return i2c_master_send(ina219->addr, NULL, 0);
//}

uint8_t
ina219_configure(ina219_t *ina219, uint16_t config)
{
	return ina219_write_register(ina219, INA219_REG_CONFIG, config);
}

uint8_t
ina219_set_mode(ina219_t *ina219, uint16_t mode)
{
	return ina219_write_single_setting(ina219, INA219_CONFIG_MODE_MASK, mode);
}

uint8_t
ina219_set_voltage_range(ina219_t *ina219, uint16_t range)
{
	return ina219_write_single_setting(ina219, INA219_CONFIG_VRANGE_MASK, range);
}

uint8_t
ina219_set_gain(ina219_t *ina219, uint16_t gain)
{
	return ina219_write_single_setting(ina219, INA219_CONFIG_GAIN_MASK, gain);
}

uint8_t
ina219_set_bus_adc(ina219_t *ina219, uint16_t badc)
{
	return ina219_write_single_setting(ina219, INA219_CONFIG_BADC_MASK, badc);
}

uint8_t
ina219_set_shunt_adc(ina219_t *ina219, uint16_t sadc)
{
	return ina219_write_single_setting(ina219, INA219_CONFIG_SADC_MASK, sadc);
}

uint8_t
ina219_calibrate(ina219_t *ina219, uint16_t cal_value)
{
	switch (cal_value) {
		case INA219_CALIBRATION_MAX_05A:
			ina219->current_lsb = INA219_CURRENT_LSB_CAL_05A;
			break;
		case INA219_CALIBRATION_MAX_2A:
			ina219->current_lsb = INA219_CURRENT_LSB_CAL_2A;
			break;
		case INA219_CALIBRATION_MAX_4A:
			ina219->current_lsb = INA219_CURRENT_LSB_CAL_4A;
			break;
		case INA219_CALIBRATION_MAX_8A:
			ina219->current_lsb = INA219_CURRENT_LSB_CAL_8A;
			break;
	}

	return ina219_write_register(ina219, INA219_REG_CAL, cal_value);
}

uint8_t
ina219_reset(ina219_t *ina219)
{
	return ina219_write_register(ina219, INA219_REG_CONFIG, 0x8000);
}



uint8_t
ina219_read_config(ina219_t *ina219, uint16_t *dest)
{
	return ina219_read_register(ina219, INA219_REG_CONFIG, dest);
}

//uint8_t
//ina219_read_bus_register(ina219_t *ina219, uint16_t *dest)
//{
//	return ina219_read_register(ina219, INA219_REG_BUS_V, dest);
//}
//
//uint8_t
//ina219_read_shunt_register(ina219_t *ina219, uint16_t *dest)
//{
//	return ina219_read_register(ina219, INA219_REG_SHUNT_V, dest);
//}
//
//uint8_t
//ina219_read_power_register(ina219_t *ina219, uint16_t *dest)
//{
//	return ina219_read_register(ina219, INA219_REG_POWER, dest);
//}
//
//uint8_t
//ina219_read_current_register(ina219_t *ina219, uint16_t *dest)
//{
//	return ina219_read_register(ina219, INA219_REG_CURRENT, dest);
//}

uint8_t
ina219_get_bus_voltage(ina219_t *ina219, uint16_t *bus_mv)
{
	uint8_t ret;

	ret = ina219_read_register(ina219, INA219_REG_BUS_V, bus_mv);
	if (ret != 0) {
		return ret;
	}

	*bus_mv = (*bus_mv>>3)*4;
	return 0;
}

uint8_t
ina219_get_shunt_voltage(ina219_t *ina219, int16_t *shunt_mv)
{
	uint8_t ret;

	ret = ina219_read_register(ina219, INA219_REG_SHUNT_V, (uint16_t *)shunt_mv);
	if ( ret != 0) {
		return ret;
	}

	*shunt_mv *= 0.01;
	return 0;
}

uint8_t
ina219_get_power(ina219_t *ina219, uint16_t *power_mw)
{
	uint8_t ret;

	ret = ina219_read_register(ina219, INA219_REG_POWER, power_mw);
	if (ret != 0) {
		return ret;
	}

	*power_mw *= 0.02*ina219->current_lsb;
	return 0;
}

uint8_t
ina219_get_current(ina219_t *ina219, int16_t *current_ma)
{
	uint8_t ret;

	ret = ina219_read_register(ina219, INA219_REG_CURRENT, (uint16_t *)current_ma);
	if (ret != 0) {
		return ret;
	}

	*current_ma *= 0.001*ina219->current_lsb;
	return 0;
}
