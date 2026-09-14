#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"

#include "ili9341-driver.h"
#include "ili9341-display.h"
#include "ili9341-font.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

#define ILI9341_PIN_MISO 4
#define ILI9341_PIN_CS 10
#define ILI9341_PIN_SCK 6
#define ILI9341_PIN_MOSI 7
#define ILI9341_PIN_DC 8
#define ILI9341_PIN_RESET 9
// #define PIN_LED -> 3.3V

void version_callback(const char* args);
void help_callback(const char* args);
void led_on_callback(const char* args);
void led_off_callback(const char* args);
void led_blink_callback(const char* args);
void led_blink_set_period_ms_callback(const char* args);
void mem_callback(const char* args);
void wmem_callback(const char* args);
void disp_screen_callback(const char* args);
void disp_px_callback(const char* args);
void disp_line_callback(const char* args);
void disp_rect_callback(const char* args);
void disp_frect_callback(const char* args);
void disp_text_callback(const char* args);

void rp2040_spi_write(const uint8_t *data, uint32_t size);
void rp2040_spi_read(uint8_t *buffer, uint32_t length);
void rp2040_gpio_cs_write(bool level);
void rp2040_gpio_dc_write(bool level);
void rp2040_gpio_reset_write(bool level);
void rp2040_delay_ms(uint32_t ms);

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
	{"disp_screen", disp_screen_callback, "show screen of chosen colour"},	
	{"disp_px", disp_px_callback, "show pixel (x,y)of chosen colour"},	
	{"disp_line", disp_line_callback, "show line from (x1,y1) to (x2,y2) of chosen colour"},	
	{"disp_rect", disp_rect_callback, "show rectangle from (x1,y1) of chosen width and height of chosen line colour"},	
	{"disp_frect", disp_frect_callback, "show rectangle from (x1,y1) of chosen width and height filled with chosen colour"},
	{"disp_text", disp_text_callback, "show text starting on chosen place of chosen colour with chosen background colour"},	
	{NULL, NULL, NULL},
};

// контекст драйвера дисплея
static ili9341_display_t ili9341_display = {0};

