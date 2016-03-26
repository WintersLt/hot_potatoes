CC=g++
CFLAGS=-c -Wall -g
LDFLAGS= -L .
SOURCES=comm.cpp rm.cpp player.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=hello

INCLUDES=-I .

all: $(SOURCES) $(OBJECTS) master player

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

master: 
	$(CC) $(INCLUDES) rm.cpp comm.cpp -o $@ $(LDFLAGS) -lm

player: 
	$(CC) $(INCLUDES) player.cpp comm.cpp -o $@ $(LDFLAGS) -lm

clean:
	rm -f *.o
	rm -f master player
