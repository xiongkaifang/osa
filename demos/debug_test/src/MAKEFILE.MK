
include $(PROJECT_PATH)/makerules/common_header_host.mk

INCLUDE+= $(OSA_INC_DIR)

LIBS= $(LIB_DIR)/debug_test.a $(LIB_DIR)/osa.a

include $(PROJECT_PATH)/makerules/common_footer_host.mk
