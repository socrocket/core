#/bin/bash

function runsc() {
  make clean
  echo "=== Building SC Test: $@ ========="
  CPPFLAGS="$@" make systemc 
  echo "=== Running SC Test: $@ ========="
  make exec
}

function runvsim() {
  make clean
  echo "=== Building VSIM Test: $@ =========="
  CPPFLAGS="$@" make modelsim
  echo "=== Running VSIM Test: $@ =========="
  vsim -c -do "vsim -voptargs=+acc work.sc_main;run 1000001000ns"
}

function runtest() {
  echo "== Process Tests: $@ =========="
  runsc -DCTBUS=1 $@
  runsc -DCTBUS=1 -DDEBUGOUT=1 $@
  #runvsim $@
  echo "== End: $@ =========="
  echo
  echo
}

runtest -DSCVAL=6 -DT1VAL=4 -DT2VAL=3 -DT3VAL=4
runtest -DSCVAL=10
runtest -DSCVAL=100
runtest -DSCVAL=1000
runtest -DSCVAL=10000
runtest -DSCVAL=100 -DT1VAL=100
runtest -DSCVAL=1000 -DT1VAL=1000
runtest -DSCVAL=10000 -DT1VAL=10000
runtest -DSCVAL=100 -DT1VAL=100 -DT2VAL=100
runtest -DSCVAL=1000 -DT1VAL=1000 -DT2VAL=1000
runtest -DSCVAL=10000 -DT1VAL=10000 -DT2VAL=10000
runtest -DSCVAL=100 -DT1VAL=100 -DT2VAL=100 -DT3VAL=100
runtest -DSCVAL=1000 -DT1VAL=1000 -DT2VAL=1000 -DT3VAL=1000
runtest -DSCVAL=10000 -DT1VAL=10000 -DT2VAL=10000 -DT3VAL=10000
