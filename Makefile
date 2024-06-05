#
# Makefile
#
CIRCLEHOME = ../..

#OBJS	= main.o kernel.o
OBJS = main.o kernel.o \
	   z80/cpu.o z80/mcycle.o z80/registers.o z80/special_registers.o \
       z80/bus/bus.o z80/bus/cgpio_bus.o \
       z80/opcode.o


#LIBS	= $(CIRCLEHOME)/app/lib/template/libtempl.a \
#	  $(CIRCLEHOME)/lib/libcircle.a

LIBS = $(CIRCLEHOME)/lib/sched/libsched.a \
		$(CIRCLEHOME)/lib/libcircle.a

include ../Rules.mk

-include $(DEPS)

EXTRACLEAN = z80/bus/*.d z80/bus/*.o z80/*.d z80/*.o

flash:
	flashy /dev/tty.usbserial-FTC7WYB3 flash kernel7l.img
.PHONY: flash

# ctrl + K > a > y to exit
serial:
	screen /dev/tty.usbserial-FTC7WYB3 9600
.PHONY: serial

