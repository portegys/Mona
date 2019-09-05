# Build instinct UNIX executables.
# Special makefile are for Allegro graphics.

if [ "`uname -p 2>/dev/null`" = "sparc" ]
then
    make -f makefile_sparc
elif [ "`uname -m 2>/dev/null`" = "sparc" ]
then
    make -f makefile_sparc
elif [ "`uname -s 2>/dev/null`" = "Linux" ]
then
    make -f makefile_intel
else
    make -f makefile
fi
exit $?
