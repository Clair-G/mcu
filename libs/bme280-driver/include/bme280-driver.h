#pragma once

#include <stdint.h>

#define NUM_CALIB_PARAMS_1 26
#define NUM_CALIB_PARAMS_2 7

//тип для калибровочных коэффициентов
 typedef struct  
 {
	// temperature params
	uint16_t dig_t1;
	int16_t dig_t2;
	int16_t dig_t3;
	
	// pressure params
	uint16_t dig_p1;
	int16_t dig_p2;
	int16_t dig_p3;
	int16_t dig_p4;
	int16_t dig_p5;
	int16_t dig_p6;
	int16_t dig_p7;
	int16_t dig_p8;
	int16_t dig_p9;
	
	uint8_t dig_h1;
	int16_t dig_h2;
	uint8_t dig_h3;
	int16_t dig_h4;
	int16_t dig_h5;
	int8_t dig_h6;
	
} bmp280_calib_param_t;

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
//uint16_t bme280_read_temp_raw();
int32_t bme280_read_temp_raw();

//прототип функции чтения значений температуры
int32_t bme280_read_press_raw();

//прототип функции чтения значений температуры
uint16_t bme280_read_hum_raw();

//получение t_fine
int32_t bmp280_convert(int32_t temp);

//получение калибровочных параметров
void bmp280_get_calib_params();

//прототип функции чтения значений температуры C
int32_t bme280_read_temp();

//прототип функции чтения значений давления кПа
int32_t bme280_read_press();

//прототип функции чтения значений влажности
uint32_t bme280_read_hum();


//тип для объединения всех переменных, необходимых для работы драйвера
typedef struct
{
	bme280_i2c_read i2c_read;
	bme280_i2c_write i2c_write;
} bme280_ctx_t;



//контекст драйвера BME280
static bme280_ctx_t bme280_ctx = {0};

//калибровочные параметры экземпляра BME280
static bmp280_calib_param_t params = {0};

