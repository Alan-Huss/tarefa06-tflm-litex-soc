# TFLM firmware for LiteX/VexRiscv
# Uses pre-built TFLM library

# Directories
# IMPORTANTE: Confirme se sua pasta de build é colorlight_i5 ou i9
BUILD_DIR = ../build/colorlight_i5
OUT = build
TFLM = tflm

# LiteX configuration
include $(BUILD_DIR)/software/include/generated/variables.mak
include $(SOC_DIRECTORY)/software/common.mak

# --- TOOLCHAIN FIX ---
RISCV_PATH = $(CURDIR)/../tools/xpack-riscv-none-elf-gcc-15.2.0-1/bin

RISCV_PREFIX = $(RISCV_PATH)/riscv-none-elf-

CC      = $(RISCV_PREFIX)gcc
CXX     = $(RISCV_PREFIX)g++
OBJCOPY = $(RISCV_PREFIX)objcopy
SIZE    = $(RISCV_PREFIX)size

# --- FIX DO LINKER (CRÍTICO) ---
# Adiciona a flag de segurança diretamente nas flags do Linker e do Compilador C
LDFLAGS += -fno-threadsafe-statics
CFLAGS += -fno-threadsafe-statics

# TFLM defines
TFLM_DEFINES = -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON
TFLM_DEFINES += -DGEMMLOWP_ALLOW_SLOW_SCALAR_FALLBACK
TFLM_DEFINES += -DTF_LITE_USE_GLOBAL_CMATH_FUNCTIONS
TFLM_DEFINES += -DTF_LITE_USE_GLOBAL_MAX -DTF_LITE_USE_GLOBAL_MIN

# TFLM includes
TFLM_INCLUDES = -I$(TFLM) -I$(TFLM)/third_party/flatbuffers/include
TFLM_INCLUDES += -I$(TFLM)/third_party/gemmlowp -I$(TFLM)/third_party/ruy

# Application flags
APP_FLAGS = $(CPUFLAGS) -Os -g -std=c++17
APP_FLAGS += -fno-rtti -fno-exceptions -ffunction-sections -fdata-sections -Wall
APP_FLAGS += -fno-threadsafe-statics
APP_FLAGS += -I. -I$(BUILDINC_DIRECTORY) -I$(BUILDINC_DIRECTORY)/../libc
APP_FLAGS += -I$(SOC_DIRECTORY)/software/include -I$(SOC_DIRECTORY)/software/libbase
APP_FLAGS += -I$(CPU_DIRECTORY) -Iplatform
APP_FLAGS += $(TFLM_INCLUDES) $(TFLM_DEFINES)

# Sources
# --- FIX DO MODELO ---
# Mudamos de int8 para float para satisfazer o main.cc
APP_SOURCES = main.cc models/hello_world_float_model_data.cc

# Objects
PLATFORM_OBJECTS = $(patsubst %.cc,$(OUT)/%.o,$(PLATFORM_SOURCES))
APP_OBJECTS = $(patsubst %.cc,$(OUT)/%.o,$(APP_SOURCES))
OBJECTS = $(OUT)/crt0.o $(APP_OBJECTS) $(PLATFORM_OBJECTS)

# Pre-built TFLM library
TFLM_LIB = $(TFLM)/build/libtflm.a

# Build
all: $(OUT)/main.bin

$(OUT)/main.bin: $(OUT)/main.elf
	$(OBJCOPY) -O binary $< $@
	@echo "Firmware ready: $@"

$(OUT)/main.elf: $(OBJECTS) $(TFLM_LIB)
	$(CC) $(LDFLAGS) -T linker.ld -N \
		-Wl,--start-group $^ \
		$(PACKAGES:%=-L$(BUILD_DIR)/software/%) \
		$(LIBS:lib%=-l%) -lgcc \
		-Wl,--end-group \
		-Wl,--gc-sections -Wl,--allow-multiple-definition \
		-Wl,-Map,$@.map -o $@
	@echo "Firmware size:"
	@$(SIZE) $@

$(OUT)/platform/%.o: platform/%.cc
	@mkdir -p $(dir $@)
	$(CXX) $(APP_FLAGS) -c $< -o $@

$(OUT)/%.o: %.cc
	@mkdir -p $(dir $@)
	$(CXX) $(APP_FLAGS) -c $< -o $@

$(OUT)/crt0.o: $(CPU_DIRECTORY)/crt0.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OUT)

.PHONY: all clean
