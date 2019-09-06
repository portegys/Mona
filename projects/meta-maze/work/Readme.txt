Training and testing meta-mazes.

I. Mona

To prepare, compile all the *.cpp files using the makefile,
build maze_tester and maze_maker using src/maze/makefile,
and copy maze_tester and maze_maker here.

Test meta-mazes by running batch_metamaze_test with the following options:

[-contextMaze (generate context patterns) [-modular (modular training)]]
<maze_maker maze discovery file>

Each maze entry in the maze_maker file looks like this:

numRooms=5, numDoors=3, numGoals=1
contextSizes: 5
effectDelayScale=10
metaSeed=1755873489
instance seeds/frequencies: 1755873490 (1.000000) 1755873495 (1.000000)

Hint: the run_d<doors>r<rooms>.out files are maze_maker discovery files.

Output: test results written to standard output.

II. SNNS (Stuttgart Neural Network Simulator).

See snns/Readme.txt

III. LSTM (Long Short-Term Memory)

See lstm/Readme.txt

=====================================

Meta-mazes can be discovered by editing and running run_maze_maker.sh.



