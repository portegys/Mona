# Mona makefile

CC = gcc

CCFLAGS = -DUNIX

all: mona.o utility.o

mona.o: mona.hpp mona.cpp sense.cpp enable.cpp drive.cpp respond.cpp \
	../common/pointerlist.hpp ../common/valueset.hpp
	$(CC) $(CCFLAGS) -c mona.cpp sense.cpp enable.cpp drive.cpp respond.cpp

utility.o: mona.hpp utility.cpp
	$(CC) $(CCFLAGS) -c utility.cpp

clean:
	/bin/rm -f *.o
