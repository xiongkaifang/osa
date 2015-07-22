
debug_test.base:
	$(MAKE) -fMAKEFILE.MK libs TARGET=depend
	$(MAKE) -fMAKEFILE.MK libs
	$(MAKE) -fMAKEFILE.MK libs TARGET=exe

debug_test:
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/debug_test/src MODULE=debug_test $(TARGET)

clean:
	$(MAKE) -fMAKEFILE.MK libs TARGET=clean

libs: debug_test

all: clean debug_test

.PHONY: all debug_test.base clean
