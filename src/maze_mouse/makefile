# Maze-learning mouse makefile
# Use keytool -genkey -alias maze_mouse -keypass <password>
# to generate a maze_mouse key for the jarsigner tool.

CC = cc

all: ../../lib/MazeMouse.jar ../../bin/evolve_mouse ../../bin/socker

# Maze-learning mouse.
../../lib/MazeMouse.jar: MazeMouse.java MazeMouseApplication.java \
                         ../mona/Mona.java ../mona/NativeFileDescriptor.java
	javac -d . MazeMouse.java MazeMouseApplication.java \
              ../mona/Mona.java ../mona/NativeFileDescriptor.java
	cp ../../resource/images/mouse.gif ../../resource/images/cheese.jpg \
           ../../resource/sounds/squeak.wav mona
	jar cfm MazeMouse.jar maze_mouse_manifest.mf mona
	jarsigner MazeMouse.jar maze_mouse
	mkdir -p ../../lib	
	cp MazeMouse.jar ../../lib

../../bin/evolve_mouse: evolveMouse.cpp evolveMouse.hpp
	$(CC) -L../../lib -o ../../bin/evolve_mouse evolveMouse.cpp \
            -lmona -lcommon -lstdc++

../../bin/socker: socker.cpp
	$(CC) -o ../../bin/socker socker.cpp -lstdc++

clean:
	/bin/rm -fr mona

