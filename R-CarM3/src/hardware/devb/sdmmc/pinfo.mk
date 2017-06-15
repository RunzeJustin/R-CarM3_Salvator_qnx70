define PINFO
PINFO DESCRIPTION=sd/mmc disk driver
endef
ifneq ($(wildcard $(PWD)/$(NAME).use),)
USEFILE=$(PWD)/$(NAME).use
endif
PUBLIC_INCVPATH += $(wildcard $(PROJECT_ROOT)/$(SECTION)/public )

NDAS := $(wildcard *.o-nda)

ifeq ($(filter $(basename $(NDAS)), $(basename $(notdir $(SRCS)))),)
# we don't have the NDA source, have to use pre-compiled binaries
SRCS += $(NDAS)
else
# We don't need the following lines if we're willing to manually update
# the *.?-nda files.
#UPDATE_NDA_OBJECTS = 1
#POST_TARGET = $(NDAS)
endif
