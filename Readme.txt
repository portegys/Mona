This directory contains the Mona neural network, version 5.2.
See the white paper at http://tom.portegys.com/research/MonaWhitepaper.pdf
For conditions of distribution and use, see the license in src/mona/mona.hpp.

Projects included:

1. The Muzz world, a 3D environment of blocks and ramps. 
   http://tom.portegys.com/research.html#muzz
2. Maze-learning mouse.
3. The Minc T-maze world. A T-maze is generated from a grammar. 
   http://tom.portegys.com/research.html#minc
4. The Pong game world.
   http://tom.portegys.com/research.html#pong
5. Atani simulated robot using Microsoft Robotics Studio (see atani/Readme.txt).
   http://tom.portegys.com/research.html#atani
6. instinct: Instinct evolution with Monkey and Bananas problem.
   http://tom.portegys.com/research.html#instinct
7. Lego Mindstorms NXT robot driver using LeJOS NXJ OS (see legoNXT/Readme.txt).
8. Mox world: Mona creatures evolving in a Conway's Game of Life cellular automaton
   environment (see mox/Readme.txt).
   http://tom.portegys.com/research.html#mox
9. maze: Instinct and Learning Synergy in Simulated Foraging Using a Neural Network.
   http://tom.portegys.com/research.html#muzz
10. meta-maze: A Maze Learning Comparison of Elman, Long Short-Term Memory, and Mona Neural Networks.
    http://tom.portegys.com/research.html#maze
11. prototypes: Foraging ant and nesting birds.
    http://tom.portegys.com/research.html#mona1
    http://tom.portegys.com/research.html#mona2

Required packages:

The UNIX/Cygwin version requires the C++ compiler and make commands.
The Windows version requires Microsoft Visual Studio 2015 (or later).
The Muzz world requires the OpenGL graphics and the glut/freeglut packages.
The Mona jar with JNI library, Minc T-maze viewer, Pong game app
and the maze-learning mouse require Java.
The Mona C# dll requires Visual C#.
The Minc world uses the included Lens neural network which requires
the TCL library (ActiveTcl is available for Windows).
The Atani robot and the Mona C# dll require Microsoft Robotics Studio and
Microsoft Visual Studio C# 2008 (or later).
The Lego NXT robot driver requires LeJOS NXJ and Java.
The Mox world is Java based, but also uses JNI to call C++ libraries.

To install and build:

1. Unpack the tarball/zip file.

UNIX/Cygwin:
2. Set environment (bash/ksh syntax):
   cd mona; export MONA_HOME=`pwd`;
   PATH=$MONA_HOME/bin:$PATH
   LD_LIBRARY_PATH=$MONA_HOME/lib:$LD_LIBRARY_PATH
   If building Java:
   JAVA_HOME=<JDK directory>
   PATH=$JAVA_HOME/bin:$PATH
3. Run 'make' to build base libraries in the lib directory
   and executables in the bin directory.
4. Run 'make muzz' to build OpenGL graphics, GUI, and the Muzz world.
5. Run 'make minc' to build Lens and the Minc world.
6. Run 'make pong' to build the Pong game world.
7. Run 'make java' to build the Mona jar with JNI library (incompatible
   with Cygwin), Minc world viewers, Pong game app, and the maze-learning
   mouse (see applet signing note).
8. For Mox world, run 'make' in src directory.
Note: Project can also be imported and built using Eclipse IDE (with C++ tools).

Windows:
2. Build the base libraries and executables with Visual Studio
   using solution (.sln) file.
3. Build the muzz project for the OpenGL graphics, GUI, and Muzz world.
4. Build the minc project for the Lens neural network and Minc world.
5. Build the pong project for the Pong game world.
6. To build Java projects, set JAVA_HOME to the JDK folder.
   Projects: mona_java for the Mona jar with JNI library, minc_viewers,
   Pong game app, and maze_mouse for the maze-learning mouse
   (see applet signing note).
7. For Atani simulated robot, install atani folder under MSRS apps folder then
   follow atani/Readme.txt for build instructions.
8. For Lego NXT driver, run build.bat in the legoNXT/bin folder.
9. For Mox world, build with the Visual Studio solution file in the src folder.

Note on signing maze-learning mouse applet:
Generate a one-time maze_mouse key for the jarsigner tool:
keytool -genkey -alias maze_mouse -keypass <password>
Then sign: jarsigner MazeMouse.jar maze_mouse

Directories/folders:

1. src/
- mona.
- common.
- graphics.
- gui library.
- muzz world
- maze-learning mouse.
- minc T-maze world.
- pong game world.
2. lens/
- LENS neural network.
3. bin/, lib/ executables and libraries:
- mona.lib, mona.dll: Windows import lib and dll
- lib*.a, libmona.so: UNIX static, shared libs.
- mona.jar, mona_jni.dll, libmona_jni.so: Java jar and JNI Windows/UNIX libs.
- mona_cs.dll, mona_cs.snk: C# dll, use with mona_cs_dll.dll.
- MincRunViewer.jar: Minc world run viewer.
- TmazeViewer.jar: T-maze viewer to view T-mazes (*.ex) created for Minc world.
- pong.jar: Pong game app.
- MazeMouse.jar: maze-learning mouse and socker TCP network interface to Mona
  (see instructions below).
4. resource/ 
- GUI layout
- images
- sounds
5. projects/
- Atani
- instinct
- LegoNXT
- maze
- meta-maze
- Mox
- prototypes

Commands:

1. mona: (stdio) interactive driver.
2. muzz_world: Muzz environment (uses OpenGL graphics).
3. minc_world: Minc environment.
4. pong_world: Pong game environment (see src/pong/Readme for more details).

Running the Muzz world:

1. To run a 6x6 randomly generated world and save it in file "muzz.world":

muzz_world -terrainDimension 6 -save muzz.world

2. To load file "muzz.world" and continue after pausing:

muzz_world -load muzz.world -pause

3. Training the Muzz:

a. Muzz training with the reward-trainable Muzz is done using the GUI.

b. Forced response training:

To run a 5x5 world using a random seed of 9 and forcibly train a successful 
goal path for 25 trials:

muzz_world -terrainDimension 5 -randomSeed 9 -numTrainingTrials 25 -forcedResponseTrain

This first entails searching for a response sequence that satisfies active needs. The 
muzz is then forced to express these responses to take a path to goals.

Maze-learning mouse:

Applet:
1. Launch mona server:
Run socker on port 7671 to launch mona on connect:
socker 7671 $MONA_HOME/bin/mona
Note: kill socker command when finished to free port and avoid security risk.
2. Edit maze_mouse.html and set ServerHost (and ServerPort if needed).
3. Run appletviewer on maze_mouse.html. Instructions are on the web page.

Application:
java -Djava.library.path=$MONA_HOME/bin (Windows) or $MONA_HOME/lib (UNIX)
   -jar MazeMouse.jar [-randomSeed <random seed>]
See src/maze_mouse/maze_mouse.bat,maze_mouse.sh

Atani simulated robot:

Train robot to solve T-maze with dashboard.

Lego NXT robot:

Train robot using dashboard.

Mox world: see mox/Readme.txt



