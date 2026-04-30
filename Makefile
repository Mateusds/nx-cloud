# Nome do projeto
TARGET := AppSwitch

# Tenta encontrar o devkitPro automaticamente
ifeq ($(strip $(DEVKITPRO)),)
  DEVKITPRO := /opt/devkitpro
endif

# Configura as ferramentas usando as variáveis oficiais
include $(DEVKITPRO)/libnx/switch_rules

# Forçamos o uso do compilador que vem no kit
CXX := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-g++

# Pastas
SOURCE := source
INCLUDE := include
BUILD  := build

# Flags de Compilação
ARCH    := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
CXXFLAGS := -g -Wall -O2 -ffunction-sections $(ARCH) -I$(CURDIR)/$(INCLUDE) -I$(DEVKITPRO)/libnx/include -I$(DEVKITPRO)/portlibs/switch/include
LDFLAGS  := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -L$(DEVKITPRO)/libnx/lib -L$(DEVKITPRO)/portlibs/switch/lib

# Bibliotecas
LIBS := -lcurl -lz -lnx

all: $(TARGET).nro

$(TARGET).nro: $(TARGET).elf

$(TARGET).elf: $(SOURCE)/main.cpp $(SOURCE)/qrcodegen.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SOURCE)/main.cpp $(SOURCE)/qrcodegen.cpp $(LIBS) -o $@

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).nro