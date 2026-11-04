#---------------------------------------------------------------------------------
# Space Cadet Pinball for PS3. Build inside the toolchain container: tools/build.ps1.
#
# The real build runs in a sub-make inside build/. Bootable output is
# build/$(TARGET).elf: only that copy goes through sprxlinker.
#---------------------------------------------------------------------------------
.SUFFIXES:
ifeq ($(strip $(PSL1GHT)),)
$(error "PSL1GHT is not set. Build through tools/build.ps1.")
endif

include $(PSL1GHT)/ppu_rules

TARGET    := spacecadet
BUILD     := build
SOURCES   := source platform shaders
TITLE     := 3D Pinball Space Cadet
APPID     := PNBL00001
CONTENTID := UP0001-$(APPID)_00-0000000000000000
LIBS      := -lvorbisfile -lvorbis -logg -lrsx -lgcm_sys -lio -laudio -lsysutil -lrt -llv2 -lm

# BIG_ENDIAN selects the byte swaps the Wii port added to the data loaders.
DEFINES   := -DBIG_ENDIAN=1
CFLAGS    = -O2 -Wall -mcpu=cell -std=gnu99 $(DEFINES) $(MACHDEP) $(INCLUDE)
CXXFLAGS  = -O2 -Wall -Wno-unused-variable -Wno-sign-compare -mcpu=cell -std=gnu++17 $(DEFINES) $(MACHDEP) $(INCLUDE)
LDFLAGS   = $(MACHDEP) -Wl,-Map,$(notdir $@).map

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT    := $(CURDIR)/$(TARGET)
export VPATH     := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR   := $(CURDIR)/$(BUILD)
export BUILDDIR  := $(CURDIR)/$(BUILD)

CFILES           := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES         := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
VCGFILES         := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.vcg)))
FCGFILES         := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.fcg)))

export LD        := $(CXX)
export OFILES    := $(addsuffix .o,$(VCGFILES:.vcg=.vpo)) \
                    $(addsuffix .o,$(FCGFILES:.fcg=.fpo)) \
                    $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

export INCLUDE   := -I$(PORTLIBS)/include \
                    $(foreach dir,$(SOURCES),-I$(CURDIR)/$(dir)) \
                    $(LIBPSL1GHT_INC) \
                    -I$(CURDIR)/$(BUILD)

export LIBPATHS  := -L$(PORTLIBS)/lib $(LIBPSL1GHT_LIB)

# The PKG holds the game executable and an empty Data folder: the game's own files never go in it.
PKGFILES  := $(CURDIR)/pkgfiles

.PHONY: $(BUILD) clean pkg

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

pkg: $(BUILD) $(OUTPUT).pkg

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).self $(TARGET).fake.self $(TARGET).pkg

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).self: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

# The shader headers must exist before any source that includes them compiles.
$(CFILES:.c=.o) $(CPPFILES:.cpp=.o): $(VCGFILES:.vcg=_vpo.h) $(FCGFILES:.fcg=_fpo.h)

%.vpo.o %_vpo.h: %.vpo
	@$(bin2o)

%.fpo.o %_fpo.h: %.fpo
	@$(bin2o)

-include $(DEPENDS)

endif
