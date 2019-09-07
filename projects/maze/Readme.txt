Maze learning.

The learner can be trained to respond to a general
meta-maze or specific maze instances. For meta-maze
training, initially the learner is directed to respond
optimally to the probabilistic maze, but as learner
responses reveal information about the specific
instance of the maze, the learner is directed
accordingly. For maze instance training, the learner
is directed as though the maze probabilities are
resolved from the start. Response training can be
tailored to taper off in strength gradually.

Optionally, the learner must retain contextual
knowledge while traversing the maze, and then apply
the retained knowledge to reach a goal. Specifically,
the learner is presented with an initial set of doors,
only one of which leads to the beginning of a maze
That door choice must be repeated at the maze exit
in order to obtain the goal.

See paper at http://tom.portegys.com/research.html#context2

To build:

UNIX:
Run 'make' to make executables (requires gcc compiler).

Windows:
Run Microsoft Visual Studio (2015 or greater) using solution (.sln) file.

Directories/folders:

1. src:
- mona: Mona version 3.1.1
- common files.
- maze: maze environment.
- gui: graphical programs.
2. bin: executables.
3. resource: images, etc.

Commands:

1. mona.sh: interactive driver.
2. maze_bridge_tester: bridge maze tester.
3. maze_tester.sh: maze tester program.
4. maze_maker.sh: discovers interesting mazes.

To run:

UNIX:
Run the bin .sh commands.

Windows:
Use MS Visual Studio,
or,
Open a Command Prompt, navigate to bin and run the .bat files.
