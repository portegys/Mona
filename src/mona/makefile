# Mona makefile
# Make standalone executable and libraries.

MONA_SOURCES = mona.cpp sense.cpp enable.cpp drive.cpp \
           respond.cpp learn.cpp homeostat.cpp

MONA_OBJECTS = $(MONA_SOURCES:%.cpp=%.o)

MONA_EXEC = ../../bin/mona

MONA_STATIC_LIB = ../../lib/libmona.a

MONA_SHARED_LIB = ../../lib/libmona.so

MONA_JAR = ../../lib/mona.jar

MONA_JNI_LIB = ../../lib/libmona_jni.so

MONA_JAVA = $(MONA_JAR) $(MONA_JNI_LIB)

COMMON_STATIC_LIB = ../../lib/libcommon.a

CC = cc

# Windows code is always position independent
SHELL = /bin/sh
OSNAME := $(shell uname -o)
ifeq ($(OSNAME),Cygwin)
PICFLAG =
JAVA_OS = win32
else
PICFLAG = -fPIC
JAVA_OS = linux
endif

CCFLAGS = $(PICFLAG) -O3

all: $(MONA_EXEC) $(MONA_STATIC_LIB) $(MONA_SHARED_LIB)

java: $(MONA_JAVA)

$(MONA_EXEC): main.o $(MONA_STATIC_LIB)
	$(CC) -o $(MONA_EXEC) main.o -L../../lib -lmona -lcommon -lm -lstdc++

$(MONA_STATIC_LIB) : $(MONA_OBJECTS)
	mkdir -p ../../lib
	ar -cr -o $(MONA_STATIC_LIB) $(MONA_OBJECTS) $(COMMON_STATIC_LIB)

$(MONA_SHARED_LIB) : $(MONA_OBJECTS)
	mkdir -p ../../lib
	$(CC) -shared -o $(MONA_SHARED_LIB) $(MONA_OBJECTS) \
        -Wl,--whole-archive $(COMMON_STATIC_LIB) -Wl,--no-whole-archive -lm -lstdc++

main.o: mona.hpp mona-aux.hpp main.cpp
	$(CC) $(CCFLAGS) -c main.cpp

mona.o: mona.hpp mona-aux.hpp mona.cpp
	$(CC) $(CCFLAGS) -c mona.cpp

sense.o: mona.hpp mona-aux.hpp sense.cpp
	$(CC) $(CCFLAGS) -c sense.cpp

enable.o: mona.hpp mona-aux.hpp enable.cpp
	$(CC) $(CCFLAGS) -c enable.cpp

drive.o: mona.hpp mona-aux.hpp drive.cpp
	$(CC) $(CCFLAGS) -c drive.cpp

learn.o: mona.hpp mona-aux.hpp learn.cpp
	$(CC) $(CCFLAGS) -c learn.cpp

respond.o: mona.hpp mona-aux.hpp respond.cpp
	$(CC) $(CCFLAGS) -c respond.cpp

homeostat.o: mona.hpp homeostat.hpp homeostat.cpp
	$(CC) $(CCFLAGS) -c homeostat.cpp

$(MONA_JAR): mona/NativeFileDescriptor.class mona/Mona.class
	jar cf mona.jar mona
	mkdir -p ../../lib
	cp mona.jar ../../lib

mona/NativeFileDescriptor.class: NativeFileDescriptor.java
	javac -d . NativeFileDescriptor.java
	javah -jni mona.NativeFileDescriptor

mona/Mona.class: Mona.java
	javac -d . Mona.java
	javah -jni mona.Mona

$(MONA_JNI_LIB): cygwin_check mona_jni.o $(MONA_OBJECTS)
	mkdir -p ../../lib
	$(CC) -shared -o $(MONA_JNI_LIB) mona_jni.o $(MONA_OBJECTS) \
        -Wl,--whole-archive $(COMMON_STATIC_LIB) -Wl,--no-whole-archive -lm -lstdc++

cygwin_check:
ifeq ($(OSNAME),Cygwin)
	@echo "Error: JNI incompatible with Cygwin"
	@exit 1
endif

mona_jni.o: mona_Mona.h mona.hpp mona_jni.cpp
	$(CC) $(CCFLAGS) -w -I$(JAVA_HOME)/include \
        -I$(JAVA_HOME)/include/$(JAVA_OS) -c mona_jni.cpp

clean:
	/bin/rm -f *.o
	/bin/rm -f *.so
	/bin/rm -f mona/*.class mona.jar
