# Name: Makefile
# Project: Monster Joysticks DE9 to USB Joystick Adapter V1
# Author: Monster Joysticks Ltd. <info@monsterjoysticks.com>
# Copyright: (C) 2017 - 2019 Monster Joysticks Ltd. <info@monsterjoysticks.com>

COMPILE = avr-gcc -Wall -Os -Iusbdrv -I. -mmcu=atmega8 -DF_CPU=12000000L
COMMON_OBJS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o main.o
HEXFILE=bin/main.hex

DEVICE = atmega8
F_CPU = 12000000
FUSEH = 0xc0
FUSEL = 0x9f

AVRDUDE = avrdude -c USBasp -P avrdoper -p $(DEVICE)

OBJS = $(COMMON_OBJS) db9.o devdesc.o

# symbolic targets:
all:	$(HEXFILE)

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:bin/main.hex:i

fuse:
	$(AVRDUDE) -U hfuse:w:$(FUSEH):m -U lfuse:w:$(FUSEL):m

lock:
	$(AVRDUDE) -U lock:w:0x2f:m

clean:
	rm -f $(HEXFILE) main.lst main.obj main.cof main.list bin/main.map bin/main.eep.hex bin/main.bin *.o usbdrv/*.o main.s usbdrv/oddebug.s usbdrv/usbdrv.s

# file targets:
main.bin:	$(OBJS)	db9.o devdesc.o 
	$(COMPILE) -o bin/main.bin $(OBJS) -Wl,-Map=bin/main.map

$(HEXFILE):	main.bin
	rm -f $(HEXFILE) main.eep.hex
	avr-objcopy -j .text -j .data -O ihex bin/main.bin $(HEXFILE)