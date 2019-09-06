#!/bin/bash
# Run SNNS (Stuttgart Neural Network Simulator) session.
# Usage runsnns.sh <batch file> <network file> <pattern file/directory> <epochs> [initialize]
PATH=$PATH:/home/teporte/packages/SNNS/SNNSv4.2/tools/bin/sparc-sun-solaris2.9
if [ $# -ne 4 ]
then
  if [ $# -ne 5 ]
  then
    echo 'Usage: runsnns.sh <batch file> <network file> <pattern file/directory> <epochs> [initialize]'
    exit 1
  fi
fi
runtype="resume"
if [ $# -eq 5 ]
then
runtype=$5
  if [ "$runtype" != "initialize" ]
  then
    echo run type must be either \"initialize\" or unset
    exit 1
  fi
fi
batchfile=$1
network=$2
if [ "$network" != "network.net" ]
then
  cp $network network.net
fi
find $3 -print > patterns.txt
epochs=$4
e=0
echo saved network file: network.net
echo appended results file: results.res
echo appended log file: batch.log
while [ $e -lt $epochs ]
do
  rm -f tmp.log
  patterns=`./randomize_strings < patterns.txt`
  for p in $patterns
  do
    if [ -f $p ]
    then
      cp $p pattern.pat
      if [ "$runtype" = "initialize" ]
      then
        sed -e "s/#setInitFunc/setInitFunc/" < $batchfile  | \
          sed -e "s/#initNet/initNet/" > batch.bat
        runtype="resume"
      else
        cp $batchfile batch.bat
      fi
      batchman -f batch.bat -l tmp.log
      cat tmp.log >> batch.log
      rm -f tmp.log
    fi
  done
  e=`expr $e + 1`
done
