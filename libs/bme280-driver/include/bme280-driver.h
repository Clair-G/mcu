#pragma once

#include <stdint.h>


//типы указателей на функции для чтения и записи данных в I2C шину
typedef void (*bme280_i2c_read)(uint8_t* buffer, uint16_t length);
typedef void (*bme280_i2c_write)(uint8_t* data, uint16_t size);

//прототип функции инициализации bme280, 
//принимающий указатели на функции чтения и записи в I2C
void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write);

//прототип функции чтения регистров BME280
void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length);

//прототип функции записи в регистр BME280:
void bme280_write_reg(uint8_t reg_address, uint8_t value);

//прототип функции чтения значений температуры
uint16_t bme280_read_temp_raw();

//тип для объединения всех переменных, необходимых для работы драйвера
typedef struct
{
	bme280_i2c_read i2c_read;
	bme280_i2c_write i2c_write;
} bme280_ctx_t;

//контекст драйвера BME280
static bme280_ctx_t bme280_ctx = {0};

