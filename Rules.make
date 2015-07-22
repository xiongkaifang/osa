#
#   Common softwares install directory.
#

ifeq ($(PROJECT_PATH), )

# Default build environment, windows or linux
ifeq ($(OS), )
  OS := linux
endif

PROJECT_RELPATH = osa.base

ifeq ($(OS),Windows_NT)
  PROJECT_BASE     := $(CURDIR)/..
endif

ifeq ($(OS),linux)
  PROJECT_BASE     := $(shell pwd)/..
endif

PROJECT_PATH     := $(PROJECT_BASE)/$(PROJECT_RELPATH)

# Code gen tools
CODEGEN_PATH_HOST:= /home/xiaoxiong/workspace/TI816X/DVRRDK_04.00.00.03/ti_tools/cgt_a8/arago/linux-devkit
CODEGEN_PREFIX   := $(CODEGEN_PATH_HOST)/bin/arm-arago-linux-gnueabi-
CSTOOL_PREFIX    := arm-arago-linux-gnueabi-
ifeq ($(OS),Windows_NT)
CODEGEN_PREFIX   := $(TI_SW_ROOT)/cgt_a8/arm-2009q1/bin/arm-none-linux-gnueabi-
CSTOOL_PREFIX    := arm-none-linux-gnueabi-
endif

# BIOS side tools

# Codecs

# Audio framework (RPE) and Codecs

# Linux side tools

ifeq ($(CORE), )
  CORE := host
endif

# Default platform
ifeq ($(PLATFORM), )
  PLATFORM := linux
endif

# Default profile
ifeq ($(PROFILE_m3video), )
  PROFILE_m3video := release
#  PROFILE_m3video := debug
endif

# Default configuration
ifeq ($(CONFIG), )
  CONFIG := debug
endif

endif

TREAT_WARNINGS_AS_ERROR=no

#include $(ROOTDIR)/makerules/build_config.mk
#include $(ROOTDIR)/makerules/env.mk
#include $(ROOTDIR)/makerules/platform.mk
#include $(dvr_rdk_PATH)/component.mk

#
#   Export global environment variables.
#
export OS
export PLATFORM
export CORE
export PROJECT_PATH
export TREAT_WARNINGS_AS_ERROR
export CODEGEN_PREFIX
export CONFIG
