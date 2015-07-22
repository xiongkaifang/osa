#
#	Common headers directory.
#

ifndef $(INCLUDES_MK)
INCLUDES_MK = 1

#
#	Common headers
#

OSA_INC_DIR=-I$(PROJECT_PATH)/osa/include

#
#	Common library.
#
OSA_LIBS=-I$(LIB_DIR)/osa/include

endif # ifndef $(INCLUDES_MK)

