AHBProf - AHB SystemC Profiler {#ahbprof_p}
===========================================
[TOC]

@section ahbprof_p1 Overview

The AHBPROF model provides profiling functionality for the simulator. 
This class inherits from the classes `AHBSlave` and `CLKDevice`. 
Like `AHBIn` and `AHBOut` it has no VHDL reference in the GRLIB hardware library. 

The model creates several registers which are accessible from the simulator by software. 
The registers are used to control an internal mechanism for measuring SystemC time and real execution time.
The registers can be written with following control values:

* 1: start measureing time (simulation time and real time)
* 2: stop measureing time
* 3: print timing report

All control registers are considered to be 32bit wide. 
Register 255 has a special purpose and is reserved for shutting down the simulation. 

@section ahbprof_p2 Interface

This component provides the typical AHB Slave generics refactored as constructor parameters of the class AHBProf. 
An overview about the available parameters is given in table 44.

@table Table 44 - AHBOUT Constructor Parameters
| Parameter | Description                                      |
|-----------|--------------------------------------------------|
| nm        | SystemC name of the module                       |
| index     | The AHB slave bus index                          |
| addr      | The 12bit MSB address at the AHB bus             |
| mask      | The 12bit address mask for the AHB bus           |
| ambaLayer | Coding style/abstraction of the model (LT or AT) |
@endtable

@section ahbprof_p3 Example Instantiation

This example shows how to instantiate the module AHBPROF. 
In line 900 the constructor is called to create the new object. 
In line 908 the module is connected to the bus and in the next line the clock is set. 

~~~{.cpp}
AHBProf *ahbprof = new AHBProf("ahbprof",
  p_ahbprof_index,  // index
  p_ahbprof_addr,   // paddr
  p_ahbprof_mask,   // pmask
  ambaLayer
);

// Connecting APB Slave
ahbctrl.ahbOUT(ahbprof->ahb);
ahbprof->set_clk(p_system_clock,SC_NS);
~~~