int main()
{
    stdio_init_all();
	
	stdio_task_init();
	protocol_task_init(device_api);
	led_task_init();
	
	spi_init(spi0, 62500000);
	
	gpio_init(ILI9341_PIN_MISO);
	gpio_init(ILI9341_PIN_MOSI);
	gpio_init(ILI9341_PIN_SCK);
	gpio_set_function(ILI9341_PIN_MISO, GPIO_FUNC_SPI);
	gpio_set_function(ILI9341_PIN_MOSI, GPIO_FUNC_SPI);
	gpio_set_function(ILI9341_PIN_SCK, GPIO_FUNC_SPI);
	
	gpio_init(ILI9341_PIN_CS);
	gpio_init(ILI9341_PIN_DC);
	gpio_init(ILI9341_PIN_RESET);
    gpio_set_dir(ILI9341_PIN_CS, GPIO_OUT);
	gpio_set_dir(ILI9341_PIN_DC, GPIO_OUT);
	gpio_set_dir(ILI9341_PIN_RESET, GPIO_OUT);
	
	gpio_put(ILI9341_PIN_CS, true);
	gpio_put(ILI9341_PIN_DC, false);
	gpio_put(ILI9341_PIN_RESET, false);
	
	ili9341_hal_t ili9341_hal = {0};
	ili9341_hal.spi_write = rp2040_spi_write;
	ili9341_hal.spi_read = rp2040_spi_read;
	ili9341_hal.gpio_cs_write = rp2040_gpio_cs_write;
	ili9341_hal.gpio_dc_write = rp2040_gpio_dc_write;
	ili9341_hal.gpio_reset_write = rp2040_gpio_reset_write;
	ili9341_hal.delay_ms = rp2040_delay_ms;
	
	ili9341_init(&ili9341_display, &ili9341_hal);
	ili9341_set_rotation(&ili9341_display, ILI9341_ROTATION_90);
	
	ili9341_fill_screen(&ili9341_display, COLOR_BLACK);
	sleep_ms(300);
	/* 2. Coloured rectangles */
	ili9341_draw_filled_rect(&ili9341_display, 10, 10, 100, 60, COLOR_RED);
	ili9341_draw_filled_rect(&ili9341_display, 120, 10, 100, 60, COLOR_GREEN);
	ili9341_draw_filled_rect(&ili9341_display, 230, 10, 80, 60, COLOR_BLUE);
	/* 3. Hollow rectangle outline */
	ili9341_draw_rect(&ili9341_display, 10, 90, 300, 80, COLOR_WHITE);

	/* 4. Diagonal lines */
	ili9341_draw_line(&ili9341_display, 0, 0, 319, 239, COLOR_YELLOW);
	ili9341_draw_line(&ili9341_display, 319, 0, 0, 239, COLOR_CYAN);

	ili9341_draw_text(&ili9341_display, 20, 100, "Hello, ILI9341!", &jetbrains_font, COLOR_WHITE, COLOR_BLACK);

	ili9341_draw_text(&ili9341_display, 20, 116, "RP2040 / Pico SDK", &jetbrains_font, COLOR_YELLOW, COLOR_BLACK);

    while (1)
    {
		stdio_task_handle();
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

void disp_screen_callback(const char* args)
{
	uint32_t c = 0;
	int result = sscanf(args, "%x", &c);
	
	uint16_t color = COLOR_BLACK;
	
	if (result == 1)
	{
		color = RGB888_2_RGB565(c);
	}
	
	ili9341_fill_screen(&ili9341_display, color);
}

void disp_px_callback(const char* args)
{
	uint32_t c = 0;
	uint32_t x = 0;
	uint32_t y = 0;
	int result = sscanf(args, "%d %d %x", &x, &y, &c);

	uint16_t color = COLOR_BLACK;
	
	if (result == 3)
	{
		color = RGB888_2_RGB565(c);
	}
	
	ili9341_draw_pixel(&ili9341_display, x, y, color);
}

void disp_line_callback(const char* args)
{
	uint32_t c = 0;
	uint32_t x1 = 0;
	uint32_t y1 = 0;
	uint32_t x2 = 0;
	uint32_t y2 = 0;

	int result = sscanf(args, "%d %d %d %d %x", &x1, &y1, &x2, &y2, &c);
	uint16_t color = COLOR_BLACK;
	
	if (result == 5)
	{
		color = RGB888_2_RGB565(c);
	}
	
	ili9341_draw_line(&ili9341_display, x1, y1, x2, y2, color);
}

void disp_rect_callback(const char* args)
{
	uint32_t c = 0;
	uint32_t x1 = 0;
	uint32_t y1 = 0;
	uint32_t x2 = 0;
	uint32_t y2 = 0;

	int result = sscanf(args, "%d %d %d %d %x", &x1, &y1, &x2, &y2, &c);

	uint16_t color = COLOR_BLACK;
	
	if (result == 5)
	{
		color = RGB888_2_RGB565(c);
	}
	
	ili9341_draw_rect(&ili9341_display, x1, y1, x2, y2, color);
}

void disp_frect_callback(const char* args)
{
	uint32_t c = 0;
	uint32_t x1 = 0;
	uint32_t y1 = 0;
	uint32_t x2 = 0;
	uint32_t y2 = 0;

	int result = sscanf(args, "%d %d %d %d %x", &x1, &y1, &x2, &y2, &c);

	uint16_t color = COLOR_BLACK;
	
	if (result == 5)
	{
		color = RGB888_2_RGB565(c);
	}
	
	ili9341_draw_filled_rect(&ili9341_display, x1, y1, x2, y2, color);
}

void disp_text_callback(const char* args)
{
	uint16_t color_txt = COLOR_BLACK;
	uint16_t color_bg = COLOR_WHITE;
	uint16_t start_x = 100;	
	uint16_t start_y  = 100;	
	char text[255];
		
	uint32_t c_bg = 0;
	uint32_t c_txt = 0;
	uint32_t x = 0;
	uint32_t y = 0;
	char txt[255];
// считать сначала текст, если он есть - читать остальное
	int result = sscanf(args, "%s %d %d %x %x", txt, &x, &y, &c_txt, &c_bg);

	if (result >= 1)
	{
		strcpy(text, txt);
		printf("%s\n", txt);
		printf("%s\n", text);
	}	
	else  strcpy(text, "No text provided");
		
	if (result >=3)
	{
		start_x = x;
		start_y = y;
	}	
	
	if (result >= 4)
	{
		color_txt = RGB888_2_RGB565(c_txt);
	}
	
	if (result >= 5)
	{
		color_bg = RGB888_2_RGB565(c_bg);
	}

	ili9341_draw_text(&ili9341_display, start_x, start_y, text, &jetbrains_font, color_txt, color_bg);
}


void rp2040_spi_write(const uint8_t *data, uint32_t size)
{
	spi_write_blocking(spi0, data, size);
}

void rp2040_spi_read(uint8_t *buffer, uint32_t length)
{
	spi_read_blocking(spi0, 0, buffer, length);
}

void rp2040_gpio_cs_write(bool level)
{
	gpio_put(ILI9341_PIN_CS, level);
}

void rp2040_gpio_dc_write(bool level)
{
	gpio_put(ILI9341_PIN_DC, level);
}

void rp2040_gpio_reset_write(bool level)
{
	gpio_put(ILI9341_PIN_RESET, level);
}

void rp2040_delay_ms(uint32_t ms)
{
	sleep_ms(ms);
}