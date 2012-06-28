
CPP=g++
RM=rm
CPPFLAGS=-Wall
ECHO=echo
STRIP=strip
LDFLAGS=-lboost_system -lboost_filesystem -pthread
OPT_FLAGS=-O3
DEBUG_FLAGS=-O0 -g3
STATIC_FLAGS=-static
INCLUDES=-Isrc/ -Isrc/packets/
EXE=bin/IF.MGMT

-include Makefile.sources

all: dynamic_binary

debug_binary:
	$(ECHO) Building debug binary...
	$(CPP) $(DEBUG_FLAGS) $(CPPFLAGS) $(SOURCES) -o $(EXE) $(LDFLAGS)

dynamic_binary:
	$(ECHO) Building dynamic binary...
	$(CPP) $(CPPFLAGS) $(SOURCES) -o $(EXE) $(LDFLAGS)

static_binary:
	$(ECHO) Building static binary...
	$(CPP) $(STATIC_FLAGS) $(CPPFLAGS) $(SOURCES) -o $(EXE) $(LDFLAGS)

strip:
	$(STRIP) $(EXE) &> /dev/null

clean:
	$(RM) -rf *.o src/*.o src/packets/*.o $(EXE)

