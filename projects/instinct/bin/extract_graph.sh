#!/bin/ksh
# Extract Mona network graph(s) from input and write to stdout

if [ $# -eq 2 ]
then
	if [ "$1" = "-?" -o "$1" = "-h" ]
	then
		echo "Usage: $0 <\"graph label\"> <file name or \"-\">" >&2
		exit 1
	else
		label="$1"
		if [ "$label" = "" ]
		then
			echo "$0: graph label cannot be empty string" >&2
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
	echo "Usage: $0 <\"graph label\"> <file name or \"-\">" >&2
	exit 1
fi

DIR=/usr/tmp/xng$$
mkdir $DIR
if [ $? -ne 0 ]
then
	echo "$0: cannot make directory $DIR" >&2
	exit 1
fi

if [ "$file" = "-" ]
then
	cd $DIR
	/usr/bin/csplit -f graph -k -n 3 -s - '/^digraph Mona {/' {999} 2>/dev/null
else
	/bin/cp $file $DIR/input
	if [ $? -ne 0 ]
	then
		echo "$0: cannot copy $file to directory $DIR" >&2
		exit 1
	fi
	cd $DIR
	/usr/bin/csplit -f graph -k -n 3 -s input '/^digraph Mona {/' {999} 2>/dev/null
	/bin/rm -f input
fi
/bin/rm -f graph000

for f in *
do
	/bin/grep "label = \"$label" $f >/dev/null
	if [ $? -eq 0 ]
	then
		/bin/cat $f
	fi
done

cd - >/dev/null
rm -fr $DIR

exit 0
