#
# make:       compile, link and convert to ihex
# make clean: remove files created by this makefile
# make flash: flash the controller with avrdude
# make size:  show size information for created binary
#

# snippets directory
SNIPPETSDIR = ..

# controller
MCU = atmega8

# clock frequency
F_CPU = 10000000

# optimization level
OPT = s

# target name
TARGET = example

# all sources the compiler will use
SRCS = $(TARGET).c w1-master.c uart.c
vpath uart.c $(SNIPPETSDIR)/uart

# all objects the linker will use
OBJS = $(SRCS:.c=.o)
# vpath obj.o $(SNIPPETSDIR)/

# c language standard
CSTANDARD = c99

# compiler flags
CFLAGS = -mmcu=$(MCU)
CFLAGS += -O$(OPT)
CFLAGS += -Wall
CFLAGS += -std=$(CSTANDARD)
CFLAGS += -DF_CPU=$(F_CPU)
CFLAGS += -I. -I$(SNIPPETSDIR)

# linker flags
LFLAGS = -mmcu=$(MCU)
LFLAGS += -lm

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
AVRSIZE = avr-size
RM = rm -f

# default target
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $(TARGET).elf $(TARGET).hex

$(TARGET).elf: $(OBJS)
	$(CC) $(LFLAGS) -o $@ $(OBJS)

%.o: %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

# remove created files
clean:
	$(RM) $(TARGET).elf $(TARGET).hex $(OBJS)

# program the controller
flash: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)

size: $(TARGET).elf
	$(AVRSIZE) -C -mcu=$(MCU) $(TARGET).elf

.PHONY : clean flash
