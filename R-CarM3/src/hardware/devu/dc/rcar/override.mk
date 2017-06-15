TYPE = $(firstword $(filter a o dll, $(VARIANTS)) o)
BOARD = $(firstword $(filter otg3 hsusb, $(VARIANTS)))
USEFILE_dll = $(SECTION_ROOT)/$(BOARD)/$(PROTOCOL)/$(NAME)-$(PROTOCOL)-$(BOARD).use
USEFILE   = $(USEFILE_$(TYPE))
USBDC_DLL_NAME = \"$(NAME)-$(PROTOCOL)-$(BOARD)-rcar.so\"
CCFLAGS  += -DUSBDC_DLL_NAME=$(USBDC_DLL_NAME)
