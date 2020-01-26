TARGET = main
CC = sdcc
MCU = stm8s103f3

CFLAGS = -mstm8 --opt-code-size

DEFINES = -DSTM8S103

INCLUDES = -I'/home/danya/Projects/mylibs/STM8S_StdPeriph_Driver/inc/' \
 -I'./code/include/'

LIBPATHS = -L'/home/danya/Projects/mylibs/compiled/stm8/'

STATICLIBS = -lstdperiphdriver_stm8s103_osize.lib

SOURCES = \
 code/main.c \
 code/utils.c

HEADERS = \
 code/include/utils.h

OUTPUT_DIR = ./build
OBJ_FILES = $(addprefix $(OUTPUT_DIR)/, $(notdir $(SOURCES:.c=.rel)))

all: $(OUTPUT_DIR) $(OUTPUT_DIR)/$(TARGET).hex

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

%.hex: %.ihx
	packihx $< > $@
	size $@

$(OUTPUT_DIR)/$(TARGET).ihx: $(OBJ_FILES)
	$(CC) $(CFLAGS) $(LIBPATHS) $(STATICLIBS) -o $@ $^

build/%.rel: code/%.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c -o $@ $<

clean:
	rm -Rrf ./build/*

prog: all
	stm8flash -c stlinkv2 -p $(MCU) -w build/$(TARGET).hex

uart:
	gtkterm --port /dev/ttyUSB0 --speed 9600

dbg:
	echo $(OBJ_FILES)
	echo $(SOURCES)
	echo $(HEADERS)
