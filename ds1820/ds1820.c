#include <util/crc16.h>
#include "ds1820.h"

/* sensor commands */
#define DSCMD_CONVERT_T			0x44
#define DSCMD_WRITE_SCRATCHPAD	0x4E
#define DSCMD_READ_SCRATCHPAD	0xBE
#define DSCMD_COPY_SCRATCHPAD	0x48
#define DSCMD_RECALL_E2			0xB8
#define DSCMD_READ_POWER_SUPPLY	0xB4



void ds1820_new(ds1820_t *sensor, w1id_t id)
{
	for (uint8_t i=0; i<8; i++)
		sensor->id[i] = id[i];

	sensor->reg_temp = 0x0550;
	sensor->reg_th = 0x7F;
	sensor->reg_tl = 0x80;
	sensor->reg_conf = DS18B20_RESOLUTION_12;
}

uint8_t
ds1820_search_bus(ds1820_t *buffer, uint8_t size)
{
	int8_t ret, cnt = 0;
	struct w1_search_state state = { 63, { FC_DS18B20, 0, 0, 0, 0, 0, 0, 0 }};

	while (cnt < size) {
		ret = w1_search_rom(&state);
		if (ret == W1_SEARCH_FAILED) return 0;
		if (state.device_id[0] != FC_DS18B20) break;

		ds1820_new(buffer+cnt, state.device_id);
		cnt++;

		if (ret == W1_SEARCH_DONE) break;
	}

	state.last_deviation = 63;
	state.device_id[0] = FC_DS18S20;
	for (uint8_t i=1; i<8; i++) state.device_id[i] = 0;

	while (cnt < size) {
		ret = w1_search_rom(&state);
		if (ret == W1_SEARCH_FAILED) return 0;
		if (state.device_id[0] != FC_DS18S20) break;

		ds1820_new(buffer+cnt, state.device_id);
		cnt++;

		if (ret == W1_SEARCH_DONE) break;
	}

	return cnt;
}

void
ds1820_convert_t(ds1820_t *sensor)
{
	w1_reset();
	w1_match_rom(sensor->id);
	w1_write_byte(DSCMD_CONVERT_T);
}

void
ds1820_convert_t_all()
{
	w1_reset();
	w1_skip_rom();
	w1_write_byte(DSCMD_CONVERT_T);
}

uint8_t
ds1820_read_scratchpad(ds1820_t *sensor)
{
	uint8_t buf[9];
	uint8_t crc = 0;

	w1_reset();
	w1_match_rom(sensor->id);
	w1_write_byte(DSCMD_READ_SCRATCHPAD);
	for (uint8_t i=0; i<9; i++) {
		buf[i] = w1_read_byte();
		crc = _crc_ibutton_update(crc, buf[i]);
	}

	if (crc != 0) return 1;

	sensor->reg_temp = ((uint16_t)buf[1]<<8) | buf[0];
	sensor->reg_th = buf[2];
	sensor->reg_tl = buf[3];
	sensor->reg_conf = buf[4];
	return 0;
}

void
ds1820_write_scratchpad(ds1820_t *sensor)
{
	w1_reset();
	w1_match_rom(sensor->id);

	w1_write_byte(sensor->reg_th);
	w1_write_byte(sensor->reg_tl);
	if (IS_DS18S20(sensor->id))
		w1_write_byte(sensor->reg_conf);
}

void
ds1829_copy_scratchpad(ds1820_t *sensor)
{
	w1_reset();
	w1_match_rom(sensor->id);
	w1_write_byte(DSCMD_COPY_SCRATCHPAD);
}

void
ds1820_recall_e2(ds1820_t *sensor)
{
	w1_reset();
	w1_match_rom(sensor->id);
	w1_write_byte(DSCMD_RECALL_E2);
}

uint16_t
ds1820_get_temperature_raw(ds1820_t *sensor)
{
	return sensor->reg_temp;
}

int16_t
ds1820_get_temperature_deci_degrees(ds1820_t *sensor)
{
	uint8_t sign;
	uint16_t temperature, frac;

	temperature = sensor->reg_temp;
	sign = (temperature & 0x8000) != 0;
	if (sign) temperature = (~temperature)+1;
	if (IS_DS18B20(sensor->id)) {
		frac = ((temperature & 0x000F) * 63) / 100;
		temperature = ((temperature & 0x07F8) >> 4) * 10 + frac;
	} else {
		temperature *= 5;
	}
	if (sign) return -1 * temperature;
	else return temperature;
}

int16_t
ds1820_get_temperature_degrees(ds1820_t *sensor)
{
	uint8_t sign, lsb;
	uint16_t temperature;

	temperature = sensor->reg_temp;
	sign = (temperature & 0x8000) != 0;
	if (sign) {
		temperature = (~temperature) + 1;
	}
	if (IS_DS18B20(sensor->id)) {
		lsb = temperature & 0x08;
		temperature = temperature >> 4;
	} else {
		lsb = temperature & 0x01;
		temperature = temperature >> 1;
	}
	if (lsb) {
		temperature += 1;
	}
	if (sign) return -1 * temperature;
	else return temperature;
}

/*
float
ds1820_get_temperature(ds1820_t *sensor)
{

}
*/

#if STRING_CONVERSION
#include <stdlib.h>

char *
ds1820_get_temperature_as_string(ds1820_t *sensor, char *strbuf)
{
	uint8_t i = 0;
	int16_t temperature = ds1820_get_temperature_deci_degrees(sensor);

	itoa(temperature/10, strbuf, 10);
	while (strbuf[i] != '\0')
		i++;
	strbuf[i++] = '.';
	itoa(abs(temperature%10), strbuf+i, 10);

	return strbuf;
}
#endif

#if 0
int8_t
ds1820_get_alarm_high_value(ds1820_t *sensor)
{
	return sensor->reg_th;
}

void
ds1820_set_alarm_high_value(ds1820_t *sensor, int8_t temperature)
{
	sensor->reg_th = temperature;
}

int8_t
ds1820_get_alarm_low_value(ds1820_t *sensor)
{
	return sensor->reg_tl;
}

void
ds1820_set_alarm_low_value(ds1820_t *sensor, int8_t temperature)
{
	sensor->reg_tl = temperature;
}
#endif

int8_t
ds1820_get_resolution(ds1820_t *sensor)
{
	return sensor->reg_conf;
}

void
ds1820_set_resolution(ds1820_t *sensor, uint8_t resolution)
{
	if (IS_DS18B20(sensor->id))
		sensor->reg_conf = resolution;
}

uint16_t
ds1820_get_conversion_time(ds1820_t *sensor)
{
	if (IS_DS18B20(sensor->id)) {
		return DS18B20_CONV_TIME(sensor->reg_conf);
	} else {
		return DS18S20_CONV_TIME;
	}
}
