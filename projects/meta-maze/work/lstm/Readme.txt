Training and testing meta-mazes with the LSTM (Long Short-Term Memory neural network)
simulator obtained from Johannes Kepler University, Institute of Bioinformatics
(www.bioinf.jku.at/software/lstm).

To prepare, type 'make' to compile the lstm.c and *.cpp files,
build maze_tester using src/maze/makefile, and copy maze_tester here.

Then run test_metamaze with the following options:

-epochs <training epochs>
[-contextMaze (generate context patterns) [-modular (modular training)]]
-maze_maker_file <maze_maker maze discovery file>

Each maze entry in the maze_maker file looks like this:

numRooms=5, numDoors=3, numGoals=1
contextSizes: 5
effectDelayScale=10
metaSeed=1755873489
instance seeds/frequencies: 1755873490 (1.000000) 1755873495 (1.000000)

Hint: the run_d<doors>r<rooms>.out files are maze_maker discovery files.

Results: test.res file.



