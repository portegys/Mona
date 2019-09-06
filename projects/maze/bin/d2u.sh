# Convert source files from DOS to UNIX format.

for i in `ls makefile *.h *.hpp *.cpp *.java 2>/dev/null`
do
    dos2unix $i $i
done

exit 0
