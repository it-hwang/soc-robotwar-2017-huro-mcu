#

CROSS_COMPILE 	= ae32000-elf-uclibc-

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP	= $(CROSS_COMPILE)objdump
RANLIB	= $(CROSS_COMPILE)ranlib
ELF2FLT	= $(CROSS_COMPILE)elf2flt

CFLAGS	= -O2 -Wall -Wstrict-prototypes -Dlinux -D__linux__ -Dunix -D__uClinux__ -DEMBED -fshort-enums -std=gnu99

COMPILER_PREFIX=/usr/local/ae32000-elf-uclibc-tools
LDFLAGS = -r -Xlinker -T$(COMPILER_PREFIX)/lib/ae32000-elf2flt.ld
LDFLAGS_DBG = -Xlinker -T$(COMPILER_PREFIX)/lib/ae32000-elf2flt.ld

LDLIBS	= -lm -lc 


export AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP RANLIB CFLAGS

PROJ = main


OBJS = main.o log.o terminal.o uart_api.o robot_protocol.o graphic_api.o graphic_interface.o amazon2_timer_api.o timer.o matrix.o vector3.o lut.o color.o screenio.o obstacle_manager.o white_balance.o image_filter.o object_detection.o polygon_detection.o line_detection.o camera.o check_center.o check_bridge_center.o vertical_barricade.o red_bridge.o mine.o corner_detection.o boundary.o blue_gate.o green_bridge.o golf.o processor.o


all: $(PROJ)

$(PROJ): $(OBJS)
	$(CC) $(LDFLAGS) -o $@.elf $^ $(LDLIBS)
	$(CC) $(LDFLAGS_DBG) -o $@.dbg $^ $(LDLIBS)
	$(ELF2FLT) $@.elf
	mv $@.elf.bflt $@
	#${OBJDUMP} -Dtx $@.elf > $@.elf.dis
	#${OBJDUMP} -Dtx $@.dbg > $@.dbg.dis

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
	
.s.o:
	$(AS) $< -o $@
	
clean:
	rm -f *.o *.elf *.bflt *.dis $(TEST) *.dbg main


