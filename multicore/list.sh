#!/bin/bash
start=0
inc=1

echo nr, real time in ticks, simulated time in ns
for (( i=$start ; i<=108 ; i=i+$inc ))
do
  cfg=$(printf '%03d' ${i})
  if [ -e cfg.${cfg}.sim.log ] ; then
    (echo $i ; cat cfg.${cfg}.sim.log | grep "real time (15)" | cut -d'=' -f2 ; cat cfg.${cfg}.sim.log | grep "simulated time (15)" | cut -d'=' -f2 | sed 's/ns//g' ) | sed 's/s//g' | sed ':a;N;$!ba;s/\n/, /g'
  fi
done
