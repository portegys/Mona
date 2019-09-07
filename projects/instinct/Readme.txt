Instinct evolution with Monkey and Bananas problem.

This program demonstrates the learning of "instincts" which are hard-wired mediator neurons that are evolved and
inherited by organisms. The purpose of instincts is to assist an organism in succeeding in its environment, 
serving as a boost to experiential learning. The task is the "Monkey and Bananas" problem.
See paper at http://tom.portegys.com/research.html#instinct for details.

To build:

UNIX:
1. cd mona; export MONA_HOME=`pwd`
2. Set PATH=$MONA_HOME/bin:$PATH
3. Run bin/trim.sh to remove files not used by your platform.
4. Run 'make' to make executables (requires gcc compiler).

Windows:
1. Run the trim.bat file in the bin folder to remove files that
   Windows doesn't need.
2. Run Microsoft Visual Studio (2015 or greater) using solution (.sln) file.

Directories/folders:

1. src:
- mona: Mona version 3.1
- common files.
- instinct: instinct learning program.
- gui: browser demo using WeirdX X Windows Server applet.
2. bin: executables.
3. resource: images, etc.
4. data: data files.
5. include: external headers (*).
6. lib: pre-built libraries (*).

(*): If you have FreeType, Allegro, and/or GlyphKeeper
     installed on your system, you may remove some or all
     of these to save space.

Commands:

1. mona.sh: interactive driver.
2. instinct.sh: instinct demo program (*).
3. evolve_instincts.sh: instinct evolution program (*).
4. extract_dump.sh/extract_graph.sh (Korn shell): extract dump/graph.

(*): These programs use Allegro graphics:
   Allegro (http://alleg.sourceforge.net), and
   Glyph Keeper (http://kd.lab.nig.ac.jp/glyph-keeper/)
   
To run:

UNIX:
Run the bin .sh commands.

Windows:
Use MS Visual Studio,
or,
Open a Command Prompt, navigate to bin and run the .bat files.

The extract commands may be connected together with dot command
(www.graphviz.org) to produce graph images.
Note: compile with MONA_TRACE on.

# Example: create instinct environment jpg image.
instinct.sh <options> | extract_dump.sh 0 - | \
   extract_graph.sh "Instinct environment" - | dot -Tjpg -o env.jpg

# Example: create instinct neural network jpg image.
instinct.sh <options> | extract_dump.sh 1 - | \
   extract_graph.sh "Instinct network" - | dot -Tjpg -o network.jpg
