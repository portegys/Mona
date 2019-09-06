# LSTM makefile

CC = gcc

CCFLAGS = -O3

all: lstm randomize_strings test_metamaze

lstm: lstm.c
	$(CC) $(CCFLAGS) -o lstm lstm.c -lm

randomize_strings: randomize_strings.cpp
	$(CC) $(CCFLAGS) -o randomize_strings randomize_strings.cpp -lstdc++

test_metamaze: test_metamaze.cpp
	$(CC) $(CCFLAGS) -o test_metamaze test_metamaze.cpp -lstdc++

clean:
	/bin/rm -f *.o

