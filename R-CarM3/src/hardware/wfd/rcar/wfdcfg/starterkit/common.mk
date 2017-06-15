ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

# Peripherals
EXTRA_INCVPATH += $(PROJECT_ROOT)/../../public
EXTRA_INCVPATH += $(PROJECT_ROOT)/../../../common

INSTALL_ROOT_nto = $(PROJECT_ROOT)/../../../../../../install
USE_INSTALL_ROOT=1

include $(MKFILES_ROOT)/qmacros.mk
include $(PROJECT_ROOT)/../wfdcfg.mk

include $(MKFILES_ROOT)/qtargets.mk
include $(PROJECT_ROOT)/../usemsg.mk

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib/graphics/R_CarM3$(DBG_DIRSUFFIX))
