# The Mona program makefile

FILES = README.txt makefile prototypes.sln bin/extract* \
	src/mona/*.h src/mona/*.hpp src/mona/*.cpp src/mona/makefile \
	src/common/*.h src/common/*.hpp src/common/*.cpp src/common/makefile \
	src/ant/*.h src/ant/*.hpp src/ant/*.cpp src/ant/makefile ant.vcxproj ant.vcxproj.filters \
	src/nest/*.h src/nest/*.hpp src/nest/*.cpp src/nest/makefile ant.vcxproj.filters \
	src/java/*.java src/java/makefile

# Windows files.
WINFILES = \
	winant/ReadMe winant/*.h winant/*.hpp winant/*.cpp winant/*.dsp winant/*.dsw \
	winnest/ReadMe winnest/*.h winnest/*.hpp winnest/*.cpp winnest/*.dsp winnest/*.dsw

PRINT = lp

all:
	@echo "Making common..."
	@(cd src/common; make)
	@echo "Making mona..."
	@(cd src/mona; make)
	@echo "Making ant..."
	@(cd src/ant; make)
	@echo "Making nest..."
	@(cd src/nest; make)
	@echo "Making java..."
	@(cd src/java; make)
	@echo "done"

print:
	@echo "Printing mona source code..."
	$(PRINT) $(FILES)
	@echo "done"

cpio:
	@echo "Creating mona.cpio file..."
	@/bin/ls -d $(FILES) src/java/*.gif doc/* | cpio -ovcdm > mona.cpio
	@echo "done"

zip:
	@echo "Creating mona.zip file..."
	@/bin/ls -d $(FILES) src/java/*.gif doc/* $(WINFILES) | zip mona -@
	@echo "done"

clean:
	(cd src/common; make clean)
	(cd src/mona; make clean)
	(cd src/ant; make clean)
	(cd src/nest; make clean)
	(cd src/java; make clean)
