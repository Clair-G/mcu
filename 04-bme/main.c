#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "led-task/led-task.h"

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
void read_regs_callback(const char* args);

void rp2040_i2c_read(uint8_t* buffer, uint16_t length);
void rp2040_i2c_write(uint8_t* data, uint16_t size);

api_t device_api[] =
{
	{"help", help_callback, "get help"},
	{"version", version_callback, "get device name and firmware version"},
	{"on", led_on_callback, "turn led on"},
	{"off", led_off_callback, "turn led on"},
	{"blink", led_blink_callback, "set led blink"},
	{"set_period", led_blink_set_period_ms_callback, "set led blink period"},
	{"mem", mem_callback, "show mem"},
	{"wmem", wmem_callback, "write mem"},
	{"read_reg", read_regs_callback, "read regs from bme280"},
	{NULL, NULL, NULL},
};

int main()
{
    stdio_init_all();
	i2c_init(i2c1, 100000);
	
	gpio_set_function(14, GPIO_FUNC_I2C);
	gpio_set_function(15, GPIO_FUNC_I2C);
	gpio_pull_up(14);
	gpio_pull_up(15);
	
	led_task_init();
	stdio_task_init();
	protocol_task_init(device_api);
// ???	
	bme280_init(rp2040_i2c_read, rp2040_i2c_write);
	
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


void read_regs_callback(const char* args)
{
	//вытаскиваем из строки аргуметнов адрес и количество регистров
	uint32_t addr = 0;
		
	sscanf(args, "%x", &addr);
	if (addr >= 0xFF || addr < 0)
		printf("Not valid start reg address\n");
	else 
	{	
		char* space_symbol = strchr(args, ' ');

		if (!space_symbol) //нет второго аргумента
		{
			printf("Not valid regs quantity\n");
		}	
		else
		{
			//*space_symbol = '\0';
			args = space_symbol + 1;
			
			uint32_t quantity = 0;
			sscanf(args, "%u", &quantity);
		//	printf("%u\n", quantity);
			if ((quantity >= 0 && quantity <= 0xFF) && (addr + quantity <= 0x100))
			{
				uint8_t buffer[256] = {0};
				bme280_read_regs(addr, buffer, quantity);
				
				for (int i = 0; i < quantity; i++)
				{
					printf("bme280 register [0x%X] = 0x%X\n", addr + i, buffer[i]);
				}
			}
		}
	}
	
	return;
}

void rp2040_i2c_read(uint8_t* buffer, uint16_t length)
{
	i2c_read_timeout_us(i2c1, 0x76, buffer, length, false, 100000);
	return;
}

void rp2040_i2c_write(uint8_t* data, uint16_t size)
{
	
	int ret = i2c_write_timeout_us(i2c1, 0x76, data, size, false, 100000);
	if (ret == PICO_ERROR_GENERIC)
		printf("PICO_ERROR_GENERIC\n");
	else if (ret == PICO_ERROR_TIMEOUT)
		printf("PICO_ERROR_TIMEOUT\n");
	else 
		printf("%u\n", ret);

	return;
}
