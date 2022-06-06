# The Mona neural network makefile

MAKEFLAGS += s

all: mona muzz minc pong
	@echo "make all done"

mona:
	@(chmod 755 bin/*.sh)
	@echo "Making common..."
	@(cd src/common; make)
	@echo "Making Mona..."
	@(cd src/mona; make)
	@echo "done"

muzz:
	@echo "Making graphics..."
	@(cd src/graphics; make)
	@echo "Making GUI..."
	@(cd src/gui; make)
	@echo "Making Muzz world..."
	@(cd src/muzz; make)
	@echo "done"

minc:
	@echo "Making lens..."
	@(cd lens; make)
	@echo "Making Minc world..."
	@(cd src/minc; make)
	@echo "done"

pong:
	@echo "Making Pong game world..."
	@(cd src/pong; make)
	@echo "done"

java:
	@(chmod 755 bin/*.sh)
	@echo "Making common..."
	@(cd src/common; make)
	@echo "Making Mona..."
	@(cd src/mona; make)	
	@(cd src/mona; make java)
	@echo "Making Minc world viewer..."
	@(cd src/minc; make java)
	@echo "Making Pong game app..."
	@(cd src/pong; make java)
	@echo "Making maze-learning mouse..."
	@(cd src/maze_mouse; make)
	@echo "done"

help:
	@echo "Targets: all mona muzz minc pong java tarball zip clean"

tarball: tar
	@echo "Creating mona_5_2.tgz file..."
	@/bin/rm -fr mona mona_5_2.tgz
	@/bin/mkdir mona
	@/bin/mv mona.tar mona
	@(cd mona; tar -xf mona.tar; /bin/rm mona.tar)
	@tar -cf mona.tar mona
	@/bin/rm -fr mona
	@gzip mona.tar
	@/bin/mv mona.tar.gz mona_5_2.tgz
	@echo "done"

zip: tar
	@echo "Creating mona_5_2.zip file..."
	@/bin/rm -fr mona mona_5_2.zip
	@/bin/mkdir mona
	@/bin/mv mona.tar mona
	@(cd mona; tar -xf mona.tar; /bin/rm mona.tar)
	@find mona -print | zip mona_5_2 -@
	@/bin/rm -fr mona
	@echo "done"

tar:
	@/bin/rm -f mona.tar
	@ls [Rr]eadme.txt [Mm]akefile .cproject .project *.sln 2>/dev/null > tarfiles.txt
	@find src -name '*.h' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.hpp' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.c' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.cpp' -print 2>/dev/null >> tarfiles.txt
	@find src -name '[Mm]akefile*' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.sln' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.vcproj' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.vcxproj' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.vcxproj.filters' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.vcxproj.user' -print 2>/dev/null >> tarfiles.txt
	@find src -name [Rr]eadme.txt -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.java' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.mf' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.cs' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.snk' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.csproj' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.csproj.user' -print 2>/dev/null >> tarfiles.txt
	@find src -name '*.html' -print 2>/dev/null >> tarfiles.txt
	@find src/maze_mouse -name '*.bat' -print 2>/dev/null >> tarfiles.txt
	@find src/maze_mouse -name '*.sh' -print 2>/dev/null >> tarfiles.txt
	@find lens -name '*.h' -print 2>/dev/null >> tarfiles.txt
	@find lens -name '*.hpp' -print 2>/dev/null >> tarfiles.txt
	@find lens -name '*.c' -print 2>/dev/null >> tarfiles.txt
	@find lens -name '*.cpp' -print 2>/dev/null >> tarfiles.txt
	@find lens -name '*.tcl' -print 2>/dev/null >> tarfiles.txt
	@find lens -name '[Mm]akefile*' -print 2>/dev/null >> tarfiles.txt
	@find lens -name '*.sln' -print 2>/dev/null >> tarfiles.txt
	@find lens -name '*.vcproj' -print 2>/dev/null >> tarfiles.txt
	@find lens -name '*.vcxproj' -print 2>/dev/null >> tarfiles.txt
	@find lens -name '*.vcxproj.filters' -print 2>/dev/null >> tarfiles.txt
	@find lens -name [Rr]eadme.txt -print 2>/dev/null >> tarfiles.txt
	@find lens -name '*.java' -print 2>/dev/null >> tarfiles.txt
	@find lens -name dummy.txt -print 2>/dev/null >> tarfiles.txt
	@find bin -name '*.bat' -print 2>/dev/null >> tarfiles.txt	
	@find bin -name '*.sh' -print 2>/dev/null >> tarfiles.txt
	@find resource -type f -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.h' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.hpp' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.c' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.cpp' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '[Mm]akefile*' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.sln' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.vcproj' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.vcxproj' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.vcxproj.filters' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.vcxproj.user' -print 2>/dev/null >> tarfiles.txt
	@find projects -name [Rr]eadme.txt -print 2>/dev/null >> tarfiles.txt
	@find projects -name dummy.txt -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.java' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.mf' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.cs' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.snk' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.csproj' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.csproj.user' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.html' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.xml' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.bat' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.sh' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.jpg' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.png' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.bmp' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.resx' -print 2>/dev/null >> tarfiles.txt
	@find projects -name '*.ico' -print 2>/dev/null >> tarfiles.txt
	@tar -cf mona.tar -T tarfiles.txt
	@/bin/rm -f tarfiles.txt

# Uses uncrustify (uncrustify.sourceforge.net)
beautify:
	@echo "Beautifying..."
	-@(cd src/common; uncrustify *.h *.c *.hpp *.cpp 2>/dev/null; uncrustify *.java 2>/dev/null; uncrustify *.cs 2>/dev/null)
	-@(cd src/mona; uncrustify *.h *.c *.hpp *.cpp 2>/dev/null; uncrustify *.java 2>/dev/null; uncrustify *.cs 2>/dev/null)
	-@(cd src/graphics; uncrustify *.h *.c *.hpp *.cpp 2>/dev/null; uncrustify *.java 2>/dev/null; uncrustify *.cs 2>/dev/null)
	-@(cd src/muzz; uncrustify *.h *.c *.hpp *.cpp 2>/dev/null; uncrustify *.java 2>/dev/null; uncrustify *.cs 2>/dev/null)
	-@(cd src/maze_mouse; uncrustify *.h *.c *.hpp *.cpp 2>/dev/null; uncrustify *.java 2>/dev/null; uncrustify *.cs 2>/dev/null)
	-@(cd src/minc; uncrustify *.h *.c *.hpp *.cpp 2>/dev/null; uncrustify *.java 2>/dev/null; uncrustify *.cs 2>/dev/null)
	-@(cd src/pong; uncrustify *.h *.c *.hpp *.cpp 2>/dev/null; uncrustify *.java 2>/dev/null; uncrustify *.cs 2>/dev/null)
	@echo "done"

dos2unix:
	@echo "Converting to UNIX format..."
	@chmod 755 bin/dos2unix.sh
	@bin/dos2unix.sh
	@(cd src/common; ../../bin/dos2unix.sh)
	@(cd src/mona; ../../bin/dos2unix.sh)
	@(cd src/graphics; ../../bin/dos2unix.sh)
	@(cd src/gui; ../../bin/dos2unix.sh)
	@(cd src/gui/glpng; ../../../bin/dos2unix.sh)
	@(cd src/gui/glpng/png; ../../../../bin/dos2unix.sh)
	@(cd src/gui/glpng/zlib; ../../../../bin/dos2unix.sh)
	@(cd src/muzz; ../../bin/dos2unix.sh)
	@(cd src/maze_mouse; ../../bin/dos2unix.sh)
	@(cd src/minc; ../../bin/dos2unix.sh)
	@(cd src/pong; ../../bin/dos2unix.sh)
	@echo "done"

clean:
	@(cd src/common; make clean)
	@(cd src/mona; make clean)
	@(cd src/graphics; make clean)
	@(cd src/gui; make clean)
	@(cd src/muzz; make clean)
	@(cd src/maze_mouse; make clean)
	@(cd lens; make clean)
	@(cd src/minc; make clean)
	@(cd src/pong; make clean)
