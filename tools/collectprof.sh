#!/bin/bash
start=${1}
inc=1
end=${2}
dir=${3}

echo nr, real time in ticks, simulated time in ns
for (( i=$start ; i<=$end ; i=i+$inc ))
do
  cfg=$(printf '%03d' ${i})
  if [ -e ${dir}cfg.${cfg}.sim.log ] ; then
    (echo $i ; cat ${dir}cfg.${cfg}.sim.log | grep "real time (15)" | cut -d'=' -f2 ; cat ${dir}cfg.${cfg}.sim.log | grep "simulated time (15)" | cut -d'=' -f2 | sed 's/ns//g' ) | sed 's/s//g' | sed ':a;N;$!ba;s/\n/, /g'
  fi
done
