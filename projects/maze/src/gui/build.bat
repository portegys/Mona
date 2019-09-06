set OLDCLASSPATH=%CLASSPATH%
set CLASSPATH=.
rmdir /s /q mona
javac -d . AmazingMouse.java
copy ..\..\resource\mouse.gif mona
copy ..\..\resource\cheese.jpg mona
copy ..\..\resource\squeak.wav mona
jar cvf AmazingMouse.jar mona AmazingMouse.java
@echo
@echo To sign AmazingMouse.jar file use keytool to create mona key:
@echo keytool -genkey -alias mona -keypass your_password
@echo Then uncomment jarsigner command
REM jarsigner AmazingMouse.jar mona
set CLASSPATH=%OLDCLASSPATH%
set OLDCLASSPATH=

