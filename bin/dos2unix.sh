# Convert source files from DOS to UNIX format.

for i in `ls [Rr]eadme.txt [Mm]akefile* *.h *.c *.hpp *.cpp *.java *.cs 2>/dev/null`
do
    dos2unix $i
done

exit 0
