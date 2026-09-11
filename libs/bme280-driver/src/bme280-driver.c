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
	{	
		printf("Ошибка ответа датчика");
		return;
	}
	
	bmp280_get_calib_params();
	
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

/*
uint16_t bme280_read_temp_raw()
{
	uint8_t read[2] = {0};
	bme280_read_regs(BME280_REG_temp_msb, read, sizeof(read));
	uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);
	return value;
}
*/
int32_t bme280_read_temp_raw()
{
	uint8_t read[2] = {0};
	bme280_read_regs(BME280_REG_temp_msb, read, sizeof(read));
	int32_t value = (int32_t)(read[0] << 12) | (int32_t)(read[1] << 4) | (int32_t)(read[2] >> 4);
	return value;
}

int32_t bme280_read_press_raw()
{
	uint8_t read[3] = {0};
	bme280_read_regs(BME280_REG_press_msb, read, sizeof(read));
	int32_t value = (read[0] << 12) | (read[1] << 4) | (read[2] >> 4);
	return value;
}

///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
uint16_t bme280_read_hum_raw()
{
	uint8_t read[2] = {0};
	bme280_read_regs(BME280_REG_hum_msb, read, sizeof(read));
	uint16_t value = ((uint16_t)read[0] << 8) | ((uint16_t)read[1]);
	return value;
}

// intermediate function that calculates the fine resolution temperature
// used for both pressure and temperature conversions
int32_t bmp280_convert(int32_t temp) 
{
	// use the 32-bit fixed point compensation implementation given in the
	// datasheet

	int32_t var1, var2;
	var1 = ((((temp >> 3) - ((int32_t)params.dig_t1 << 1))) * ((int32_t)params.dig_t2)) >>
  11;
	var2 = (((((temp >> 4) - ((int32_t)params.dig_t1)) * ((temp >> 4) - ((int32_t)params.dig_t1))) >> 12) * ((int32_t)params.dig_t3)) >> 14;
	return ((int32_t)var1 + var2);
}

void bmp280_get_calib_params() 
{
	// raw temp and pressure values need to be calibrated according to
	// parameters generated during the manufacturing of the sensor
	// there are 3 temperature params, and 9 pressure params, each with a LSB
	// and MSB register, so we read from 24 registers
	
	uint8_t buf[NUM_CALIB_PARAMS_1] = { 0 };
	uint8_t reg = BME280_REG_DIG_T1_LSB;
	
	bme280_read_regs(BME280_REG_DIG_T1_LSB, buf, sizeof(buf));

//Debug start	
//	for (int i = 0; i < NUM_CALIB_PARAMS; i++)
//		printf("buff [], 0x%X \n", buf[i]);
//Debug end
	
	// store these in a struct for later use
	params.dig_t1 = ((uint16_t)(buf[1] << 8)) | ((uint16_t)buf[0]);
	params.dig_t2 = ((int16_t)(buf[3] << 8)) | ((int16_t)buf[2]);
	params.dig_t3 = ((int16_t)(buf[5] << 8)) | ((int16_t)buf[4]);
	
	params.dig_p1 = ((uint16_t)(buf[7] << 8)) | ((uint16_t)buf[6]);
	params.dig_p2 = ((int16_t)(buf[9] << 8)) | ((int16_t)buf[8]);
	params.dig_p3 = ((int16_t)(buf[11] << 8)) | ((int16_t)buf[10]);
	params.dig_p4 = ((int16_t)(buf[13] << 8)) | ((int16_t)buf[12]);
	params.dig_p5 = ((int16_t)(buf[15] << 8)) | ((int16_t)buf[14]);
	params.dig_p6 = ((int16_t)(buf[17] << 8)) | ((int16_t)buf[16]);
	params.dig_p7 = ((int16_t)(buf[19] << 8)) | ((int16_t)buf[18]);
	params.dig_p8 = ((int16_t)(buf[21] << 8)) | ((int16_t)buf[20]);
	params.dig_p9 = ((int16_t)(buf[23] << 8)) | ((int16_t)buf[22]);
	
	params.dig_h1 = buf[25];
	
	bme280_read_regs(BME280_REG_DIG_H2_LSB, buf, NUM_CALIB_PARAMS_2);
	
	params.dig_h2 = ((uint16_t)(buf[1] << 8)) | ((uint16_t)buf[0]);
	params.dig_h3 = buf[2];
	params.dig_h4 = ((int16_t)(buf[3] << 4)) | ((int16_t)(buf[4] & 0x0F));
	params.dig_h5 = ((int16_t)(buf[5] << 4)) | ((int16_t)(buf[4] >> 4));
	params.dig_h6 = buf[6];
		
	return;
	}

