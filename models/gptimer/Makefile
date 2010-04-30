all: modelsim systemc

libs: modelsim/grlib modelsim/techmap modelsim/gaisler modelsim/work
vhdl: modelsim/.grlib modelsim/.techmap modelsim/.gaisler modelsim/.work
systemc: systemc/.dir systemc/sim.x
modelsim: modelsim/.link

clean:
	-rm -rf modelsim vsim.wlf systemc

# SystemC #

SOURCES=top_LT.cpp gptimer.cpp
OBJECTS=$(patsubst %.cpp,systemc/%.o, $(SOURCES))
CXX = g++
CXXFLAGS = -g -Wall -D_REENTRANT -DUSE_STATIC_CASTS -DSC_INCLUDE_DYNAMIC_PROCESSES
GREENREG_DIRS = -I$(GREENSOCS_HOME)/greenreg -I$(GREENSOCS_HOME)/greenreg/framework/core/analysis -I$(GREENSOCS_HOME)/greenreg/framework/core -I$(GREENSOCS_HOME)/greenreg/framework/devices -I$(GREENSOCS_HOME)/greenreg/framework/registers -I$(GREENSOCS_HOME)/greenreg/sysc_ext/kernel -I$(GREENSOCS_HOME)/greenreg/sysc_ext/utils -I$(GREENSOCS_HOME)/greenreg/sysc_ext/communication -I$(GREENSOCS_HOME)/greenreg/internal/device -I$(GREENSOCS_HOME)/greenreg/internal/registers/register -I$(GREENSOCS_HOME)/greenreg/internal/registers/bit -I$(GREENSOCS_HOME)/greenreg/internal/registers/register_data -I$(GREENSOCS_HOME)/greenreg/internal/registers/bit_range -I$(GREENSOCS_HOME)/greenreg/internal/registers -I$(GREENSOCS_HOME)/greenreg/greenreg_socket -I$(GREENSOCS_HOME)/greenreg/utils -I$(GREENSOCS_HOME)/greenreg/utils/stl_ext -I$(GREENSOCS_HOME)/greenreg/utils/patterns -I$(GREENSOCS_HOME)/greenreg/utils/storage -I$(GREENSOCS_HOME)/greenreg/gs
INCLUDES = -I. -I.. -I$(SYSTEMC_DIR) -I$(TLM2_DIR) -I$(SCV_DIR) -I$(BOOST_DIR) -I$(GREENSOCS_HOME) -I$(GREENSOCS_HOME)/greensocket -I$(GREENSOCS_HOME)/gsgpsocket -I$(GREENSOCS_HOME)/greencontrol $(GREENREG_DIRS) -I$(AMBA_HOME)

LDFLAGS = 
LIBDIRS = -L. -L.. -L$(SCV_LIB) -L$(SYSTEMC_LIB) -L$(BOOST_LIB) -L$(GREENSOCS_HOME)/greenreg
LIBRARIES = -lpthread -lm -lscv -lsystemc -lgreenreg

exec: systemc
	systemc/sim.x

systemc/.dir:
	mkdir -p systemc
	@touch $@

systemc/%.o: %.cpp systemc/.dir
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

systemc/gpimer.o: gptimer.cpp gptimer.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<  

systemc/top_LT.o: top_LT.cpp testbench.h gptimer.h

systemc/sim.x: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(LIBDIRS) -o $@ $(OBJECTS) $(LIBRARIES) 2>&1 | c++filt

# Modelsim #
  
wave: modelsim
	vsim -do 'vsim -voptargs=+acc work.sc_main; add wave -r sim:/sc_main/*; run 1000ns;'

vsim: modelsim
	vsim

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

modelsim/.testbench: modelsim/work/.dir modelsim/gptimer.h testbench.h top_CT.cpp
	sccom -nologo -work work -I. -Imodelsim -I/home/hwswsim/tools/amba_socket-modelsim -I/home/hwswsim/tools/greensocs-4.0.0 -scv top_CT.cpp
	@touch $@

modelsim/.link: modelsim/.grlib modelsim/.techmap modelsim/.gaisler modelsim/.testbench
	sccom -nologo -work work -link -scv
	@touch $@
