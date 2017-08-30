# Lens client makefile

LEARN_TMAZE = ../Bin/learnTmaze

TEST_LENS = ../Bin/testLens

CC = cc

CCFLAGS = -O3

all: $(LEARN_TMAZE) $(TEST_LENS)

$(LEARN_TMAZE): learnTmaze.o TmazeGrammar.o Tmaze.o
	$(CC) -o $(LEARN_TMAZE) learnTmaze.o TmazeGrammar.o Tmaze.o \
        -L../../lib -lcommon -L../Bin -llens2.63 -ltcl -lm -lstdc++

$(TEST_LENS): testLens.o
	$(CC) -o $(TEST_LENS) testLens.o -L../Bin -llens2.63 -ltcl -lm -lstdc++

learnTmaze.o:
	$(CC) $(CCFLAGS) -c -I../Src -DNO_TK learnTmaze.cpp

TmazeGrammar.o:
	$(CC) $(CCFLAGS) -c ../../src/minc/TmazeGrammar.cpp

Tmaze.o:
	$(CC) $(CCFLAGS) -c ../../src/minc/Tmaze.cpp

testLens.o:
	$(CC) $(CCFLAGS) -c -I../Src -DNO_TK testLens.cpp

clean:
	rm -f *.o
