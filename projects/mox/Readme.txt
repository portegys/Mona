The Mox project.

A Mox is a simple creature occupying a cell in Conway's Game of Life cellular
automaton. It is input to the Game of Life rules as a live cell, although
it is not affected by the rules. A mox has an orientation that allows it to
sense color in one direction. It also has an omni-directional sense of smell
for the proximity of food. Both of these senses have limited ranges. A mox is
capable of moving forward, turning, and consuming food. There are two species
of moxen: foragers and predators. A forager seeks a specific live cell state
as food which it consumes by changing the cell to the dead state. A predator
seeks forager moxen as food, which it also consumes by eradication. Each species
is uniquely colored. Each mox is controlled by and learns using a Mona neural
network. A primary purpose of the project is to tune the Mona parameters using
a genetic algorithm.

Setup:

1. Unzip the mox.zip file.
2. Copy mona.jar and mona_jni.dll/libmona_jni.so from Mona installation to the
   lib directory. Mona can be obtained from www.codeplex.com/mona.
3. Build with the makefile (Unix), or Visual Studio files (Windows) in the src directory.
4. Run .sh or .bat commands in the bin directory:
   a. game_of_life: run Game of Life
   b. mox_world: run mox in Game of Life world.
   c. evolve and evolve_system: learn and evolve moxen in mox world.
      Example: evolve_system.bat -generations 10 -steps 1000
               -loadCells cells50x50.txt -moxPopulations foragers_and_predators
               -output evolve.out -logfile evolve.log -dashboard
      Note: cells50x50.txt created by game_of_life command (checkpoint before saving).



