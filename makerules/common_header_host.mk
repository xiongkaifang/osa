#
# Common header host makefile.
#

ifndef $(COMMON_HEADER_MK)
COMMON_HEADER_MK = 1

#CC=$(CODEGEN_PREFIX)gcc
#AR=$(CODEGEN_PREFIX)ar
#LD=$(CODEGEN_PREFIX)gcc

CC=gcc
AR=ar
LD=gcc

LIB_BASE_DIR=$(PROJECT_PATH)/lib/$(PLATFORM)
OBJ_BASE_DIR=$(LIB_BASE_DIR)/obj
EXE_BASE_DIR=$(PROJECT_PATH)/bin

ifeq ($(CONFIG),)
LIB_DIR=$(LIB_BASE_DIR)
else
LIB_DIR=$(LIB_BASE_DIR)/$(CONFIG)
endif

CC_OPTS=-c #-Wall -Warray-bounds

ifeq ($(TREAT_WARNINGS_AS_ERROR), yes)

CC_OPTS+= -Werror

endif

ifeq ($(CONFIG), debug)

CC_OPTS+=-g
OPTI_OPTS=
DEFINE=-DDEBUG

else

CC_OPTS+=
OPTI_OPTS=-O3
DEFINE=

endif

AR_OPTS=-rc
LD_OPTS=-lpthread -lm

DEFINE += $(PROJECT_CFLAGS)
FILES=$(subst ./, , $(foreach dir,.,$(wildcard $(dir)/*.c)) ) 

vpath %.a $(LIB_DIR)


include $(PROJECT_PATH)/makerules/includes_host.mk

INCLUDE=
INCLUDE+=$(COMMON_INC)

endif #	ifndef $(COMMON_HEADER_MK)
