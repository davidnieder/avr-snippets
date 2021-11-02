#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

/* commands and their arguments */
#define CMD_CLEAR_DISPLAY		0x01
#define CMD_CURSOR_HOME			0x02

#define CMD_SET_ENTRY			0x04
#define ARG_CURSOR_DECREASE		0x00
#define ARG_CURSOR_INCREASE		0x02
#define ARG_DISPLAY_NOSHIFT		0x00
#define ARG_DISPLAY_SHIFT		0x01

#define CMD_SET_DISPLAY			0x08
#define ARG_DISPLAY_OFF         0x00
#define ARG_DISPLAY_ON          0x04
#define ARG_CURSOR_OFF          0x00
#define ARG_CURSOR_ON           0x02
#define ARG_BLINKING_OFF        0x00
#define ARG_BLINKING_ON         0x01

#define CMD_SET_SHIFT			0x10
#define ARG_NO_SHIFT			0x00
#define ARG_SHIFT				0x08
#define ARG_SHIFT_LEFT			0x00
#define ARG_SHIFT_RIGHT			0x04

#define CMD_SET_FUNCTION		0x20
#define ARG_FUNCTION_4BIT		0x00
#define ARG_FUNCTION_8BIT		0x10
#define ARG_FUNCTION_1LINE      0x00
#define ARG_FUNCTION_2LINE      0x08
#define ARG_FUNCTION_5X7        0x00
#define ARG_FUNCTION_5X10       0x04

#define CMD_RESET				0x30
#define CMD_SET_CGRAM_ADDR		0x40

#define CMD_SET_DDRAM_ADDR		0x80
#define ARG_DDRAM_ADDR_LINE1	0x00
#define ARG_DDRAM_ADDR_LINE2	0x40
#define ARG_DDRAM_ADDR_LINE3	0x10
#define ARG_DDRAM_ADDR_LINE4	0x50

/* lcd execution times */
#define DELAY_POWERON_MS		15
#define DELAY_ENABLE_US			1
#define DELAY_WRITEDATA_US		46
#define DELAY_COMMAND_US		42
#define DELAY_INIT_1_MS			5
#define DELAY_INIT_2_US			120
#define DELAY_SET_4BITMODE_MS	5
#define DELAY_CLEAR_DISPLAY_MS	2
#define DELAY_CURSOR_HOME_MS	2


/* high pulse at the enable pin */
static void
lcd_enable( void )
{
    LCD_EN_PORT |= (1<<LCD_EN_PIN);
    _delay_us(DELAY_ENABLE_US);
    LCD_EN_PORT &= ~(1<<LCD_EN_PIN);
}

/* set the data lines to the lower 4 bits in 'nibble' */
static void
lcd_out(uint8_t nibble)
{
	/* remove the 4 higher bits */
    nibble &= 0x0F;
	/* remove previously written data */
	LCD_DATA_PORT &= ~(0x0F<<LCD_DATA_PIN0);
	/* set data lines */
	LCD_DATA_PORT |= (nibble<<LCD_DATA_PIN0);
	/* enable pulse */
    lcd_enable();
}

/* send a command to the lcd */
void
lcd_command(uint8_t command)
{
	/* select command register */
    LCD_RS_PORT &= ~(1<<LCD_RS_PIN);

	/* send command, high nibble first */
	lcd_out(command>>4);
	lcd_out(command);

    _delay_us(DELAY_COMMAND_US);
}

/* send data byte to the lcd */
void
lcd_data(uint8_t data)
{
	/* select data register */
    LCD_RS_PORT |= (1<<LCD_RS_PIN);

	/* send data, high nibble first */
	lcd_out(data>>4);
	lcd_out(data);

    _delay_us(DELAY_WRITEDATA_US);
}

void
lcd_init()
{
	/* set data, RS, EN pins as output */
	LCD_DATA_DDR |= (0x0F<<LCD_DATA_PIN0);
	LCD_RS_DDR |= (1<<LCD_RS_PIN);
	LCD_EN_DDR |= (1<<LCD_EN_PIN);

	/* set all outputs low */
	LCD_DATA_PORT &= ~(0x0F<<LCD_DATA_PIN0);
	LCD_RS_PORT &= ~(1<<LCD_RS_PIN);
	LCD_EN_PORT &= ~(1<<LCD_EN_PIN);

    /* initializing sequence */
    _delay_ms(DELAY_POWERON_MS);

    lcd_out(CMD_RESET);
    _delay_ms(DELAY_INIT_1_MS);

    lcd_enable();
    _delay_us(DELAY_INIT_2_US);

    lcd_enable();
    _delay_us(DELAY_INIT_2_US);

    /* set to 4-bit mode */
    lcd_out(CMD_SET_FUNCTION | ARG_FUNCTION_4BIT);
    _delay_ms(DELAY_SET_4BITMODE_MS);

    /* set 4-bit mode, 2 rows, 5x7 dot characters */
    lcd_command(CMD_SET_FUNCTION |
			ARG_FUNCTION_4BIT |
			ARG_FUNCTION_2LINE |
			ARG_FUNCTION_5X7);

    /* set display on, cursor off, blinking off */
    lcd_command(CMD_SET_DISPLAY |
			ARG_DISPLAY_ON |
			ARG_CURSOR_OFF |
			ARG_BLINKING_OFF);

    /* set cursor position increasing, no shifting */
    lcd_command(CMD_SET_ENTRY |
			ARG_CURSOR_INCREASE |
			ARG_DISPLAY_NOSHIFT);

    lcd_clear();
}

void
lcd_clear()
{
    lcd_command(CMD_CLEAR_DISPLAY);
    _delay_ms(DELAY_CLEAR_DISPLAY_MS);
}

void
lcd_home()
{
    lcd_command(CMD_CURSOR_HOME);
    _delay_ms(DELAY_CURSOR_HOME_MS);
}

void
lcd_set_cursor(uint8_t x, uint8_t y)
{
    uint8_t command;

    switch (y)
    {
        case 0:
            command = CMD_SET_DDRAM_ADDR + ARG_DDRAM_ADDR_LINE1 + x;
            break;
        case 1:
            command = CMD_SET_DDRAM_ADDR + ARG_DDRAM_ADDR_LINE2 + x;
            break;
        case 2:
            command = CMD_SET_DDRAM_ADDR + ARG_DDRAM_ADDR_LINE3 + x;
            break;
        case 3:
            command = CMD_SET_DDRAM_ADDR + ARG_DDRAM_ADDR_LINE4 + x;
            break;
        default:
            return;
    }

    lcd_command(command);
}

void
lcd_putc(char c)
{
	lcd_data(c);
}

void
lcd_puts(const char *str)
{
    while (*str != '\0')
        lcd_data(*str++);
}
