# Common makefile

CC = gcc

CCFLAGS = -DUNIX -DMONA_TRACE -O3

all: valueset.o random.o fileio.o gettime.o log.o

valueset.o: valueset.hpp valueset.cpp
	$(CC) $(CCFLAGS) -c valueset.cpp

random.o: random.hpp random.cpp
	$(CC) $(CCFLAGS) -c random.cpp

fileio.o: fileio.h fileio.cpp
	$(CC) $(CCFLAGS) -c fileio.cpp

gettime.o: gettime.cpp
	$(CC) $(CCFLAGS) -c gettime.cpp
	
log.o: log.cpp log.hpp
	$(CC) $(CCFLAGS) -c log.cpp

clean:
	/bin/rm -f *.o
