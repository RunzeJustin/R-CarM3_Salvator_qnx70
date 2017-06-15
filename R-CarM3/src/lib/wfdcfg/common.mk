ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

include $(MKFILES_ROOT)/qmacros.mk

IMAGE_PREF_DLL=$(IMAGE_PREF_SO)
SONAME_DLL=$(IMAGE_PREF_SO)wfdcfg$(IMAGE_SUFF_SO).0

ifeq ($(filter stub,$(VARIANT_LIST)),stub)
# Install the stub into usr/lib instead of lib/dll
# so WFD drivers can link against it without using EXTRA_LIBVPATH.
INSTALLDIR=usr/lib
DESC=Stub library used for linking WFD drivers
else
DESC=Returns a list of display timings and extensions to be used by a WFD driver
endif

INSTALL_ROOT_nto = $(PROJECT_ROOT)/../../../install
USE_INSTALL_ROOT=1

define PINFO
PINFO DESCRIPTION = "$(DESC)"
endef

include $(MKFILES_ROOT)/qtargets.mk
