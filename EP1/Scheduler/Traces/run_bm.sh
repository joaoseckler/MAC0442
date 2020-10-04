#! /bin/bash

# Create output dir
if [ ! -d output ]
then
  mkdir output
fi

EP1=../ep1
TRACES="trace1.txt trace2.txt trace3.txt"
REPS=30

echo "Starting at `date -Iseconds`" > log.txt
for REP in `seq -w $REPS`
do
  for ALG in 1 2 3
  do
    for TRACE in $TRACES
    do
      $EP1 $ALG input/$TRACE output/${TRACE%.txt}_alg${ALG}_${REP}.txt d
      echo "`date -Iseconds` $EP1 $ALG input/$TRACE output/${TRACE%.txt}_alg${ALG}_${REP}.txt" >> log.txt
    done
 done
done
echo "Ended at `date -Iseconds`" >> log.txt