int32_t bme280_read_temp()
{
	// uses the BMP280 calibration parameters to compensate the temperature value
	//	read from its registers
	int32_t temp_raw = bme280_read_temp_raw();
	int32_t t_fine = bmp280_convert(temp_raw);
	int32_t temp = (t_fine * 5 + 128) >> 8;
	return temp;

}

int32_t bme280_read_press()
{

	// uses the BMP280 calibration parameters to compensate the temperature value
	//	read from its registers
	int32_t temp_raw = bme280_read_temp_raw();
	int32_t press_raw = bme280_read_press_raw();
	int32_t t_fine = bmp280_convert(temp_raw);
	
	
	int32_t var1, var2;
	uint32_t converted = 0.0;
	var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)params.dig_p6);
	var2 += ((var1 * ((int32_t)params.dig_p5)) << 1);
	var2 = (var2 >> 2) + (((int32_t)params.dig_p4) << 16);
	var1 = (((params.dig_p3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + 
			((((int32_t)params.dig_p2) * var1) >> 1)) >> 18;
	var1 = ((((32768 + var1)) * ((int32_t)params.dig_p1)) >> 15);
	if (var1 == 0) 
	{
		return 0; // avoid exception caused by division by zero
	}
	converted = (((uint32_t)(((int32_t)1048576) - press_raw) - (var2 >> 12))) * 3125;
	if (converted < 0x80000000) 
	{
		converted = (converted << 1) / ((uint32_t)var1);
	} 
	else 
	{
		converted = (converted / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)params.dig_p9) * ((int32_t)(((converted >> 3) * (converted >> 3)) >>
			13))) >> 12;
	var2 = (((int32_t)(converted >> 2)) * ((int32_t)params.dig_p8)) >> 13;
	converted = (uint32_t)((int32_t)converted + ((var1 + var2 + params.dig_p7) >> 4));

	return converted;

}

uint32_t bme280_read_hum()
{
	int32_t hum_raw = bme280_read_hum_raw();
	int32_t temp_raw = bme280_read_temp_raw();
	int32_t t_fine = bmp280_convert(temp_raw);
	
	int32_t hum = (t_fine - (int32_t)76800);
			
	hum =	(((((hum_raw << 14) - (((int32_t)params.dig_h4) << 20) - (((int32_t)params.dig_h5) *
			hum)) + ((int32_t)16384)) >> 15) * 
			(((((((hum * ((int32_t)params.dig_h6)) >> 10) * 
			(((hum * ((int32_t)params.dig_h3)) >> 11) + ((int32_t)32768))) >> 10) + 
			((int32_t)2097152)) * 
			((int32_t)params.dig_h2) + 8192) >> 14));
			
	hum = (hum - (((((hum >> 15) * (hum >> 15)) >> 7) * ((int32_t)params.dig_h1)) >> 4));
	
	hum = (hum < 0) ? 0 : hum;
	hum = (hum > 419430400) ? 419430400 : hum;
	
	return (uint32_t)(hum >> 12);
}


void bme280_set_tm_on()
{
	bme280_tm_state = BME280_TM_ON;
	return;
}

void bme280_set_tm_off()
{
	bme280_tm_state = BME280_TM_ON;
	return;
}
/////-----------------!!!
void bme280_task()
{
/*	
	if (bme280_tm_state == BME280_TM_ON)
	{
		if (time_us_64() > adc_ts) // adc_ts == 0 at first
			{
				adc_ts = time_us_64() + ADC_TASK_MEAS_PERIOD_US;
				float voltage_V = adc_task_measure();
				float temp_C = adc_task_measure_temperature();
				printf("%f %f\n", voltage_V, temp_C);
			}
	}
*/	
	return;
}