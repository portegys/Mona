This directory contains prototypes of the Mona neural network system.

To build:

UNIX:
Execute 'make' to make executables.

Windows:
Start Microsoft Visual Studio using solution.

Note: Graphical dumps are in the 'dot' format (GraphViz product).

Directories:

1. src:
- mona: mona program.
- common files.
- ant: foraging ant program.
- nest: nest-building birds program.
- java: java programs.
2. bin: executables.

Commands:

1. ant (C++): run foraging ant program.
2. nest (C++): run nest-building birds program.
3. extract_dump (Korn shell): extract dump.
4. extract_graph (Korn shell): extract graph.

Note: The extract commands may be connected together with dot:

# Example: view ant network
if [ $# -ne 1 ]
then
	echo "Usage: $0 <network view iteration>"
	exit 1
fi
v=$1
if [[ "$v" != +([0-9]) ]]
then
	echo "Usage: $0 <network view iteration>"
	exit 1
fi
if [ $v -eq 0 ]
then
	ant -i 1 -d 1 | extract_dump 0 - |
		extract_graph "Foraging ant network" - |
		dot -Tps | pageview -
else
	ant -i $v -d $v | extract_dump 1 - |
		extract_graph "Foraging ant network - iteration = $v" - |
		dot -Tps | pageview -
fi
exit 0
