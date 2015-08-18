APBUART - APB UART {#apbuart_p}
=============================
[TOC]

@section apbuart_p1 Overview

The APBUART model creates an UART device, which is mapped to a TCP port on the host system. 
The class inherits from the classes `gs::reg::gr_device`, `APBDevice` and `CLKDevice`. 
This model can be used as a serial console for exchanging data or printing debug information. 
The model has several registers which are listed in table 45. Further information can be found in [GRLIB IP Core User’s Manual](http://gaisler.com/products/grlib/grip.pdf) (chapter 16).

@table Table 45 - APBUART Registers
| APB address offset | Register              |
|--------------------|-----------------------|
| 0x0                | UART Data register    |
| 0x4                | UART Status register  |
| 0x8                | UART Control register |
| 0xC                | UART Scaler register  |
@endtable

@section apbuart_p2 Interface

The GRLIB VHDL model of the APBUART is configured using Generics. 
For the implementation of the TLM model most of these Generics were refactored to constructor parameters of class `AHBUART`. 
An overview about the available parameters is given in table 46.

@table Table 46 - APBUART Constructor Parameters
| Parameter | Description                                                                                                                                                                           |
|-----------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| name      | SystemC name of the module                                                                                                                                                            |
| backend   | Selects the IO backend to be used, currently only TCP available                                                                                                                       |
| pindex    | APB slave index                                                                                                                                                                       |
| paddr     | ADDR field of the APB BAR                                                                                                                                                             |
| pmask     | MASK field of the APB BAR                                                                                                                                                             |
| pirq      | Index of the interrupt line                                                                                                                                                           |
| console   | Prints output from the UART on console during VHDL simulation and speeds up simulation by always returning ‘1’ for Data Ready bit of UART Status register. Does not effect synthesis. |
| powmon    | Enable power monitoring                                                                                                                                                               |
@endtable

@section apbuart_p3 Example Instantiation

This example shows how to instantiate the module `APBUART`. 
In line 873 the constructor is called to create the new object. 
In line 882 the module is connected to the bus and in line 886 the clock is set. 
In line 884 the interrupts are connected via Signalkit. 

~~~{.cpp}
APBUART *apbuart = new APBUART(sc_core::sc_gen_unique_name("apbuart", false), io,
  index,           // index
  addr,            // paddr
  mask,            // pmask
  irq,             // pirq
  p_report_power   // powmon
);

// Connecting APB Slave
apbctrl.apb(apbuart->bus);
// Connecting Interrupts
signalkit::connect(irqmp.irq_in, apbuart->irq, irq);
// Set clock
apbuart->set_clk(p_system_clock,SC_NS);
~~~
