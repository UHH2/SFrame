#!/usr/bin/env bash
#
# Script to run sframe_main many times, and produce stats about runtime
#
# Run with: ./timeSFrame.sh <arguments to pass to sframe_main>

NRUNS=50
declare -a RESULTSREAL
declare -a RESULTSCPU

echo "Running sframe_main with args: $@"

for (( i=0; i<$NRUNS; i++))
do
    echo $i
    sframe_main $@ &> "tmp$i.txt"
    line=$(grep "Real time" "tmp$i.txt")
    real=$(echo $line | sed -r 's/^.*Real time +([0-9.]+).*$/\1/')
    cpu=$(echo $line | sed -r 's/^.*CPU time +([0-9.]+).*$/\1/')
    RESULTSREAL[$i]=$real
    RESULTSCPU[$i]=$cpu
    echo ${RESULTSREAL[$i]}, ${RESULTSCPU[$i]}
done

# calculate mean & rms for real & cpu times
# doing floating-point maths is horrific, have to pipe commands into bc
# as bash can't handle floating-point
totalreal=0
for i in ${RESULTSREAL[@]}; do
  totalreal=$(echo "scale=3;$totalreal + $i" | bc)
done
avereal=$(echo "scale=3;$totalreal / $NRUNS" | bc)

totalrealsubsq=0
for i in ${RESULTSREAL[@]}; do
  totalrealsubsq=$(echo "scale=3;$totalrealsubsq + ($i-$avereal)^2" | bc)
done
rmsreal=$(echo "scale=3;sqrt($totalrealsubsq / $NRUNS)" | bc)


totalcpu=0
for i in ${RESULTSCPU[@]}; do
  totalcpu=$(echo "scale=3;$totalcpu + $i" | bc)
done
avecpu=$(echo "scale=3;$totalcpu / $NRUNS" | bc)

totalcpusubsq=0
for i in ${RESULTSCPU[@]}; do
  totalcpusubsq=$(echo "scale=3;$totalcpusubsq + ($i-$avecpu)^2" | bc)
done
echo $totalcpusubsq
rmscpu=$(echo "scale=3;sqrt($totalcpusubsq / $NRUNS)" | bc)

echo "Over $NRUNS runs: average real: $avereal +- $rmsreal, average cpu: $avecpu +- $rmscpu"
