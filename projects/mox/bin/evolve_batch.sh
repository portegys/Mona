# Evolution batch run.
if [ $# -ne 4 ]
then
    echo "Usage: $0 <number of runs> <population> <number of worlds> <random seed>"
    exit 1
fi
numRuns=$1
population=$2
numWorlds=$3
randomSeed=$4
i=0
while [ $i -lt $numRuns ]
do
    loadCells=""
    j=0
    while [ $j -lt $numWorlds ]
    do
        n=`expr $RANDOM % 50`
        loadCells=`echo $loadCells -loadCells ../work/mox_worlds/cells10x10_${n}.txt`
        j=`expr $j + 1`
    done
    echo "Run=${i}"
    ./evolve.sh -generations 30 -steps 50 -randomSeed $randomSeed -moxPopulations $population $loadCells -output ../work/evolve_${population}_worlds${numWorlds}_${i}.out -logfile ../work/evolve_${population}_worlds${numWorlds}_${i}.log
    i=`expr $i + 1`
done
exit 0