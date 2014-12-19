AHBOut - AHB Demonstration Output Device {#ahbout_p}
====================================================
[TOC]

@section ahbout_p1 Overview
The AHBOUT model is a AHB slave that writes all incoming data to a file. 
The class inherits from the classes `AHBSlave` and `CLKDevice`. 
The model can be seen as a reference implementation for an AHB Slave because of its simplicity. 
It has no VHDL reference in the GRLIB hardware library. 
The name of the output file can be specified in the constructor. 

@section ahbout_p2 Interface
This component provides the typical AHB slave generics refactored as constructor parameters of the class AHBOut. 
An overview about the available parameters is given in table 43.

@table Table 43 - AHBOUT Constructor Parameters
| Parameter | Description                                            |
|-----------|--------------------------------------------------------|
| nm        | SystemC name of the module                             |
| haddr     | The 12bit MSB address at the AHB bus                   |
| hmask     | The 12bit address mask for the AHB bus                 |
| ambaLayer | Coding style/abstraction of the model (LT or AT)       |
| slave_id  | The AHB slave bus index.                               |
| outfile   | File name of a text file to initialize the memory from |
@endtable

@section ahbout_p3 Example Instantiation

This example shows how to instantiate the module AHBOUT.
In line 900 the constructor is called to create the new object. 
In line 908 the module is connected to the bus and in the next line the clock is set. 

~~~{.cpp}
AHBOut *ahbout = new AHBOut("ahbout",
  p_ahbout_addr,   // paddr
  p_ahbout_mask,   // pmask
  ambaLayer,
  "outfile.txt"
);

// Connecting APB Slave
ahbctrl.ahbOUT(ahbprof->ahb);
ahbprof->set_clk(p_system_clock,SC_NS);
~~~
