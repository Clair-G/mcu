#include "bme280-driver.h"

//функция инициализации bme280, 
//принимающая указатели на функции чтения и записи в I2C
void bme280_init(bme280_i2c_read i2c_read, 	bme280_i2c_write i2c_write)
{
	bme280_ctx.i2c_read = i2c_read;
	bme280_ctx.i2c_write = i2c_write;
	return;
}

//функция чтения регистров BME280
void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length)
{
	//массив uint8_t длиной 1, содержащий адрес регистра, 
	//с которого должно начаться чтение
	uint8_t data[1] = {start_reg_address};
	
	//отправить на шину I2C адрес регистра, с которого мы хотим начать чтение
	bme280_ctx.i2c_write(data, sizeof(data));
	
	//прочитать по I2C шине желаемое количество регистров сразу в буффер пользователя
	bme280_ctx.i2c_read(buffer, length);
		
}