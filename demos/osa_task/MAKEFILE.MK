
osa_task.base:
	$(MAKE) -fMAKEFILE.MK libs TARGET=depend
	$(MAKE) -fMAKEFILE.MK libs
	$(MAKE) -fMAKEFILE.MK libs TARGET=exe

osa_task:
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_task/src MODULE=osa_task $(TARGET)

clean:
	$(MAKE) -fMAKEFILE.MK libs TARGET=clean

libs: osa_task

all: clean osa_task

.PHONY: all osa_task.base clean
