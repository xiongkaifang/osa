
osa.base:
	$(MAKE) -fMAKEFILE.MK libs TARGET=depend
	$(MAKE) -fMAKEFILE.MK libs
	$(MAKE) -fMAKEFILE.MK libs TARGET=so

osa:
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/osa/src MODULE=osa $(TARGET)

clean:
	$(MAKE) -fMAKEFILE.MK libs TARGET=clean

libs: osa


all: clean osa.base

.PHONY: all osa.base osa clean libs
