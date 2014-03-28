# vim:set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab 
#
# make all = compile, link and convert to ihex
# make clean = remove files created by make all
# make program = flash the controller with avrdude
#

# controller
MCU = atmega8
#MCU = atmega32
#MCU = attiny2313
#MCU = atmega8535

# frequency
F_CPU = 8000000

# optimization level
OPT = s

# hex format
FORMAT = ihex

# application name + source file where main() is defined
TARGET = echo

# all c files
SRC = $(TARGET).c uart.c

# c language standard
CSTANDARD = c89

# c compiler flags
CFLAGS = -mmcu=$(MCU)
CFLAGS += -O$(OPT)
CFLAGS += -Wall
CFLAGS += -std=$(CSTANDARD)
CFLAGS += -DF_CPU=$(F_CPU)

# avrdude
AVRDUDE_PROGRAMMER = avr910
AVRDUDE_PORT = /dev/ttyUSB0
AVRDUDE_CONFIG = /etc/avrdude.conf
AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex:a

AVRDUDE_FLAGS = -C $(AVRDUDE_CONFIG) -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)

# programs and commands
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude
REMOVE = rm -f

# default target
all: $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET).elf $(SRC)
	$(OBJCOPY) -O $(FORMAT) $(TARGET).elf $(TARGET).hex

# remove created files
clean:
	$(REMOVE) $(TARGET).elf $(TARGET).hex

# program the controller
program: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)

