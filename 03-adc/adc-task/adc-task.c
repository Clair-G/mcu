#include "adc-task.h"
#include "hardware/adc.h"
#include "stdio.h"
#include "pico/stdlib.h"

const uint ADC_PIN = 26;
const uint ADC_AIN = 0;

void adc_task_init()
{
	adc_init();
	adc_gpio_init(ADC_PIN);
	
}

float adc_task_measure()
{
	adc_select_input(ADC_AIN);
	uint16_t voltage_counts = adc_read();
	float voltage_V = voltage_counts * 3.3 / 4095;
	return voltage_V;
}
