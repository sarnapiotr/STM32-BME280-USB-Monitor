/*
 * bme280.c
 *
 *  Created on: 22 mar 2026
 *      Author: Piotr Sarna
 */

#include "bme280.h"

#define BME280_ADDR (0x76 << 1)

static I2C_HandleTypeDef *bme_hi2c;

uint16_t dig_T1;
int16_t dig_T2, dig_T3;
uint16_t dig_P1;
int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
uint8_t dig_H1, dig_H3;
int16_t dig_H2, dig_H4, dig_H5;
int8_t dig_H6;
int32_t t_fine;

void BME280_Init(I2C_HandleTypeDef *hi2c) {
	bme_hi2c = hi2c;
	uint8_t trim_data[32];

	HAL_I2C_Mem_Read(bme_hi2c, BME280_ADDR, 0x88, 1, trim_data, 24, 100);
	dig_T1 = (trim_data[1] << 8) | trim_data[0];
	dig_T2 = (trim_data[3] << 8) | trim_data[2];
	dig_T3 = (trim_data[5] << 8) | trim_data[4];
	dig_P1 = (trim_data[7] << 8) | trim_data[6];
	dig_P2 = (trim_data[9] << 8) | trim_data[8];
	dig_P3 = (trim_data[11] << 8) | trim_data[10];
	dig_P4 = (trim_data[13] << 8) | trim_data[12];
	dig_P5 = (trim_data[15] << 8) | trim_data[14];
	dig_P6 = (trim_data[17] << 8) | trim_data[16];
	dig_P7 = (trim_data[19] << 8) | trim_data[18];
	dig_P8 = (trim_data[21] << 8) | trim_data[20];
	dig_P9 = (trim_data[23] << 8) | trim_data[22];

	HAL_I2C_Mem_Read(bme_hi2c, BME280_ADDR, 0xA1, 1, &dig_H1, 1, 100);
	HAL_I2C_Mem_Read(bme_hi2c, BME280_ADDR, 0xE1, 1, trim_data, 7, 100);
	dig_H2 = (trim_data[1] << 8) | trim_data[0];
	dig_H3 = trim_data[2];
	dig_H4 = (trim_data[3] << 4) | (trim_data[4] & 0x0F);
	dig_H5 = (trim_data[5] << 4) | (trim_data[4] >> 4);
	dig_H6 = trim_data[6];

	uint8_t ctrl_hum = 0x01;
	uint8_t ctrl_meas = 0x27;
	uint8_t config = 0xA0;

	HAL_I2C_Mem_Write(bme_hi2c, BME280_ADDR, 0xF2, 1, &ctrl_hum, 1, 100);
	HAL_I2C_Mem_Write(bme_hi2c, BME280_ADDR, 0xF4, 1, &ctrl_meas, 1, 100);
	HAL_I2C_Mem_Write(bme_hi2c, BME280_ADDR, 0xF5, 1, &config, 1, 100);
}

void BME280_Read(float *temperature, float *pressure, float *humidity) {
	uint8_t raw[8];

	HAL_I2C_Mem_Read(bme_hi2c, BME280_ADDR, 0xF7, 1, raw, 8, 100);

	int32_t adc_P = (raw[0] << 12) | (raw[1] << 4) | (raw[2] >> 4);
	int32_t adc_T = (raw[3] << 12) | (raw[4] << 4) | (raw[5] >> 4);
	int32_t adc_H = (raw[6] << 8) | raw[7];

	int32_t var1_t = ((((adc_T >> 3) - ((int32_t) dig_T1 << 1))) * ((int32_t) dig_T2)) >> 11;
	int32_t var2_t = (((((adc_T >> 4) - ((int32_t) dig_T1)) * ((adc_T >> 4) - ((int32_t) dig_T1))) >> 12) * ((int32_t) dig_T3)) >> 14;

	t_fine = var1_t + var2_t;

	*temperature = (t_fine * 5 + 128) >> 8;
	*temperature /= 100.0f;

	int64_t var1_p, var2_p, p;

	var1_p = ((int64_t) t_fine) - 128000;
	var2_p = var1_p * var1_p * (int64_t) dig_P6;
	var2_p = var2_p + ((var1_p * (int64_t) dig_P5) << 17);
	var2_p = var2_p + (((int64_t) dig_P4) << 35);
	var1_p = ((var1_p * var1_p * (int64_t) dig_P3) >> 8) + ((var1_p * (int64_t) dig_P2) << 12);
	var1_p = (((((int64_t) 1) << 47) + var1_p)) * ((int64_t) dig_P1) >> 33;

	if (var1_p == 0) {
		*pressure = 0;
	}
	else {
		p = 1048576 - adc_P;
		p = (((p << 31) - var2_p) * 3125) / var1_p;
		var1_p = (((int64_t) dig_P9) * (p >> 13) * (p >> 13)) >> 25;
		var2_p = (((int64_t) dig_P8) * p) >> 19;
		p = ((p + var1_p + var2_p) >> 8) + (((int64_t) dig_P7) << 4);
		*pressure = (float) p / 25600.0f;
	}

	int32_t v_x1_u32r = (t_fine - ((int32_t) 76800));

	v_x1_u32r = (((((adc_H << 14) - (((int32_t) dig_H4) << 20)
			- (((int32_t) dig_H5) * v_x1_u32r)) + ((int32_t) 16384)) >> 15)
			* (((((((v_x1_u32r * ((int32_t) dig_H6)) >> 10)
			* (((v_x1_u32r * ((int32_t) dig_H3)) >> 11)
			+ ((int32_t) 32768))) >> 10) + ((int32_t) 2097152))
			* ((int32_t) dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t) dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
	v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;

	*humidity = (float) (v_x1_u32r >> 12) / 1024.0f;
}
