CC=g++
GOCATOR_SDK=/home/ccoughlin/src/c/14400-3.4.1.155_SOFTWARE_Go2_SDK
CFLAGS=-c -Wall -I$(GOCATOR_SDK)/include -Iinclude
LDFLAGS=-L/usr/lib/i386-linux-gnu/ -lpthread -lrt -lboost_program_options -lboost_filesystem -lboost_system -lboost_thread-mt
DEPS=Go2.h
SOURCES=main.cxx go2response.cxx gocatorsystem.cxx gocatorcontrol.cxx gocatorconfigurator.cxx
OBJECTS=$(SOURCES:.cxx=.o)
EXECUTABLE=gocator_encoder

all:	$(SOURCES) $(EXECUTABLE)

$(EXECUTABLE):	$(OBJECTS)
	$(CC) $(OBJECTS) $(GOCATOR_SDK)/lib/libGo2.so $(LDFLAGS) -o $@

main.o:	main.cxx
	$(CC) $(CFLAGS) main.cxx

go2response.o:	go2response.cxx
	$(CC) $(CFLAGS) go2response.cxx

gocatorsystem.o: gocatorsystem.cxx
	$(CC) $(CFLAGS) gocatorsystem.cxx

gocatorcontrol.o:	gocatorcontrol.cxx
	$(CC) $(CFLAGS) gocatorcontrol.cxx

gocatorconfigurator.o:	gocatorconfigurator.cxx
	$(CC) $(CFLAGS) gocatorconfigurator.cxx

clean:
	rm -rf *.o gocator_encoder
