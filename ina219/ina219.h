#ifndef INA219_H
#define INA219_H

#include <stdint.h>


/* amount of devices in this application */
#define INA219_MAX_DEVICES 1

/* pre calculated values for the calibration register.
 * call calibrate() with the one closest to the
 * maximum current in your application
 *
 * values only valid for a shunt resistor of 0.1 Ohm! */
#define INA219_CALIBRATION_MAX_05A	20480
#define INA219_CALIBRATION_MAX_2A	4096
#define INA219_CALIBRATION_MAX_4A	2048
#define INA219_CALIBRATION_MAX_8A	1024

/* smallest measurable currents with above calibration values */
#define INA219_CURRENT_LSB_CAL_05A	20
#define INA219_CURRENT_LSB_CAL_2A	100
#define INA219_CURRENT_LSB_CAL_4A	200
#define INA219_CURRENT_LSB_CAL_8A	400

/* the ina219 registers */
#define INA219_REG_CONFIG	0x00
#define INA219_REG_SHUNT_V	0x01
#define INA219_REG_BUS_V	0x02
#define INA219_REG_POWER	0x03
#define INA219_REG_CURRENT	0x04
#define INA219_REG_CAL		0x05

/* voltage range configuration */
#define INA219_CONFIG_VRANGE_16	0x0000
#define INA219_CONFIG_VRANGE_32	0x8000

/* shunt gain configuration */
#define INA219_CONFIG_GAIN_40	0x0000
#define INA219_CONFIG_GAIN_80	0x0800
#define INA219_CONFIG_GAIN_160	0x1000
#define INA219_CONFIG_GAIN_320	0x1800

/* bus voltage adc configuration */
#define INA219_CONFIG_BADC_1S_9B	0x0000
#define INA219_CONFIG_BADC_1S_10B	0x0080
#define INA219_CONFIG_BADC_1S_11B	0x0100
#define INA219_CONFIG_BADC_1S_12B	0x0180
#define INA219_CONFIG_BADC_2S_12B	0x0480
#define INA219_CONFIG_BADC_4S_12B	0x0500
#define INA219_CONFIG_BADC_8S_12B	0x0580
#define INA219_CONFIG_BADC_16S_12B	0x0600
#define INA219_CONFIG_BADC_32S_12B	0x0680
#define INA219_CONFIG_BADC_64S_12B	0x0700
#define INA219_CONFIG_BADC_128S_12B	0x0780

/* shunt voltage adc configuration */
#define INA219_CONFIG_SADC_1S_9B	0x0000
#define INA219_CONFIG_SADC_1S_10B	0x0008
#define INA219_CONFIG_SADC_1S_11B	0x0010
#define INA219_CONFIG_SADC_1S_12B	0x0018
#define INA219_CONFIG_SADC_2S_12B	0x0048
#define INA219_CONFIG_SADC_4S_12B	0x0050
#define INA219_CONFIG_SADC_8S_12B	0x0058
#define INA219_CONFIG_SADC_16S_12B	0x0060
#define INA219_CONFIG_SADC_32S_12B	0x0068
#define INA219_CONFIG_SADC_64S_12B	0x0070
#define INA219_CONFIG_SADC_128S_12B	0x0078

/* device operating mode configuration */
#define INA219_CONFIG_MODE_POWER_DOWN	0x0000
#define INA219_CONFIG_MODE_SV_TRIGD		0x0001
#define INA219_CONFIG_MODE_BV_TRIGD		0x0002
#define INA219_CONFIG_MODE_SBV_TRIGD	0x0003
#define INA219_CONFIG_MODE_ADC_OFF		0x0004
#define INA219_CONFIG_MODE_SV_CONT		0x0005
#define INA219_CONFIG_MODE_BV_CONT		0x0005
#define INA219_CONFIG_MODE_SBV_CONT		0x0007

#define INA219_CONFIG_DEFAULT	0x399F
#define INA219_CONFIG_VRANGE_MASK	0x2000
#define INA219_CONFIG_GAIN_MASK		0x1800
#define INA219_CONFIG_BADC_MASK		0x0780
#define INA219_CONFIG_SADC_MASK		0x0078
#define INA219_CONFIG_MODE_MASK		0x0007


typedef struct {
	uint8_t addr;
	uint16_t current_lsb;
} ina219_t;


/*
 * call for every ina219 device on the bus you want to address
 * and use the returned pointer when calling the functions declared
 * below.
 *
 * returns NULL on error (see INA219_MAX_DEVICES above)
 */
ina219_t *ina219_new(uint8_t i2c_addr);

/*
 * writes 'config' to the configuration register of the
 * given ina219 device.
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_configure(ina219_t *ina219, uint16_t config);

/*
 * sets the operating mode of the device.
 * 'mode' is on of the INA219_CONFIG_MODE_* values defined above
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_set_mode(ina219_t *ina219, uint16_t mode);

/*
 * sets the voltage range measurable between V- and GND.
 * 'range' is INA219_CONFIG_VRANGE_16 or INA219_CONFIG_VRANGE_32
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_set_voltage_range(ina219_t *ina219, uint16_t range);

/*
 * sets the gain used when measuring shunt voltage.
 * 'gain' is one of the INA219_CONFIG_GAIN_* values defined above.
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_set_gain(ina219_t *ina219, uint16_t gain);

/*
 * sets the resolution and sample size when measuring bus voltage.
 * 'badc' is one of the INA219_CONFIG_BADC_* values defined above.
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_set_bus_adc(ina219_t *ina219, uint16_t badc);

/*
 * sets the resolution and sample size when measuring shunt voltage.
 * 'sadc' is one of the INA219_CONFIG_SADC_* values defined above.
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_set_shunt_adc(ina219_t *ina219, uint16_t sadc);

/*
 * writes 'cal_value' to the calibration register of the given ina219 device.
 * 'cal_value' should be one of the INA219_CURRENT_LSB_CAL_* values
 * defined above.
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_calibrate(ina219_t *ina219, uint16_t cal_value);

/*
 * performs a reset on the given ina219 device.
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_reset(ina219_t *ina219);

/*
 * writes the contents of the configuration register to 'dest'.
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_read_config(ina219_t *ina219, uint16_t *dest);

/*
 * reads the bus voltage register and stores the value converted
 * to milli volts in 'bus_mv'.
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_get_bus_voltage(ina219_t *ina219, uint16_t *bus_mv);

/*
 * reads the shunt voltage register and stores the signed value
 * converted to milli volts in 'shunt_mv'.
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_get_shunt_voltage(ina219_t *ina219, int16_t *shunt_mv);

/*
 * reads the power register and stores the value converted to milli
 * watts in 'power_mw'.
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_get_power(ina219_t *ina219, uint16_t *power_mw);

/*
 * reads the current register and stores the signed value converted
 * to milli amperes in 'current_ma'
 *
 * returns 0 on success, non-zero on error
 */
uint8_t ina219_get_current(ina219_t *ina219, int16_t *current_ma);


//uint8_t ina219_read_bus_register(ina219_t *ina219, uint16_t *dest);
//uint8_t ina219_read_shunt_register(ina219_t *ina219, uint16_t *dest);
//uint8_t ina219_read_power_register(ina219_t *ina219, uint16_t *dest);
//uint8_t ina219_read_current_register(ina219_t *ina219, uint16_t *dest);

#endif
