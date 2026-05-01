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

# Configurações do Aplicativo (Aparecem no Homebrew Menu)
APP_TITLE   := NX-Cloud
APP_AUTHOR  := Mateusds
APP_VERSION := v16
ICON        := icon.jpg

# Flags de Compilação
ARCH    := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
CXXFLAGS := -g -Wall -O2 -ffunction-sections $(ARCH) -I$(CURDIR)/$(INCLUDE) -I$(DEVKITPRO)/libnx/include -I$(DEVKITPRO)/portlibs/switch/include -I$(DEVKITPRO)/portlibs/switch/include/SDL2
LDFLAGS  := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -L$(DEVKITPRO)/libnx/lib -L$(DEVKITPRO)/portlibs/switch/lib

# Bibliotecas
LIBS := -lcurl -lz -lSDL2_image -lSDL2 -lpng -lfreetype -ljpeg -lwebp -lEGL -lglapi -ldrm_nouveau -lm -lz -lpthread -lnx

all: $(TARGET).nro

$(TARGET).nro: $(TARGET).elf
	@# Gera o NRO com ícone e metadados
	$(DEVKITPRO)/tools/bin/elf2nro $< $@ --icon=$(ICON) --app-title="$(APP_TITLE)" --app-author="$(APP_AUTHOR)" --app-version="$(APP_VERSION)"

$(TARGET).elf: $(SOURCE)/main.cpp $(SOURCE)/qrcodegen.cpp $(SOURCE)/graphics.cpp $(SOURCE)/discovery.cpp $(SOURCE)/network.cpp $(SOURCE)/installer.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SOURCE)/main.cpp $(SOURCE)/qrcodegen.cpp $(SOURCE)/graphics.cpp $(SOURCE)/discovery.cpp $(SOURCE)/network.cpp $(SOURCE)/installer.cpp $(LIBS) -o $@

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).nro
