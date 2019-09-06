#!/bin/bash
# Run maze_maker to find test-worthy meta-mazes.
# Vary door and rooms;
# Doors 3,5
# Rooms 5,10,15
steps=10
echo steps=$steps
mazes=10
echo mazes=$mazes
instances=10
echo instances=$instances
numoutput=10
echo numoutput=$numoutput
numrooms=10
echo numrooms=$numrooms
deltarooms=0
echo deltarooms=$deltarooms
numdoors=5
echo numdoors=$numdoors
deltadoors=0
echo deltadoors=$deltadoors
numgoals=1
echo numgoals=$numgoals
deltagoals=0
echo deltagoals=$deltagoals
contextsizes=`expr $numrooms + $numrooms`
echo contextsizes=$contextsizes
deltacontext=$numrooms
echo deltacontext=$deltacontext
effectdelay=`expr $numrooms + $numrooms`
echo effectdelay=$effectdelay
deltaeffect=$numrooms
echo deltaeffect=$deltaeffect
./maze_maker.exe -randomSeed 7 -steps $steps -mazes $mazes -instances $instances -numOutput $numoutput -numRooms $numrooms -deltaRooms $deltarooms -numDoors $numdoors -deltaDoors $deltadoors -twoway -markPath -numGoals $numgoals -deltaGoals $deltagoals -contextSizes $contextsizes -deltaContextSizes $deltacontext -effectDelayScale $effectdelay -deltaEffectDelayScale $deltaeffect

