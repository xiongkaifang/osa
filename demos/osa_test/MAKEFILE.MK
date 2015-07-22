
osa_test.base:
	$(MAKE) -fMAKEFILE.MK libs TARGET=depend
	$(MAKE) -fMAKEFILE.MK libs
	$(MAKE) -fMAKEFILE.MK libs TARGET=exe

osa_test:
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_test/src MODULE=osa_test $(TARGET)

clean:
	$(MAKE) -fMAKEFILE.MK libs TARGET=clean

libs: osa_test

all: clean osa_test

.PHONY: all osa_test.base clean
