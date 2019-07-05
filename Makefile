# Basic Configuration - see also boards.txt for your board's settings
ARCHITECTURE = avr
MCU = atmega328p
BOARD_TAG = nano
BOARD_SUB = atmega328
HEX_MAXIMUM_SIZE = 30720
# Fuse configuration to achieve the proper clock setting
# See: http://www.engbedded.com/fusecalc/
ISP_HIGH_FUSE = 0xd9
ISP_LOW_FUSE = 0xe2
ISP_EXT_FUSE = 0xff
# Resulting frequency of internal oscillator
F_CPU = 8000000L

# A user-defineable path for the Arduino toolchain
USER_ARDUPATH = $(HOME)/.arduino-toolchain
# Force default vendor setting as we do not use Arch Linux AUR package
ARDMK_VENDOR = arduino

# Programmer is USBasp
ISP_PROG := usbasp
ISP_PORT := usb

# Specify additional C(++) flags here
CXXFLAGS += -Wall -Wextra -Woverloaded-virtual -Wstrict-null-sentinel \
    -Wno-error=deprecated-declarations -mrelax -fno-rtti \
    -fshort-enums -fno-threadsafe-statics -fno-permissive -fdiagnostics-color
CFLAGS += -Wall -Wextra \
    -Waddr-space-convert -mrelax -fshort-enums -fdiagnostics-color
LDFLAGS += -Wl,--relax -fdiagnostics-color
# Used C++ standard: C++14 w/o GNU extensions - Arduino headers are based on it
CXXFLAGS_STD = -std=c++11
CFLAGS_STD = -std=c11
# -Os should be the default according to Arduino-mk docs
OPTIMIZATION_LEVEL = s

# Disable inclusion of Arduino sources.
# You may want to disable this to use Arduino libraries.
NO_CORE = y
NO_CORE_MAIN_CPP = y
# Include your own libraries here
USER_LIB_PATH = lib
ARDUINO_LIBS  =

# Be silent by default. This can be overridden if required.
VERBOSE ?= 0
ifeq ($(VERBOSE),0)
    ARDUINO_QUIET ?= 1
    MAKE_VERBOSE ?= 0
endif
ifeq ($(MAKE_VERBOSE),0)
    .SILENT:
endif

# Debug mode is enabled by adding "debug=1" to the command line
ifeq ($(debug),1)
    DEFINES += -DDEBUG
    CXXFLAGS += -g
    CFLAGS += -g
else
    override debug = 0
endif

# Allow user-defined toolchain if exists
ifeq ("$(wildcard $(USER_ARDUPATH))","")
    ARDMK_DIR = /usr/share/arduino
else
    ARDUINO_DIR = $(USER_ARDUPATH)/arduino
    ARDMK_DIR = $(USER_ARDUPATH)/arduino-mk
endif

# Arduino-mk does further autodetection and adds most commands
include $(ARDMK_DIR)/Arduino.mk
