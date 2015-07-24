#
#
#

# include global Rules.mak

#
# include local rules.make
#
include Rules.make

##########################################
#                                        #
# Codecs System Top Level Build Targets  #
#                                        #
##########################################

.PHONY : osa clean

osa:
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/osa MODULE=osa

debug_test: osa
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/debug_test MODULE=debug_test

osa_test: osa
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_test MODULE=osa_test

osa_console: osa
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_console MODULE=osa_console

osa_timer: osa
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_timer MODULE=osa_timer

osa_task: osa
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_task MODULE=osa_task

osa_clean:
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/osa MODULE=osa clean

debug_test_clean: osa_clean
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/debug_test MODULE=debug_test clean

osa_test_clean: osa_clean
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_test MODULE=osa_test clean

osa_console_clean: osa_clean
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_console MODULE=osa_console clean

osa_timer_clean: osa_clean
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_timer MODULE=osa_timer clean

osa_task_clean: osa_clean
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_task MODULE=osa_task clean

clean: osa_clean debug_test_clean osa_test_clean osa_console_clean osa_timer_clean osa_task_clean

all: clean osa osa_test debug_test osa_console osa_timer osa_task
