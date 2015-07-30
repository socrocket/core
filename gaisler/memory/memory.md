Memory Models {#memory_p} 
=========================

[TOC]

@section memory_function Functionality and Features

@subsection memory_overview Overview

The Generic Memory (GM) model is not based on any reference design from the Gaisler GRLIB. It was developed from
scratch to complement the SoCRocket MCTRL unit.  The GM comes in two implementation flavors: Map
and array memory. Both provide exactly the same functionality and interfaces, only the internal data representation
differs. The map memory uses a vmap, which can be either a std::map, a hash map or a tr1 hash
map. The array memory stores its data in a flat array. It is recommended to use the map memory for large sparse
memories. For small memories the array implementation yields better performance.  The GM is generic in a sense
that it can act as one of four supported memory types: PROM, IO, SRAM or SDRAM. All memories to be connected to
the MCTRL must be derived from class MemDevice, which encapsulates all configuration options. The MCTRL uses
this interface to determine the features of the attached components.  The GM models default devices, which means
its behaviour is plain functional. All timing related features are provided and controlled by the MCTRL. As any
memory controller needs to know all the timing information of the attached memory device anyway, the delay can be
added in the memory controller to keep the memory itself universally applicable. Respectively, the GM is merely
used to store data and to identify the device on system level.

@subsection memory_power Power Modeling

Power monitoring can be enabled by setting the constructor parameter pow_mon to true. The model is annotated with
default power information that has been gathered using a generic 90nm Standard-Cell Library and statistical power
estimation at Gate-Level.  The accuracy of the build-in power models and the default switching energy settings
cannot be guaranteed. In order to achieve the best possible results the user is recommended to annotate the design
with custom target-technology dependent power information.  The power model of the Generic Memory, all required
parameters, and default settings are explained in the SoCRocket Power Modeling Report [RD11].

@section memory_interface Interface

Both implementations of the Generic Memory can be configured using the same set of constructor parameters listed in table 1.

| Parameter      | Description                                                              |
| -------------- | ------------------------------------------------------------------------ |
| name           | SystemC name of the module                                               |
| type           | MEMDevice::device_type (0 – ROM, 1 - IO, 2 – SRAM, 3 – SDRAM)            |
| banks          |  Number of parallel banks to be modeled                                  |
| bsize          | Size of one memory bank (All banks always considered to have equal size) |
| bits           | Bit width of memory                                                      |
| cols           | Number of SDRAM cols                                                     |
| implementation | BaseMemory::MAP or BaseMemory::ARRAY                                     |
| pow_mon        | Enable power monitoring                                                  |
**Table 1 - Generic Memory Constructor Parameters**

The system-level interface consists of a TLM 2.0 target socket (GreenSocket). The TLM payload comprises an
extension for clearing memory regions (ext_erase). In the current version of the model this feature is only used
by the SDRAM controller.  Since the GM is a plain functional model the communication with the MCTRL is based on
blocking transport (LT).

@section memory_internal Internal Structure

This section describes the internal structure of both Generic Memories. All TLM
functionality is comprised in class Memory. The power estimation functionality is described in MemoryPower, whereas
the base functionality is described in BaseMemory. The storage implementation is in MapStorage or ArrayStorage and
is instatiated according to the constructor parameter in BaseMemory. File ext_erase.h provides an additional
payload extension, which is used by both implementations to organize the clearing of memory regions in SDRAM mode.

@subsection memory_interface_memdevice Interface MemDevice

The Generic Memory implements the interface MEMDevice. The MEMDevice interface enables the MCTRL to identify the
type of the memory (PROM, IO, SRAM, SDRAM) and its main configuration parameters. For access to this parameters
every MEMDevice provides a set of virtual functions, which can be overwritten by the child class. The GM uses the
default implementation of the access functions.  get_type  – Returns the memory type (MEMDevice::device_type)
get_banks   – Returns the number of parallel memory banks get_bsize   – Returns the size of one memory banks
in bytes get_bits  – Returns the width of the memory get_cols  – Returns the number of SDRAM cols

@subsection memory_functional_memory Functional Memory

The storage handling of the GM is implementation dependent. The MapStorage uses a vmap, which can be either a
std::map or a hash map with 32bit wide keys (addresses) and 8bit data entries. The ArrayStorage uses a flat data
array, with address being the index to the data elements. In all cases byte access to memory is performed using
API functions: read, write, read_block, write_block, read_dbg, write_dbg, read_block_dbg, write_block_dbg. The 
*_dbg functions bypass the integrated statistic functions. The access functions are directly called from the 
b_transport method of the model. In case the ext_erase payload extension is set, the respective memory region 
(start – end) is cleared using the erase (erase_dbg) function. This happens when switching SDRAM to 
Deep-Power-Down-Mode or Partial-Self-Refresh.

@section memory_compilation Compilation

The compilation of the GM is integrated in the build system of the library. An appropriate WAF wscript can be
found next to the source files in the ./models/memory directory.  All required objects for simulating the GM and
the MCTRL are compiled in a sub-library named memory using following command: ./waf –target=memory To utilize
the GM in simulations with other components, add memory to the use list of your wscript.

@section memory_example_instantiation Example Instantiation

The example below demonstrates the instantiation of the GM as PROM, IO, SRAM and SDRAM (with ArrayStorage and
MapStorage). The modules are declared in lines 13-16 and created, within the constructor, in lines 30-36. Lines
44-47 show how to bind the bus target socket of the GM to the mem initiator socket of the MCTRL.
 
~~~{.cpp}
#include "core/common/systemc.h"
#include "memory.h"
#include "core/models/mctrl/mctrl.h"

class Top : public sc_module {
  public:
 
    // Memory controller 
    Mctrl mctrl;
 
    // Generic memories
    Memory  rom;
    Memory  io;
    Memory  sram;
    Memory  sdram;
 
    ...
 
    // Constructor
    Top(sc_module_name mn) : sc_module(mn),
 
            ...
 
            // Initialize MCTRL
            mctrl("mctrl", romasel, sdrasel, romaddr, rommask, ioaddr, iomask,
                           ramaddr, rammask, paddr,   pmask,   wprot,  srbanks,
                           ram8,    ram16,   sepbus,  sdbits,  mobile, sden, 
                    0, 0, 0, amba::amba_LT),
            // Initialize PROM
            rom("rom", MEMDevice::ROM, 2, 512 * 1024 * 1024 / 2, 32, 0, BaseMemory::MAP, false),
            // Initialize IO
            io("io", MEMDevice::IO, 1, 512 * 1024 * 1024, 32, 0, BaseMemory:MAP, false),
            // Initialize SRAM
            sram("sram", MEMDevice::SRAM, 4, 512 * 1024 * 1024 / 4, 32, 16, BaseMemory:ARRAYfalse),
            // Initialize SDRAM
            sdram("sdram", MEMDevice::SDRAM, 2, 512 * 1024 * 1024 / 2, 32, 16, BaseMemory::ARRAY, false) {
            
        ...
 
        // Set MCTRL timing for delay calculation
        mctrl.set_clk(10, SC_NS);
 
        // Connect MCTRL to Generic Memories
        mctrl.mem(rom.bus);
        mctrl.mem(io.bus);
        mctrl.mem(sram.bus);
        mctrl.mem(sdram.bus);
~~~
