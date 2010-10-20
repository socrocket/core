global env
echo $env(GRLIB_HOME)/designs/leon3-gr-xc3s-1500
do $env(GRLIB_HOME)/designs/leon3-gr-xc3s-1500/libs.do
cd $env(GRLIB_HOME)/designs/leon3-gr-xc3s-1500
do compile.vsim
exit
