AHBMem - AHB Memory {#ahbmem_p}
==================================
[TOC]

@section ahbmem_p1 Overview
The AHBMEM model provides a simple flat memory implementation bound to an AHB slave interface. 
The class inherits from the AHBSlave and CLKDevice classes.

@section ahbmem_p2 Interface
In the manner of the GRLIB VHDL models this component provides the typical AHB Slave Generics refactored as Constructor parameters of the class AHBMem. 
An overview about the available parameters is given in table 42.

@table Table 42 - AHBMEM Constructor Parameters
| Parameter   | Description                                            |
|-------------|--------------------------------------------------------|
| nm          | SystemC name of the module                             |
| haddr       | AHB address of the AHB slave socket (12 bit)           |
| hmask       | AHB address mask (12 bit)                              |
| ambaLayer   | Abstraction layer used (AT/LT)                         |
| slave_id    | The AHB slave bus index                                |
| cachable    | Device cacheable or not                                |
| wait_states | Number of wait states to be inserted for each transfer |
| pow_mon     | Enable power monitoring                                |
@endtable

@section ahbmem_p3 Example Instantiation

This example shows how to instantiate the module AHBMEM. 
In line 547 the constructor is called to create the new object. 
In line 559 the module is connected to the bus and in the next line the clock is set. 

~~~{.cpp}
AHBMem *ahbmem = new AHBMem("ahbmem",
                            p_ahbmem_addr,
                            p_ahbmem_mask,
                            ambaLayer,
                            p_ahbmem_index,
                            p_ahbmem_cacheable,
                            p_ahbmem_waitstates,
                            p_report_power

);

// Connect to ahbctrl and clock
ahbctrl.ahbOUT(ahbmem->ahb);
ahbmem->set_clk(p_system_clock, SC_NS);
~~~
