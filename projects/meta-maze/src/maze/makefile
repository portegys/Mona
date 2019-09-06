# The Mona maze program makefile

CC = gcc

# MONA_TRACE: tracing.
# ACTIVITY_TRACKING: tracks firing, enablement, and drive activity for network dump.
#CCFLAGS = -DUNIX -DMONA_TRACE -DACTIVITY_TRACKING -O3
CCFLAGS = -DUNIX -DMONA_TRACE -O3
LINKFLAGS = -O3
LINKLIBS = -lm -lstdc++

all: ../../bin/maze_tester ../../bin/maze_bridge_tester ../../bin/maze_maker

../../bin/maze_tester: mazeTester.o mazeDriver.o mazeComponent.o maze.o
	@(cd ../mona; make)
	$(CC) $(LINKFLAGS) -o ../../bin/maze_tester ../mona/mona.o ../mona/sense.o ../mona/enable.o \
        	../mona/drive.o ../mona/respond.o ../mona/learn.o ../mona/dump.o \
		../common/valueset.o ../common/random.o ../common/fileio.o \
		mazeComponent.o maze.o mazeTester.o mazeDriver.o $(LINKLIBS)

../../bin/maze_bridge_tester: mazeBridgeTester.o mazeBridgeDriver.o mazeDriver.o mazeComponent.o maze.o
	@(cd ../mona; make)
	$(CC) $(LINKFLAGS) -o ../../bin/maze_bridge_tester ../mona/mona.o ../mona/sense.o ../mona/enable.o \
        	../mona/drive.o ../mona/respond.o ../mona/learn.o ../mona/dump.o \
		../common/valueset.o ../common/random.o ../common/fileio.o \
		mazeComponent.o maze.o mazeBridgeTester.o mazeBridgeDriver.o mazeDriver.o $(LINKLIBS)

../../bin/maze_maker: mazeMaker.o mazeComponent.o maze.o ../common/random.o ../common/fileio.o
	$(CC) $(LINKFLAGS) -o ../../bin/maze_maker mazeComponent.o maze.o mazeMaker.o ../common/random.o ../common/fileio.o $(LINKLIBS)

mazeComponent.o: mazeComponent.hpp mazeComponent.cpp
	$(CC) $(CCFLAGS) -c mazeComponent.cpp

maze.o: maze.hpp maze.cpp mazeComponent.hpp
	$(CC) $(CCFLAGS) -c maze.cpp

mazeTester.o: ../mona/mona.hpp mazeComponent.hpp maze.hpp mazeDriver.hpp mazeTester.cpp
	$(CC) $(CCFLAGS) -c mazeTester.cpp

mazeDriver.o: ../mona/mona.hpp mazeComponent.hpp maze.hpp mazeDriver.hpp mazeDriver.cpp
	$(CC) $(CCFLAGS) -c mazeDriver.cpp

mazeBridgeTester.o: ../mona/mona.hpp mazeComponent.hpp maze.hpp \
	mazeBridgeDriver.hpp mazeDriver.hpp mazeBridgeTester.cpp
	$(CC) $(CCFLAGS) -c mazeBridgeTester.cpp

mazeBridgeDriver.o: ../mona/mona.hpp mazeComponent.hpp maze.hpp \
	mazeBridgeDriver.hpp mazeBridgeDriver.cpp mazeDriver.hpp mazeDriver.cpp
	$(CC) $(CCFLAGS) -c mazeBridgeDriver.cpp

mazeMaker.o: mazeComponent.hpp maze.hpp mazeMaker.cpp mazePathFinder.hpp
	$(CC) $(CCFLAGS) -c mazeMaker.cpp

clean:
	/bin/rm -f *.o
