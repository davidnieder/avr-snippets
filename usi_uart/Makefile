# make = compile, link and convert to ihex
# make clean = remove files created by make
# make flash = flash the controller with avrdude

# controller
MCU = attiny45

# clock frequency
F_CPU = 8000000

# optimization level
OPT = s

# output file prefix
TARGET = echo_example

# c language standard
CSTANDARD = c99

# compiler options
CFLAGS = -mmcu=$(MCU)
CFLAGS += -O$(OPT)
CFLAGS += -Wall
CFLAGS += -std=$(CSTANDARD)
CFLAGS += -DF_CPU=$(F_CPU)

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
SRCS = $(wildcard *.c)
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
