
######################################
# target
######################################
TARGET ?= py32-template

# CHIP = PY32F002Ax5
# CHIP = PY32F030x4
# CHIP = PY32F030x6
# CHIP = PY32F030x7
CHIP = PY32F030x8
# CHIP = PY32F003x4
# CHIP = PY32F003x6
# CHIP = PY32F003x8
# CHIP = PY32F072xB
# CHIP = PY32F002Ax5


######################################
# source
######################################
# C sources
C_SOURCES += src/main.c
C_SOURCES += src/lcd.c
C_SOURCES += src/adc.c
C_SOURCES += src/console.c
C_SOURCES += src/nor.c
C_SOURCES += CMSIS/Device/PY32F0xx/Source/system_py32f0xx.c 
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_adc_ex.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_adc.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_adc_ex.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_comp.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_cortex.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_crc.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_dma.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_exti.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_flash.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_gpio.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_i2c.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_irda.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_iwdg.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_led.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_lptim.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_pwr.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_rcc.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_rcc_ex.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_rtc.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_rtc_ex.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_spi.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_tim.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_tim_ex.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_uart.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_usart.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_hal_wwdg.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_adc.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_comp.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_crc.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_dma.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_exti.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_gpio.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_i2c.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_led.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_lptim.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_pwr.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_rcc.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_rtc.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_spi.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_tim.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_usart.c
C_SOURCES += PY32F0xx_HAL_Driver/Src/py32f0xx_ll_utils.c


# ASM sources
ASM_SOURCES =  \
CMSIS/Device/PY32F0xx/Source/gcc/startup_py32f030.s

# C includes
C_INCLUDES =  \
-Iinc \
-ICMSIS/Include \
-ICMSIS/Device/PY32F0xx/Include \
-IPY32F0xx_HAL_Driver/Inc \


# link script
LDSCRIPT = CMSIS/Device/PY32F0xx/Source/gcc/py32f030xx8.ld


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# paths
#######################################

# Build path
BUILD_DIR = build


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m0plus

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS = -D__SOFTFP__ -D${CHIP} -DUSE_HAL_DRIVER


# AS includes
AS_INCLUDES = 


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -Wno-maybe-uninitialized -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################


# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -Wl,--print-memory-usage

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	del /s /q $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
