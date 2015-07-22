
include $(PROJECT_PATH)/makerules/common_header_host.mk

INCLUDE+= $(OSA_INC_DIR) -I../inc

LIBS= $(LIB_DIR)/osa_timer.a $(LIB_DIR)/osa.a

include $(PROJECT_PATH)/makerules/common_footer_host.mk
