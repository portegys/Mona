# The Mona instinct program makefile

CC = gcc

# MONA_TRACE: tracing.
# ACTIVITY_TRACKING: tracks firing, enablement, and drive activity for network dump.
#CCFLAGS = -DUNIX -DMONA_TRACE -DACTIVITY_TRACKING -O3
CCFLAGS = -DUNIX -DMONA_TRACE -O3
LINKFLAGS = -O3
LINKLIBS = -lm -lstdc++

all: ../../bin/instinct ../../bin/evolve_instincts ../../bin/display_control

../../bin/instinct: instinct_main.o instinct.o monkey_and_bananas.o
	@(cd ../mona; make)
	$(CC) $(LINKFLAGS) -o ../../bin/instinct ../mona/mona.o ../mona/sense.o ../mona/enable.o ../mona/drive.o ../mona/respond.o ../mona/learn.o ../mona/dump.o \
		../common/valueset.o ../common/random.o ../common/fileio.o ../common/gettime.o \
		instinct_main.o instinct.o monkey_and_bananas.o $(LINKLIBS)

../../bin/evolve_instincts: evolve.o instinct.o monkey_and_bananas.o
	@(cd ../mona; make)
	$(CC) $(LINKFLAGS) -o ../../bin/evolve_instincts ../mona/mona.o ../mona/sense.o ../mona/enable.o ../mona/drive.o ../mona/respond.o ../mona/learn.o ../mona/dump.o \
		../common/valueset.o ../common/random.o ../common/fileio.o ../common/gettime.o ../common/log.o \
		evolve.o instinct.o monkey_and_bananas.o $(LINKLIBS)

instinct_main.o: ../mona/mona.hpp monkey_and_bananas.hpp instinct.hpp instinct_main.cpp
	$(CC) $(CCFLAGS) -c instinct_main.cpp

evolve.o: ../mona/mona.hpp monkey_and_bananas.hpp evolve.cpp
	$(CC) $(CCFLAGS) -c evolve.cpp

instinct.o: ../mona/mona.hpp instinct.hpp instinct.cpp
	$(CC) $(CCFLAGS) -c instinct.cpp

monkey_and_bananas.o: ../mona/mona.hpp monkey_and_bananas.hpp monkey_and_bananas.cpp
	$(CC) $(CCFLAGS) -c monkey_and_bananas.cpp

../../bin/display_control: display_control.cpp
	$(CC) $(CCFLAGS) -o ../../bin/display_control display_control.cpp -lstdc++

clean:
	/bin/rm -f *.o

