#!/bin/bash
# Front-end for maze_maker executable.

CMD_PATH=`dirname $0`

if [ "$MONA_HOME" = "" ]
then
    MONA_HOME=${CMD_PATH}/..
    export MONA_HOME
fi

${CMD_PATH}/maze_maker $*

exit $?

