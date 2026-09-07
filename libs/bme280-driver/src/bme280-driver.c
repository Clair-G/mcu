#include "bme280-driver.h"
#include "bme280-regs.h"

//функция инициализации bme280, 
//принимающая указатели на функции чтения и записи в I2C
void bme280_init(bme280_i2c_read i2c_read, 	bme280_i2c_write i2c_write)
{
	bme280_ctx.i2c_read = i2c_read;
	bme280_ctx.i2c_write = i2c_write;
	
	uint8_t id_reg_buf[1] = {0};
	bme280_read_regs(BME280_REG_id, id_reg_buf, sizeof(id_reg_buf));
	if (id_reg_buf[0] != 0x60)
		printf("Ошибка ответа датчика");
	
	uint8_t ctrl_hum_reg_value = 0;
	ctrl_hum_reg_value |= (0b001 << 0); // osrs_h[2:0] = oversampling 1
	bme280_write_reg(BME280_REG_ctrl_hum, ctrl_hum_reg_value);
	
	uint8_t config_reg_value = 0;
	config_reg_value |= (0b0 << 0); // spi3w_en[0:0] = false
	config_reg_value |= (0b000 << 2); // filter[4:2] = Filter off
	config_reg_value |= (0b001 << 5); // t_sb[7:5] = 62.5 ms
	bme280_write_reg(BME280_REG_config, config_reg_value);
	
	uint8_t ctrl_meas_reg_value = 0;
	ctrl_meas_reg_value |= (0b11 << 0); // режим BME280 = Normal mode
	ctrl_meas_reg_value |= (0b001 << 2); // Temperature oversampling = 1
	ctrl_meas_reg_value |= (0b001 << 5); // Pressure oversampling = 1
	bme280_write_reg(BME280_REG_ctrl_meas, ctrl_meas_reg_value);
	
	
	
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
	
	return;		
}

void bme280_write_reg(uint8_t reg_address, uint8_t value)
{
	uint8_t data[2] = {reg_address, value};
	bme280_ctx.i2c_write(data, sizeof(data));
	
	return;	
}


uint16_t bme280_read_temp_raw()
{
	uint8_t read[2] = {0};
	bme280_read_regs(BME280_REG_temp_msb, read, sizeof(read));
	uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);
	return value;
}

uint16_t bme280_read_press_raw()
{
	uint8_t read[2] = {0};
	bme280_read_regs(BME280_REG_press_msb, read, sizeof(read));
	uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);
	return value;
}

uint16_t bme280_read_hum_raw()
{
	uint8_t read[2] = {0};
	bme280_read_regs(BME280_REG_hum_msb, read, sizeof(read));
	uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);
	return value;
}