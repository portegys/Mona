# The Muzz world makefile.

MUZZ_SOURCES = blockTerrain.cpp muzz.cpp muzzWorld.cpp \
             mushroom.cpp pool.cpp TmazeTerrain.cpp

MUZZ_OBJECTS = $(MUZZ_SOURCES:%.cpp=%.o)

MUZZ_WORLD = ../../bin/muzz_world

EVOLVE_SOURCES = blockTerrain.cpp muzz.cpp evolveMuzzes.cpp \
             mushroom.cpp pool.cpp TmazeTerrain.cpp

EVOLVE_OBJECTS = $(EVOLVE_SOURCES:%.cpp=%.o) muzzWorldSub.o

EVOLVE_MUZZES = ../../bin/evolve_muzzes

CC = cc

CCFLAGS = -O3

LINKLIBS = -L../../lib -lmona -lgraphics -lcommon \
         -lsxmlgui -lglpng -lglut -lGLU -lGL -lm -lstdc++

all: $(MUZZ_WORLD) $(EVOLVE_MUZZES)

$(MUZZ_WORLD): $(MUZZ_OBJECTS)
	$(CC) -o $(MUZZ_WORLD) $(MUZZ_OBJECTS) $(LINKLIBS)

$(EVOLVE_MUZZES): $(EVOLVE_OBJECTS)
	$(CC) -o $(EVOLVE_MUZZES) $(EVOLVE_OBJECTS) $(LINKLIBS)
	rm muzzWorldSub.o

blockTerrain.o: blockTerrain.hpp blockTerrain.cpp
	$(CC) $(CCFLAGS) -c blockTerrain.cpp

evolveMuzzes.o: evolveMuzzes.hpp evolveMuzzes.cpp
	$(CC) $(CCFLAGS) -c evolveMuzzes.cpp

muzz.o: muzz.hpp muzz.cpp
	$(CC) $(CCFLAGS) -c muzz.cpp

muzzWorld.o: muzzWorld.hpp muzzWorld.cpp
	$(CC) $(CCFLAGS) -c muzzWorld.cpp

muzzWorldSub.o: muzzWorld.hpp muzzWorld.cpp
	$(CC) $(CCFLAGS) -DMUZZ_WORLD_DRIVER -c muzzWorld.cpp
	mv muzzWorld.o muzzWorldSub.o

mushroom.o: mushroom.hpp mushroom.cpp
	$(CC) $(CCFLAGS) -c mushroom.cpp

pool.o: pool.hpp pool.cpp
	$(CC) $(CCFLAGS) -c pool.cpp

TmazeTerrain.o: TmazeTerrain.hpp TmazeTerrain.cpp
	$(CC) $(CCFLAGS) -c TmazeTerrain.cpp

clean:
	/bin/rm -f *.o


