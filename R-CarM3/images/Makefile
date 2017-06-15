ROOT_DIR := $(notdir $(CURDIR))
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

HOST_MKIFS := mkifs

SUFFIXES := .build .ifs .bin

.PHONY: all clean

all: ifs-rcarm3-starterkit.bin ifs-rcarm3-salvatorx.bin flashimage

ifs-rcarm3-starterkit.bin: starterkit.build
	$(HOST_MKIFS) -vvv -r../install $(MKIFSFLAGS) $^ $@

ifs-rcarm3-salvatorx.bin: salvatorx.build
	$(HOST_MKIFS) -vvv -r../install $(MKIFSFLAGS) $^ $@

flashimage:
	$(SHELL) ./mkflashimage
	
clean:
	$(RM_HOST) *.bin *.build
