#***********************************************************************#
#* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *#
#*                                                                     *#
#* File:       Makefile                                                *#
#*             This Makefile contains the build logic to build the     *#
#*             RTL and TLM Testbenches.                                *#
#*                                                                     *#
#* Modified on $Date$   *#
#*          at $Revision$                                         *#
#*                                                                     *#
#* Principal:  European Space Agency                                   *#
#* Author:     VLSI working group @ IDA @ TUBS                         *#
#* Maintainer: Rolf Meyer                                              *#
#***********************************************************************#

all: modelsim systemc

libs: modelsim/grlib modelsim/techmap modelsim/gaisler modelsim/work
vhdl: modelsim/.grlib modelsim/.techmap modelsim/.gaisler modelsim/.work
systemc: systemc/.dir systemc/sim.x
modelsim: modelsim/.link

clean:
	-rm -rf modelsim vsim.wlf systemc

# SystemC #

SOURCES=tests/top_tlm.cpp
OBJECTS=$(patsubst %.cpp,systemc/%.o, $(SOURCES))
CXX = g++
CXXFLAGS = -g -Wall -D_REENTRANT -DUSE_STATIC_CASTS -DSC_INCLUDE_DYNAMIC_PROCESSES
GREENREG_DIRS = -I$(GREENSOCS_HOME)/greenreg -I$(GREENSOCS_HOME)/greenreg/framework/core/analysis -I$(GREENSOCS_HOME)/greenreg/framework/core -I$(GREENSOCS_HOME)/greenreg/framework/devices -I$(GREENSOCS_HOME)/greenreg/framework/registers -I$(GREENSOCS_HOME)/greenreg/sysc_ext/kernel -I$(GREENSOCS_HOME)/greenreg/sysc_ext/utils -I$(GREENSOCS_HOME)/greenreg/sysc_ext/communication -I$(GREENSOCS_HOME)/greenreg/internal/device -I$(GREENSOCS_HOME)/greenreg/internal/registers/register -I$(GREENSOCS_HOME)/greenreg/internal/registers/bit -I$(GREENSOCS_HOME)/greenreg/internal/registers/register_data -I$(GREENSOCS_HOME)/greenreg/internal/registers/bit_range -I$(GREENSOCS_HOME)/greenreg/internal/registers -I$(GREENSOCS_HOME)/greenreg/greenreg_socket -I$(GREENSOCS_HOME)/greenreg/utils -I$(GREENSOCS_HOME)/greenreg/utils/stl_ext -I$(GREENSOCS_HOME)/greenreg/utils/patterns -I$(GREENSOCS_HOME)/greenreg/utils/storage -I$(GREENSOCS_HOME)/greenreg/gs
INCLUDES = -I. -I.. -I../utils -I$(SYSTEMC_DIR) -I$(TLM2_DIR) -I$(SCV_DIR) -I$(BOOST_DIR) -I$(GREENSOCS_HOME) -I$(GREENSOCS_HOME)/greensocket -I$(GREENSOCS_HOME)/gsgpsocket -I$(GREENSOCS_HOME)/greencontrol $(GREENREG_DIRS) -I$(AMBA_HOME) -I$(GREENSOCS_HOME)/signalsocket/green-signal-socket/include

LDFLAGS = 
LIBDIRS = -L. -L.. -L$(SCV_LIB) -L$(SYSTEMC_LIB) -L$(BOOST_LIB) -L$(GREENSOCS_HOME)/greenreg
LIBRARIES = -lpthread -lm -lscv -lsystemc -lgreenreg

GRSOURCES=$(shell find /home/hwswsim/tools/greensocs-4.0.0/greenreg/ -name *.cpp)

exec: systemc
	systemc/sim.x
	-vcd2wlf register.vcd register.wlf
	-vcd2wlf signals.vcd signals.wlf

systemc/.dir:
	mkdir -p systemc
	@touch $@

systemc/%.o: %.cpp systemc/.dir
	-mkdir -p $(shell dirname $@)
	$(CXX) $(CXXFLAGS) -DSPEEDTEST $(CPPFLAGS) $(INCLUDES) -c -o $@ $<

#systemc/gpimer.o: gptimer.cpp gptimer.h
#	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<  

systemc/top_tlm.o: tests/top_tlm.cpp tests/top_tlm.h gptimer.h gptimer.tpp

systemc/sim.x: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(LIBDIRS) -o $@ $(OBJECTS) $(LIBRARIES) 2>&1 | c++filt

# Modelsim #

modelsim/vsys/.dir: modelsim/.dir
	vlib modelsim/vsys
	@touch $@

