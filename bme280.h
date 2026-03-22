/*
 * bme280.h
 *
 *  Created on: 22 mar 2026
 *      Author: Piotr Sarna
 */

#ifndef INC_BME280_H_
#define INC_BME280_H_

#include "stm32f4xx_hal.h"

void BME280_Init(I2C_HandleTypeDef *hi2c);

void BME280_Read(float *temperature, float *pressure, float *humidity);

#endif /* INC_BME280_H_ */
