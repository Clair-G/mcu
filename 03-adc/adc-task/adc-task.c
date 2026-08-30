#include "adc-task.h"
#include "hardware/adc.h"
#include "stdio.h"
#include "pico/stdlib.h"

const uint ADC_PIN = 26;
const uint ADC_AIN = 0;
const uint ADC_TS_AIN = 4;

void adc_task_init()
{
	adc_init();
	adc_gpio_init(ADC_PIN);
	adc_set_temp_sensor_enabled(true);
	
}

float adc_task_measure()
{
	adc_select_input(ADC_AIN);
	uint16_t voltage_counts = adc_read();
	float voltage_V = voltage_counts * 3.3 / 4095;
	return voltage_V;
}

float adc_task_measure_temperature()
{
	adc_select_input(ADC_TS_AIN);
	uint16_t temp_counts = adc_read();
	float temp_V = temp_counts * 3.3 / 4095;
	float temp_C = 27.0f - (temp_V - 0.706f) / 0.001721f;
	return temp_C;
}