vsys: 
	vlib modelsim/work
	sccom -nologo -work work -D_REENTRANT -DUSE_STATIC_CASTS -DSC_INCLUDE_DYNAMIC_PROCESSES -I/opt/tools/modelsim6.6/modeltech/include/systemc -I. -I.. -I/home/hwswsim/tools/include/boost-1_37 -I/home/hwswsim/tools/greensocs-4.0.0 -I/home/hwswsim/tools/greensocs-4.0.0/greensocket -I/home/hwswsim/tools/greensocs-4.0.0/gsgpsocket -I/home/hwswsim/tools/greensocs-4.0.0/greencontrol -I/home/hwswsim/tools/greensocs-4.0.0/greenreg -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/framework/core/analysis -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/framework/core -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/framework/devices -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/framework/registers -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/sysc_ext/kernel -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/sysc_ext/utils -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/sysc_ext/communication -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/internal/device -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/internal/registers/register -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/internal/registers/bit -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/internal/registers/register_data -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/internal/registers/bit_range -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/internal/registers -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/greenreg_socket -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/utils -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/utils/stl_ext -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/utils/patterns -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/utils/storage -I/home/hwswsim/tools/greensocs-4.0.0/greenreg/gs -I/home/hwswsim/tools/amba_socket-modelsim -scv top_LT.cpp
	sccom -nologo -work work -link -L$(GREENSOCS_HOME)/greenreg -scv -lgreenreg
	vsim -do 'vsim -voptargs=+acc work.sc_main; add wave -r sim:/sc_main/*; run 1000ns;'
  
wave: modelsim
	vsim -do 'vsim -voptargs=+acc work.sc_main; add wave -r sim:/sc_main/*; run 1000ns;'

vsim: modelsim
	vsim -c -do 'vsim -voptargs=+acc work.sc_main; run -all;'

modelsim/.dir:
	vlib modelsim
	@touch $@

modelsim/grlib/.dir: modelsim/.dir
	vlib modelsim/grlib
	@touch $@

modelsim/techmap/.dir: modelsim/.dir
	vlib modelsim/techmap
	@touch $@

modelsim/gaisler/.dir: modelsim/.dir
	vlib modelsim/gaisler
	@touch $@

modelsim/work/.dir: modelsim/.dir
	vlib modelsim/work
	@touch $@

modelsim/.grlib: modelsim/grlib/.dir
	vcom -quiet -explicit -93 -work  grlib ../../grlib/lib/grlib/stdlib/version.vhd
	vcom -quiet -explicit -93 -work  grlib ../../grlib/lib/grlib/stdlib/stdlib.vhd
	vcom -quiet -explicit -93 -work  grlib ../../grlib/lib/grlib/amba/amba.vhd
	vcom -quiet -explicit -93 -work  grlib ../../grlib/lib/grlib/amba/devices.vhd
	@touch $@

modelsim/.techmap: modelsim/techmap/.dir modelsim/.grlib
	vcom -quiet -explicit -93 -work  techmap ../../grlib/lib/techmap/gencomp/gencomp.vhd
	@touch $@

modelsim/.gaisler: modelsim/gaisler/.dir modelsim/.techmap modelsim/.grlib
	vcom -quiet -explicit -93 -work  gaisler ../../grlib/lib/gaisler/misc/misc.vhd
	vcom -quiet -explicit -93 -work  gaisler ../../grlib/lib/gaisler/misc/gptimer.vhd
	@touch $@

#modelsim/.work: modelsim/work/.dir modelsim/.gaisler modelsim/.grlib
#	vcom -quiet -explicit -93 -work  work gptimer_wrapper.vhd
#	@touch $@

modelsim/gptimer.h: modelsim/.gaisler
	scgenmod -modelsimini modelsim.ini -lib gaisler -bool -map "std_logic_vector=sc_uint" -createtemplate gptimer > $@

modelsim/.testbench: modelsim/work/.dir modelsim/gptimer.h tests/top.h tests/top_rtl.cpp tests/prescalertest.h
	sccom -nologo -work work $(CPPFLAGS) -I. -Itests -I../utils -I../../signalkit -Imodelsim -I/home/group/socrocket/tools/amba_socket-modelsim -I/home/group/socrocket/tools/greensocs-4.0.0 -I$(TLM2_HOME)/unit_test/tlm/multi_sockets/include -scv tests/top_rtl.cpp
	@touch $@

modelsim/.link: modelsim/.grlib modelsim/.techmap modelsim/.gaisler modelsim/.testbench
	sccom -nologo -work work -link -scv
	@touch $@

