#!/bin/bash
# Front-end for instinct executable.

CMD_PATH=`dirname $0`

if [ "$MONA_HOME" = "" ]
then
    MONA_HOME=${CMD_PATH}/..
    export MONA_HOME
fi

if [ "`uname -p 2>/dev/null`" = "sparc" ]
then
    LD_LIBRARY_PATH=${CMD_PATH}/../lib/unix/sparc:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    (cd ${CMD_PATH}/../lib/unix/sparc; /bin/ln -s liballeg-4.2.0.so liballeg.so.4.2 2>/dev/null)
    (cd ${CMD_PATH}/../lib/unix/sparc; /bin/ln -s libfreetype.so libfreetype.so.6 2>/dev/null)
elif [ "`uname -m 2>/dev/null`" = "sparc" ]
then
    LD_LIBRARY_PATH=${CMD_PATH}/../lib/unix/sparc:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    (cd ${CMD_PATH}/../lib/unix/sparc; /bin/ln -s liballeg-4.2.0.so liballeg.so.4.2 2>/dev/null)
    (cd ${CMD_PATH}/../lib/unix/sparc; /bin/ln -s libfreetype.so libfreetype.so.6 2>/dev/null)
elif [ "`uname -s 2>/dev/null`" = "Linux" ]
then
    LD_LIBRARY_PATH=${CMD_PATH}/../lib/unix/intel:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    (cd ${CMD_PATH}/../lib/unix/intel; /bin/ln -s liballeg-4.2.0.so liballeg.so.4.2 2>/dev/null)
fi

if [ "$*" = "" ]
then
    args=""
    echo -n "Enter number of steps: "
    read steps
    args="${args} -steps $steps "
    echo -n "Enter input file (<return> for none): "
    read input
    if [ "$input" != "" ]
    then
        args="${args} -input $input "
    fi
    echo -n "Scramble boxes? (y|n): "
    read scramble
    if [ "$scramble" = "y" ]
    then
        args="${args} -scramble "
    fi
    echo -n "Enter random seed (<return> for default): "
    read random
    if [ "$random" != "" ]
    then
        args="${args} -randomSeed $random "
    fi
    echo -n "Print instincts? (y|n): "
    read print
    if [ "$print" = "y" ]
    then
        args="${args} -printInstincts "
    fi
    echo -n "Display? (text, graphics, or <return> for none): "
    read display
    if [ "$display" != "" ]
    then
        args="${args} -display $display "
    fi
    ${CMD_PATH}/instinct $args
else
    ${CMD_PATH}/instinct $*
fi

exit $?
