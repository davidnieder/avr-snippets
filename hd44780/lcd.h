#ifndef LCD_H
#define LCD_H

/* port and data direction register for the 4 data lines.
 * the pins must be next to each other, LCD_DATA_PIN0 denotes
 * the first data line (ie DB4 on the lcd)
 */
#define LCD_DATA_PORT	PORTB
#define LCD_DATA_DDR	DDRB
#define LCD_DATA_PIN0	PB0

/* port, ddr and pin of the register select (RS) line */
#define LCD_RS_PORT		PORTB
#define LCD_RS_DDR		DDRB
#define LCD_RS_PIN		PB4

/* port, ddr and pin of the enable (EN) line */
#define LCD_EN_PORT		PORTB
#define LCD_EN_DDR		DDRB
#define LCD_EN_PIN		PB5


/*
 * initialize hardware; call once before using the functions declared below
 */
void lcd_init();

/*
 * clear the whole display
 *
 * (sends the "Clear Display" command)
 */
void lcd_clear();

/*
 * set the cursor to (0,0)
 *
 * (sends the "Return Home" command)
 */
void lcd_home();

/*
 * set the cursor to (x,y)
 */
void lcd_set_cursor(uint8_t x, uint8_t y);

/*
 * write character 'c' to the current cursor position
 */
void lcd_putc(char c);

/*
 * write the string 'str' to the current cursor position
 */
void lcd_puts(const char *str);

#endif
