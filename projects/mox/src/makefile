# Mox UNIX makefile

CC = cc

SHELL = /bin/sh
OSNAME := $(shell uname -o)
PICFLAG = -fPIC
JAVA_OS = linux

JAVA_SOURCES = EvolveCommon.java EvolveMoxen.java EvolveMoxenSystem.java \
           ForagerMox.java GameOfLife.java GameOfLifeApplication.java \
           GameOfLifeCanvas.java Gene.java Genome.java Mox.java \
           MoxWorld.java MoxDashboard.java MoxWorldDashboard.java \
           MoxWorldBuilder.java PredatorMox.java ProcessInformation.java \
           Utility.java

JNI_SOURCES = ProcessInformation.java

JNI_CLASSES = $(JNI_SOURCES:%.java=%)

JNI_OBJECTS = $(JNI_SOURCES:%.java=%.class)

JNI_HEADER = $(JNI_SOURCES:%.java=%.h)

CCFLAGS = $(PICFLAG) -DUNIX -O3

LINKLIBS =  -lm -lstdc++

CPP_SOURCES = ProcessInformationUNIX.cpp

CPP_OBJECTS = $(CPP_SOURCES:%.cpp=../build/%.o)

SHARED_LIB = ../lib/libProcessInformation.so

all: java_objects $(SHARED_LIB)

java_objects: $(JAVA_SOURCES)
	javac -d ../build -classpath ../lib/mona.jar $(JAVA_SOURCES)

$(SHARED_LIB): $(JNI_HEADER) $(CPP_OBJECTS)
	$(CC) -shared -o $(SHARED_LIB) $(CPP_OBJECTS) $(LINKLIBS)

$(JNI_HEADER): $(JNI_OBJECTS)
	javah -jni -classpath ../build $(JNI_CLASSES)

$(JNI_OBJECTS): $(JNI_SOURCES)
	javac -d ../build $(JNI_SOURCES)

$(CPP_OBJECTS): $(CPP_SOURCES)
	$(CC) $(CCFLAGS) -w -I$(JAVA_HOME)/include \
        -I$(JAVA_HOME)/include/$(JAVA_OS) -c $(CPP_SOURCES) -o $(CPP_OBJECTS)

# Uses uncrustify (uncrustify.sourceforge.net)
.SILENT beautify:
	@echo "Beautifying..."
	-@(uncrustify *.java 2>/dev/null)
	-@(uncrustify *.h *.cpp 2>/dev/null)
	@echo "done"

clean:
	/bin/rm -f ../build/*.class
	/bin/rm -f $(CPP_OBJECTS)
