
osa_log.base:
	$(MAKE) -fMAKEFILE.MK libs TARGET=depend
	$(MAKE) -fMAKEFILE.MK libs
	$(MAKE) -fMAKEFILE.MK libs TARGET=exe

osa_log:
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_log/src MODULE=osa_log $(TARGET)

clean:
	$(MAKE) -fMAKEFILE.MK libs TARGET=clean

libs: osa_log

all: clean osa_log

.PHONY: all osa_log.base clean
