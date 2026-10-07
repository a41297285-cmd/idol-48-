# NDS Project Makefile Template

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitpro")
endif

include $(DEVKITARM)/ds_rules

TARGET		:= Idol_Producer_48
BUILD		:= build
SOURCES		:= source
INCLUDES	:= include

ARCH	:= -marm -cpu=arm946e-s -mthumb-interwork
CFLAGS	:= -Wall -O2 -march=armv5te -mtune=arm946e-s -fomit-frame-pointer -ffast-math $(INCLUDE) -DARM9
CXXFLAGS:= $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS	:= -g $(ARCH)
LDFLAGS	= -specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(notdir $(CURDIR)).map
LIBS	:= -lnds9

export OUTPUT	:= $(CURDIR)/$(TARGET)
export VPATH	:= $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR	:= $(CURDIR)/$(BUILD)

CFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
OFILES		:= $(CFILES:.c=.o)

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) -I$(LIBNDS)/include -I$(CURDIR)/$(BUILD)
export LIBPATHS	:= -L$(LIBNDS)/lib

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds



DEPENDS	:= $(OFILES:.o=.d)

$(OUTPUT).nds : $(OUTPUT).elf
$(OUTPUT).elf : $(OFILES)

%.o: %.c
	@echo $(notdir $<)
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@

-include $(DEPENDS)

endif
