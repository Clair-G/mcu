#include "adc-task.h"
#include "hardware/adc.h"
#include "stdio.h"
#include "pico/stdlib.h"

const uint ADC_PIN = 26;
const uint ADC_AIN = 0;
const uint ADC_TS_AIN = 4;


adc_task_state_t adc_state = ADC_TASK_STATE_IDLE;
uint64_t adc_ts = 0;

uint64_t ADC_TASK_MEAS_PERIOD_US = 100000;

void adc_task_init()
{
	adc_init();
	adc_gpio_init(ADC_PIN);
	adc_set_temp_sensor_enabled(true);
	return;
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

void adc_task_handle()
{
	if (adc_state == ADC_TASK_STATE_RUN)
	{
		if (time_us_64() > adc_ts) // adc_ts == 0 at first
			{
				adc_ts = time_us_64() + ADC_TASK_MEAS_PERIOD_US;
				float voltage_V = adc_task_measure();
				float temp_C = adc_task_measure_temperature();
				printf("%f %f\n", voltage_V, temp_C);
			}
	}
	return;
}

void adc_task_set_state(adc_task_state_t state)
{
	adc_state = state;
	return;
}