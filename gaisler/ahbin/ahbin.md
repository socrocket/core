AHBIn - AHB Demonstration Input Device {#ahbin_p}
=================================================
[TOC]

@section ahbin_p1 Overview

The AHBIN model generates input data for the AHB bus. 
The input data consists of fixed size random data frames, which are written to a certain address in a certain interval. 
The class inherits from the `AHBMaster` and `CLKDevice` classes. 
The model can be seen as a reference implementation of an AHB Master, since it is very simple. 
It has no VHDL reference in the Gaisler Library.

@section ahbin_p2 Interface

This component provides the typical AHB master generics refactored as constructor parameters of the class `AHBOut`. 
An overview about the available parameters is given in table 41.

@table Table 41 - AHBIN Constructor Parameters
| Parameter | Description                                     |
|-----------|-------------------------------------------------|
| name      | SystemC name of the module                      |
| hindex    | The master index for registering with the AHB   |
| hirq      | The number of the IRQ raised for available data |
| framesize | The size of the data frame to be generated      |
| frameaddr | The address the data is supposed to be copied   |
| interval  | The interval between data frames                |
| pow_mon   | Enable power monitoring                         |
| ambaLayer | TLM abstraction layer                           |
@endtable

@section ahbin_p3 Example Instantiation

This example shows how to instantiate the module AHBIN. 
In line 590 the constructor is called to create the new object. 
In line 601 the module is connected to the bus and in the next line the clock is set. 
In line 605 the interrupt output is connected via Signalkit. 

~~~{.cpp}
AHBIn *ahbin = new AHBIn("ahbin",
    p_ahbin_index,
    p_ahbin_irq,
    p_ahbin_framesize,
    p_ahbin_frameaddr,
    sc_core::sc_time(p_ahbin_interval, SC_MS),
    p_report_power,
    ambaLayer
);

// Connect sensor to bus
ahbin->ahb(ahbctrl.ahbIN);
ahbin->set_clk(p_system_clock, SC_NS);

// Connect interrupt out
signalkit::connect(irqmp.irq_in, ahbin->irq, p_ahbin_irq);
~~~
