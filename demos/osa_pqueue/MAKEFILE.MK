
osa_pqueue.base:
	$(MAKE) -fMAKEFILE.MK libs TARGET=depend
	$(MAKE) -fMAKEFILE.MK libs
	$(MAKE) -fMAKEFILE.MK libs TARGET=exe

osa_pqueue:
	$(MAKE) -fMAKEFILE.MK -C$(PROJECT_PATH)/demos/osa_pqueue/src MODULE=osa_pqueue $(TARGET)

clean:
	$(MAKE) -fMAKEFILE.MK libs TARGET=clean

libs: osa_pqueue

all: clean osa_pqueue

.PHONY: all osa_pqueue.base clean
