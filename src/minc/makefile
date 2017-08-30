# The Minc world makefile

MINC_WORLD = ../../bin/minc_world

TMAZE_MAKER = ../../bin/Tmaze_maker

MINC_RUN_VIEWER = ../../lib/mincRunViewer.jar

TMAZE_VIEWER = ../../lib/TmazeViewer.jar

CC = cc

CCFLAGS = -DNO_TK -O3

LINKLIBS = -L../../lib -L../../lens/Bin -lmona -lcommon -llens2.63 -ltcl -lm -lstdc++

all: $(MINC_WORLD) $(TMAZE_MAKER)

java: $(MINC_RUN_VIEWER) $(TMAZE_VIEWER)

$(MINC_WORLD): mincWorld.o minc.o TmazeGrammar.o Tmaze.o
	$(CC) -o $(MINC_WORLD) mincWorld.o minc.o TmazeGrammar.o Tmaze.o $(LINKLIBS)

$(TMAZE_MAKER): TmazeMaker.o TmazeGrammar.o Tmaze.o
	$(CC) -o $(TMAZE_MAKER) TmazeMaker.o TmazeGrammar.o Tmaze.o $(LINKLIBS)

mincWorld.o: Tmaze.hpp mincWorld.cpp
	$(CC) $(CCFLAGS) -c mincWorld.cpp

minc.o: minc.hpp minc.cpp
	$(CC) $(CCFLAGS) -c minc.cpp

TmazeMaker.o: TmazeMaker.cpp
	$(CC) $(CCFLAGS) -c TmazeMaker.cpp

TmazeGrammar.o: TmazeGrammar.hpp TmazeGrammar.cpp
	$(CC) $(CCFLAGS) -c TmazeGrammar.cpp

Tmaze.o: Tmaze.hpp Tmaze.cpp
	$(CC) $(CCFLAGS) -c Tmaze.cpp

$(MINC_RUN_VIEWER): MincRunViewer.java TmazeDisplay.java Tmaze.java
	/bin/rm -fr mona
	javac -d . MincRunViewer.java TmazeDisplay.java Tmaze.java
	jar cfm mincRunViewer.jar mincRunViewer.mf mona
	mkdir -p ../../lib
	cp mincRunViewer.jar ../../lib

$(TMAZE_VIEWER): TmazeViewer.java TmazeDisplay.java Tmaze.java
	/bin/rm -fr mona
	javac -d . TmazeViewer.java TmazeDisplay.java Tmaze.java
	jar cfm TmazeViewer.jar TmazeViewer.mf mona
	mkdir -p ../../lib
	cp TmazeViewer.jar ../../lib

clean:
	/bin/rm -f *.o
