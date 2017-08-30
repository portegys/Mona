# The Pong game world makefile

PONG_WORLD = ../../bin/pong_world

PONG_GAME = ../../lib/pong.jar

PONG_LENS = ../../bin/pong_lens

CC = cc

CCFLAGS = -O3

LINKLIBS = -L../../lib -lmona -lcommon -lm -lstdc++

CCFLAGS_LENS = -DNO_TK -O3

LINKLIBS_LENS = -L../../lib -L../../lens/Bin -llens2.63 -ltcl -lm -lstdc++

all: $(PONG_WORLD)

java: $(PONG_GAME)

lens: $(PONG_LENS)

$(PONG_WORLD): pongWorld.o pong.o
	$(CC) -o $(PONG_WORLD) pongWorld.o pong.o $(LINKLIBS)

pongWorld.o: pongWorld.cpp
	$(CC) $(CCFLAGS) -c pongWorld.cpp

pong.o: pong.hpp pong.cpp
	$(CC) $(CCFLAGS) -c pong.cpp

$(PONG_GAME): Pong.java
	javac -d . Pong.java
	cp ../../resource/sounds/click.au mona
	jar cfm pong.jar pong.mf mona
	mkdir -p ../../lib
	cp pong.jar ../../lib

$(PONG_LENS): pongLens.o
	$(CC) -o $(PONG_LENS) pongLens.o $(LINKLIBS_LENS)

pongLens.o: pongLens.cpp
	$(CC) $(CCFLAGS_LENS) -c pongLens.cpp

clean:
	/bin/rm -f *.o
	/bin/rm -f mona/*.class *.jar
