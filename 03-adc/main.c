#include "stdio.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"
#include "adc-task/adc-task.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

void version_callback(const char* args);
void help_callback(const char* args);
void led_on_callback(const char* args);
void led_off_callback(const char* args);
void led_blink_callback(const char* args);
void led_blink_set_period_ms_callback(const char* args);
void mem_callback(const char* args);
void wmem_callback(const char* args);
void adc_callback ();

api_t device_api[] =
{
	{"help", help_callback, "get help"},
	{"version", version_callback, "get device name and firmware version"},
	{"on", led_on_callback, "turn led on"},
	{"off", led_off_callback, "turn led on"},
	{"blink", led_blink_callback, "set led blink"},
	{"set_period", led_blink_set_period_ms_callback, "set led blink period"},
	{"mem", mem_callback, "show mem"},
	{"wmem", wmem_callback, "show mem"},
	{"get_adc", adc_callback, "measure with ADC"},
	{NULL, NULL, NULL},
};

int main()
{
    stdio_init_all();
	
	led_task_init();
	stdio_task_init();
	protocol_task_init(device_api);
	adc_task_init(device_api);
	
	
    while (1)
    {
		protocol_task_handle(stdio_task_handle());
		led_task_handle();
    }
	return 0;
}

void version_callback(const char* args)
{
	printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void help_callback(const char* args)
{
	print_commands();
	
	return;	
}

void led_on_callback(const char* args)
{
	led_task_state_set(LED_STATE_ON);
	return;
}

void led_off_callback(const char* args)
{
	led_task_state_set(LED_STATE_OFF);
	return;
}

void led_blink_callback(const char* args)
{
	led_task_state_set(LED_STATE_BLINK);
	return;
}

void led_blink_set_period_ms_callback(const char* args)
{
	uint period_ms = 0;
	sscanf(args, "%u", &period_ms);
	if (period_ms == 0)
		printf("Blink period was not defined\n");
	else 
		led_task_set_blink_period_ms(period_ms);
	
	return;
}

void mem_callback(const char* args)
{
	uint32_t addr = 0;
	sscanf(args, "%x", &addr);
	if (addr == 0)
		printf("Mem 0 is not allowed\n");
	else 
	{	
	//	printf("Addr: %x\n", addr);
		print_mem_content(addr);
	}
	return;
}

void wmem_callback(const char* args)
{
	uint32_t addr = 0;
		
	sscanf(args, "%x", &addr);
	if (addr == 0)
		printf("Mem 0 is not allowed\n");
	else 
	{	
		char* space_symbol = strchr(args, ' ');

		if (space_symbol)
		{
			//*space_symbol = '\0';
			args = space_symbol + 1;
			
			uint32_t value = 0;
			sscanf(args, "%u", &value);
			
			set_mem_content(addr, value);
		}
		else
			printf("Value is not defined\n");
		
	}
	
	return;
}

void adc_callback ()
{
	float voltage_V = adc_task_measure();
	printf("%f\n", voltage_V);
}