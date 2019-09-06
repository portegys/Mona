# JAVA program makefile

all: AmazingMouse.jar

# Maze-learning mouse.
AmazingMouse.jar: AmazingMouse.java
	/bin/rm -fr mona
	javac -d . AmazingMouse.java
	cp ../../resource/mouse.gif ../../resource/cheese.jpg ../../resource/squeak.wav mona
	jar cvf AmazingMouse.jar mona AmazingMouse.java
	@echo To sign AmazingMouse.jar file use keytool to create mona key:
	@echo keytool -genkey -alias mona -keypass your_password
	@echo Then uncomment jarsigner command
	#jarsigner AmazingMouse.jar mona

clean:
	/bin/rm -fr mona *.jar *.sig *.o
