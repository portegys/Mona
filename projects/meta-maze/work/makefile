# Meta-maze test makefile

CC = gcc

CCFLAGS = -O3

all: batch_metamaze_test batch_qlearn_test

batch_metamaze_test: batch_metamaze_test.cpp
	$(CC) $(CCFLAGS) -o batch_metamaze_test batch_metamaze_test.cpp -lstdc++

batch_qlearn_test: batch_qlearn_test.cpp
	$(CC) $(CCFLAGS) -o batch_qlearn_test batch_qlearn_test.cpp -lstdc++

clean:
	/bin/rm -f *.o

