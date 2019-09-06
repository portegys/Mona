# JAVA program makefile

all: InstinctDemoStarter.jar ../../bin/instinct_demo_starter

# Instinct demo client starter.
InstinctDemoStarter.jar: InstinctDemoStarter.java
	/bin/rm -fr mona
	javac -d . InstinctDemoStarter.java
	jar cvf InstinctDemoStarter.jar mona InstinctDemoStarter.java
	@echo To sign InstinctDemoStarter.jar file use keytool to create mona key:
	@echo keytool -genkey -alias mona -keypass your_password
	@echo Then uncomment jarsigner command
	#jarsigner InstinctDemoStarter.jar mona

# Instinct demo server starter.
../../bin/instinct_demo_starter: instinct_demo_starter.cpp
	gcc -o ../../bin/instinct_demo_starter instinct_demo_starter.cpp

clean:
	/bin/rm -fr mona *.jar *.sig *.o
