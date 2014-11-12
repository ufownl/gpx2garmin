CXX = g++
LD = g++
CXXFLAGS = -g -Wall -I../contrib/include -I/usr/local/include
LDFLAGS =

all: gpx2garmin

gpx2garmin: main.o
	$(LD) $^ $(LDFLAGS) -o $@

main.o: main.cpp

clean:
	$(RM) *.o
	$(RM) gpx2garmin

.PHONY: all clean
