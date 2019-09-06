# SNNS makefile

CC = gcc

CCFLAGS = -O3

all: check_results create_metamaze_patterns randomize_strings test_metamaze

check_results: check_results.cpp
	$(CC) $(CCFLAGS) -o check_results check_results.cpp -lstdc++

create_metamaze_patterns: create_metamaze_patterns.cpp
	$(CC) $(CCFLAGS) -o create_metamaze_patterns create_metamaze_patterns.cpp -lstdc++

randomize_strings: randomize_strings.cpp
	$(CC) $(CCFLAGS) -o randomize_strings randomize_strings.cpp -lstdc++

test_metamaze: test_metamaze.cpp
	$(CC) $(CCFLAGS) -o test_metamaze test_metamaze.cpp -lstdc++

clean:
	/bin/rm -f *.o

