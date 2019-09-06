Training and testing meta-mazes with the SNNS (Stuttgart Neural Network Simulator).

To prepare, type 'make' to compile all the *.cpp files, 
build maze_tester using src/maze/makefile, and copy maze_tester here.

Get and install SNNS 4.2, available at:
www-ra.informatik.uni-tuebingen.de/SNNS/
Make sure the batchman batch simulator is in the PATH.

Then run test_metamaze with the following options:

-epochs <training epochs>
[-contextMaze (generate context patterns)]
[-filePrefix <file prefix>
   (file names: <prefix>-<metaSeed>-<instanceSeed>.pat)]
-maze_maker_file <maze_maker maze discovery file>

Each maze entry in the maze_maker file looks like this:

numRooms=5, numDoors=3, numGoals=1
contextSizes: 5
effectDelayScale=10
metaSeed=1755873489
instance seeds/frequencies: 1755873490 (1.000000) 1755873495 (1.000000)

Hint: the run_d<doors>r<rooms>.out files are maze_maker discovery files.

Output:

Trained network: network.net
Results: test.res



