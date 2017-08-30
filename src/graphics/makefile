# Graphics makefile

GRAPHICS_SOURCES = glbmp.cpp glm.cpp meshHeightmap.cpp baseObject.cpp \
           camera.cpp spacial.cpp frameRate.cpp texture.cpp

GRAPHICS_OBJECTS = $(GRAPHICS_SOURCES:%.cpp=%.o)

GRAPHICS_STATIC_LIB = ../../lib/libgraphics.a

COMMON_STATIC_LIB = ../../lib/libcommon.a

CC = cc

CCFLAGS = -O3

all : $(GRAPHICS_STATIC_LIB)

$(GRAPHICS_STATIC_LIB) : $(GRAPHICS_OBJECTS)
	mkdir -p ../../lib
	ar -cr -o $(GRAPHICS_STATIC_LIB) $(GRAPHICS_OBJECTS) $(COMMON_STATIC_LIB)

glbmp.o: glbmp.h glbmp.cpp
	$(CC) $(CCFLAGS) -c glbmp.cpp

glm.o: glm.h glm.cpp
	$(CC) $(CCFLAGS) -w -c glm.cpp

meshHeightmap.o: meshHeightmap.hpp meshHeightmap.cpp
	$(CC) $(CCFLAGS) -c meshHeightmap.cpp

baseObject.o: baseObject.hpp baseObject.cpp
	$(CC) $(CCFLAGS) -c baseObject.cpp

camera.o: camera.hpp camera.cpp
	$(CC) $(CCFLAGS) -c camera.cpp

spacial.o: spacial.hpp spacial.cpp
	$(CC) $(CCFLAGS) -c spacial.cpp

frameRate.o: frameRate.hpp frameRate.cpp
	$(CC) $(CCFLAGS) -c frameRate.cpp

texture.o: texture.h texture.cpp
	$(CC) $(CCFLAGS) -c texture.cpp

clean:
	/bin/rm -f *.o
