set OLDCLASSPATH=%CLASSPATH%
set CLASSPATH=.
rmdir /s /q mona
javac -d . InstinctDemoStarter.java
jar cvf InstinctDemoStarter.jar mona InstinctDemoStarter.java
@echo
@echo To sign InstinctDemoStarter.jar file use keytool to create mona key:
@echo keytool -genkey -alias mona -keypass your_password
@echo Then uncomment jarsigner command
REM jarsigner InstinctDemoStarter.jar mona
set CLASSPATH=%OLDCLASSPATH%
set OLDCLASSPATH=

