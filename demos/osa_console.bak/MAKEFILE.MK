
osa_console.base:
	$(MAKE) -fMAKEFILE.MK libs TARGET=depend
	$(MAKE) -fMAKEFILE.MK libs
	$(MAKE) -fMAKEFILE.MK libs TARGET=exe

osa_console:
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_console/src MODULE=osa_console $(TARGET)

clean:
	$(MAKE) -fMAKEFILE.MK libs TARGET=clean

libs: osa_console

all: clean osa_console

.PHONY: all osa_console.base clean
