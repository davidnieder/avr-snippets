# make = compile, link and convert to ihex
# make clean = remove files created by make
# make flash = flash the controller with avrdude

UARTLIB = ../uart
W1LIB = ../w1

# controller
MCU = atmega8
#MCU = atmega32
#MCU = attiny2313
#MCU = atmega8535

# clock frequency
F_CPU = 10000000

UART_BAUD_RATE = 115200

# optimization level
OPT = s

# output file prefix
TARGET = fw

# c language standard
CSTANDARD = c99

# compiler options
CFLAGS = -mmcu=$(MCU)
CFLAGS += -O$(OPT)
CFLAGS += -Wall
CFLAGS += -std=$(CSTANDARD)
CFLAGS += -DF_CPU=$(F_CPU)
CFLAGS += -DUART_BAUD_RATE=$(UART_BAUD_RATE)
CFLAGS += -I$(UARTLIB) -I$(W1LIB)

# linker options
LFLAGS = -mmcu=$(MCU)

# avrdude
AVRDUDE_PROGRAMMER = avr910
AVRDUDE_PORT = /dev/ttyUSB0
AVRDUDE_CONFIG = /etc/avrdude.conf
AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex:a
AVRDUDE_FLAGS = -C $(AVRDUDE_CONFIG) -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)

# programs
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude
RM = rm -f

# compile these files
SRCS = $(wildcard *.c) uart.c w1-master.c
vpath uart.c $(UARTLIB)
vpath w1-master.c $(W1LIB)
# link these files
OBJS = $(patsubst %.c,%.o,$(SRCS))


$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $(TARGET).elf $(TARGET).hex

$(TARGET).elf: $(OBJS)
	$(CC) $(LFLAGS) -o $@ $(OBJS)

%.o: %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

flash: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)

clean:
	$(RM) $(TARGET).elf $(TARGET).hex $(OBJS)

.PHONY: clean
