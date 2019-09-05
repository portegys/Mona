# Mona makefile

CC = gcc

CCFLAGS = -DUNIX -DMONA_TRACE -O3

all: ../../bin/mona mona.o main.o dump.o

../../bin/mona: main.o mona.o dump.o
	@(cd ../common; make)
	$(CC) -o ../../bin/mona main.o mona.o sense.o enable.o drive.o respond.o learn.o dump.o \
		../common/valueset.o ../common/random.o ../common/fileio.o -lm -lstdc++

mona.o: mona.hpp mona-aux.hpp mona.cpp sense.cpp enable.cpp drive.cpp respond.cpp learn.cpp \
	../common/valueset.hpp ../common/random.hpp
	$(CC) $(CCFLAGS) -c mona.cpp sense.cpp enable.cpp drive.cpp respond.cpp learn.cpp

main.o: mona.hpp main.cpp
	$(CC) $(CCFLAGS) -c main.cpp

dump.o: mona.hpp dump.cpp
	$(CC) $(CCFLAGS) -c dump.cpp

clean:
	/bin/rm -f *.o
