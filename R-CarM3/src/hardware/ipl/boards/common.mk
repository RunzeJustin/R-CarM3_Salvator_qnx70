ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)


INSTALLDIR = boot/sys

NAME = ipl-$(SECTION)
EXTRA_SILENT_VARIANTS+=$(subst -, ,$(SECTION))
USEFILE = 

## DEFFILE = $(LIBIPL_ROOT)/$(CPU)/$(LIB_VARIANT)/asmoff.def 

LDF_ += -nostartfiles -T$(PROJECT_ROOT)/$(SECTION)/$(SECTION).lnk
LDF_qcc += -M -nostartup -Wl,-T$(PROJECT_ROOT)/$(SECTION)/$(SECTION).lnk
LDFLAGS += -nostdlib $(LDF_$(COMPILER_DRIVER)) -Wl,--build-id=none


include $(MKFILES_ROOT)/qmacros.mk

-include $(PROJECT_ROOT)/roots.mk
   INSTALL_ROOT_nto = $(PROJECT_ROOT)/../../../../install
   USE_INSTALL_ROOT=1


include $(PROJECT_ROOT)/$(SECTION)/pinfo.mk
# Don't try to add usage messages to ipl's.
define ADD_USAGE
endef

#
# Make sure *_reset.o gets linked _first_.
#
old_OBJS := $(OBJS)
OBJS := $(filter %_reset.o, $(old_OBJS)) $(filter-out %_reset.o, $(old_OBJS))

include $(MKFILES_ROOT)/qtargets.mk

GCCVER:= $(if $(GCC_VERSION),$(GCC_VERSION), $(shell qcc -V 2>&1 | grep default | sed -e 's/,.*//'))

ifneq ($(filter 4.4.%, $(strip $(GCCVER))),)
       CCFLAGS+=-fno-dwarf2-cfi-asm 
endif

CCF_gcc_ = -O2 -fomit-frame-pointer
CCF_gcc_qcc = -O2 -Wc,-fomit-frame-pointer
CCFLAGS += $(CCFLAGS_$(COMPILER_TYPE)_$(COMPILER_DRIVER)) -fno-stack-protector
OPTIMIZE_SIZE_gcc=-O0

		
#
# This particular little kludge is to stop GCC from using F.P. instructions
# to move 8 byte quantities around. 
#
CC_nto_ppc_gcc += -msoft-float
CC_nto_ppc_gcc_qcc += -Wc,-msoft-float



