CFLAGS = $(shell rtai-config --lxrt-cflags)
LDFLAGS = $(shell rtai-config --lxrt-ldflags) -lpthread -llxrt

CFLAGS += -D__IN_RTAI__ -Wall

all: condtest

condtest: condtest.c
	gcc $(CFLAGS) $(LDFLAGS) condtest.c -o condtest

run: all
	rtai-load --verbose

clean:
	rm condtest
