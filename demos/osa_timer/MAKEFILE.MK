
osa_timer.base:
	$(MAKE) -fMAKEFILE.MK libs TARGET=depend
	$(MAKE) -fMAKEFILE.MK libs
	$(MAKE) -fMAKEFILE.MK libs TARGET=exe

osa_timer:
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_timer/src MODULE=osa_timer $(TARGET)

clean:
	$(MAKE) -fMAKEFILE.MK libs TARGET=clean

libs: osa_timer

all: clean osa_timer

.PHONY: all osa_timer.base clean
