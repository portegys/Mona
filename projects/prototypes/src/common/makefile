# Common makefile

CC = gcc

CCFLAGS = -DUNIX -DMONA_TRACE

all: pointerlist.o valueset.o random.o point.o fsm.o

pointerlist.o: pointerlist.hpp pointerlist.cpp
	$(CC) $(CCFLAGS) -c pointerlist.cpp

valueset.o: valueset.hpp valueset.cpp
	$(CC) $(CCFLAGS) -c valueset.cpp

random.o: random.hpp random.cpp
	$(CC) $(CCFLAGS) -c random.cpp

point.o: point.hpp point.cpp
	$(CC) $(CCFLAGS) -c point.cpp

fsm.o: fsm.hpp fsm.cpp
	$(CC) $(CCFLAGS) -c fsm.cpp

lregress.o: lregress.h lregress.cpp
	$(CC) $(CCFLAGS) -c lregress.cpp

clean:
	/bin/rm -f *.o
