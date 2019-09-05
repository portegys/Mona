#!/bin/bash
# Trim unneeded files.

CMD_PATH=`dirname $0`

if [ "`uname -p 2>/dev/null`" = "sparc" ]
then
    /bin/rm -fr ${CMD_PATH}/../lib/win32
    /bin/rm -fr ${CMD_PATH}/../lib/unix/intel
elif [ "`uname -m 2>/dev/null`" = "sparc" ]
then
    /bin/rm -fr ${CMD_PATH}/../lib/win32
    /bin/rm -fr ${CMD_PATH}/../lib/unix/intel
elif [ "`uname -s 2>/dev/null`" = "Linux" ]
then
    /bin/rm -fr ${CMD_PATH}/../lib/win32
    /bin/rm -fr ${CMD_PATH}/../lib/unix/sparc
else
    /bin/rm -fr ${CMD_PATH}/../lib/win32
    /bin/rm -fr ${CMD_PATH}/../lib/unix
fi

exit 0
