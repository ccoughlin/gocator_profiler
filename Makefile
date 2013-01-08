CC=g++
CFLAGS=-c -Wall -I/home/ccoughlin/src/c/14400-3.4.1.155_SOFTWARE_Go2_SDK/include -Iinclude
LDFLAGS=-L/usr/lib/i386-linux-gnu/ -lpthread -lrt
DEPS=Go2.h
SOURCES=main.cxx go2response.cxx
OBJECTS=$(SOURCES:.cxx=.o)
EXECUTABLE=gocator_encoder

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) /home/ccoughlin/src/cxx/gocator_encoder_scratch/libGo2.so $(LDFLAGS) -o $@

main.o:
	$(CC) $(CFLAGS) main.cxx

go2response.o:
	$(CC) $(CFLAGS) go2response.cxx

clean:
	rm -rf *.o gocator_encoder
