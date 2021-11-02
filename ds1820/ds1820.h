#ifndef DS1820_H
#define DS1820_H

#include "w1-master.h"

typedef struct {
	w1id_t id;
	uint16_t reg_temp;
	uint8_t reg_th;
	uint8_t reg_tl;
	uint8_t reg_conf;
} ds1820_t;

/* sensor family codes */
#define FC_DS18B20 0x28
#define FC_DS18S20 0x10

#define IS_DS18B20(id) ((id)[0]==FC_DS18B20)
#define IS_DS18S20(id) ((id)[0]==FC_DS18S20)

#define DS18B20_RESOLUTION_9	0x00
#define DS18B20_RESOLUTION_10	0x20
#define DS18B20_RESOLUTION_11	0x40
#define DS18B20_RESOLUTION_12	0x60

#define DS1820_CONV_TIME_MAX 750
#define DS18S20_CONV_TIME 750
#define DS18B20_CONV_TIME_MIN 94
#define DS18B20_CONV_TIME(res) (DS18B20_CONV_TIME_MIN*(1<<(res>>5)))


/*
 * initializes the given 'sensor' struct with device id 'id'
 * and default values;
 */
void ds1820_new(ds1820_t *sensor, w1id_t id);

/*
 * searches the bus for DS18B20 and DS18S20 sensors
 * and puts up to 'size' sensors in 'buffer'
 */
uint8_t ds1820_search_bus(ds1820_t *buffer, uint8_t size);

/*
 * send the 'convert_t' signal to a sensor. this triggers
 * a temperature conversion.
 */
void ds1820_convert_t(ds1820_t *sensor);

/*
 * broadcast the 'convert_t' signal to all devices on the bus
 */
void ds1820_convert_t_all();

/*
 * copies the sensor's internal scratchpad memory into the
 * ds1820_t struct
 */
uint8_t ds1820_read_scratchpad(ds1820_t *sensor);

/*
 * writes the data from 'sensor' into the sensor's scratchpad
 */
void ds1820_write_scratchpad(ds1820_t *sensor);

/*
 * copies sensor's scratchpad to eeprom.
 */
void ds1829_copy_scratchpad(ds1820_t *sensor);

/*
 * copy eeprom to scratchpad
 */
void ds1821_recall_e2(ds1820_t *sensor);

/*
 * get the raw value of the temperature register
 */
uint16_t ds1820_get_temperature_raw(ds1820_t *sensor);

/*
 * get the temperature in deci (1/10) degrees
 */
int16_t ds1820_get_temperature_deci_degrees(ds1820_t *sensor);

/*
 * get the temperature in degrees celsius
 */
int16_t ds1820_get_temperature_degrees(ds1820_t *sensor);

#ifndef STRING_CONVERSION
#define STRING_CONVERSION 1
/*
 * writes the temperature as a string to 'strbuf'.
 * 'strbuf' needs to be at least 7 bytes wide.
 */
char *ds1820_get_temperature_as_string(ds1820_t *sensor, char *strbuf);
#endif

/*
 * returns the set resolution of the sensor, ie one of the
 * DS18B20_RESOLUTION_* symbols
 *
 * (only for DS18B20 sensors)
 */
int8_t ds1820_get_resolution(ds1820_t *sensor);

/*
 * set temperature resolution (only for DS18B20). possible values
 * for resolution are:
 *  DS18B20_RESOLUTION_9
 *  DS18B20_RESOLUTION_10
 *  DS18B20_RESOLUTION_11
 *  DS18B20_RESOLUTION_12
 */
void ds1820_set_resolution(ds1820_t *sensor, uint8_t resolution);


/*
 * get the time (in ms) the sensor needs after a "convert_t" signal
 * to complete the temperature conversion and update the value in it's
 * internal scratchpad memory.
 */
uint16_t ds1820_get_conversion_time(ds1820_t *sensor);

#endif
