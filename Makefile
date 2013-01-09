CC=g++
GOCATOR_SDK=/home/ccoughlin/src/c/14400-3.4.1.155_SOFTWARE_Go2_SDK
CFLAGS=-c -Wall -I$(GOCATOR_SDK)/include -Iinclude
LDFLAGS=-L/usr/lib/i386-linux-gnu/ -lpthread -lrt -lboost_program_options
DEPS=Go2.h
SOURCES=main.cxx go2response.cxx gocatorsystem.cxx gocatorcontrol.cxx
OBJECTS=$(SOURCES:.cxx=.o)
EXECUTABLE=gocator_encoder

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(GOCATOR_SDK)/lib/libGo2.so $(LDFLAGS) -o $@

main.o:
	$(CC) $(CFLAGS) main.cxx

go2response.o:
	$(CC) $(CFLAGS) go2response.cxx

gocatorsystem.o:
	$(CC) $(CFLAGS) gocatorsystem.cxx

gocatorcontrol.o:
	$(CC) $(CFLAGS) gocatorcontrol.cxx

clean:
	rm -rf *.o gocator_encoder
