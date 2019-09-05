#!/bin/ksh
# Extract a Mona network dump occurrence from input and write to stdout

if [ $# -eq 2 ]
then
	if [ "$1" = "-?" -o "$1" = "-h" ]
	then
		echo "Usage: $0 <dump occurrence> <file name or \"-\">" >&2
		exit 1
	else
		num="$1"
		if [[ "$num" != +([0-9]) ]]
		then
			echo "$0: dump occurrence must be non-negative number" >&2
			exit 1
		fi
		file="$2"
		if [ "$file" != "-" ]
		then
			if [ ! -r "$file" ]
			then
				echo "$0: file $file not readable" >&2
				exit 1
			fi
		fi
	fi
else
	echo "Usage: $0 <dump occurrence> <file name or \"-\">" >&2
	exit 1
fi

DIR=/usr/tmp/xnd$$
mkdir $DIR
if [ $? -ne 0 ]
then
	echo "$0: cannot make directory $DIR" >&2
	exit 1
fi

if [ "$file" = "-" ]
then
	cd $DIR
	/usr/bin/csplit -f dump -k -n 3 -s - '/^%%%_Begin_network_dump_%%%/' {999}  2>/dev/null
else
	/bin/cp $file $DIR/input
	if [ $? -ne 0 ]
	then
		echo "$0: cannot copy $file to directory $DIR" >&2
		exit 1
	fi
	cd $DIR
	/usr/bin/csplit -f dump -k -n 3 -s input '/^%%%_Begin_network_dump_%%%/' {999} 2>/dev/null
	/bin/rm -f input
fi
/bin/rm -f dump000

(( n=${num}+1 ))
file=$(/bin/ls -1 * 2>/dev/null | /bin/sed -n -e "${n}p" 2>&1)
if [ ! -r "$file" ]
then
	echo "$0: occurrence $num not found" >&2
	exit 1
fi
/bin/mv $file input
/bin/rm -f dump*

/usr/bin/csplit -f dump -k -n 3 -s input '/^%%%_End_network_dump_%%%/' 2>/dev/null
/usr/bin/tail +2 dump000

cd - >/dev/null
rm -fr $DIR

exit 0
