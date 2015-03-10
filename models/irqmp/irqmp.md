Irqmp - Interrupt Controler for multiple Processors {#irqmp_p}
==============================================================
[TOC]

@section irqmp_p1 Functionality and Features

@subsection irqmp_p1_1 Overview

The SystemC IRQMP unit models behaviour and timing of the IRQMP VHDL model from the Aeroflex/Gaisler GRLIB ([GRLIB IP Core User’s Manual](http://gaisler.com/products/grlib/grip.pdf)). 
Purpose of the IP is the priorization and masking of the interrupts from all AHB and APB devices in the system. 
The interrupt with the highest priority is propagated to one or multiple processors. 
Up to 16 LEON3 cores are supported. Two different modes of IR distribution are implemented:

 * The IR is forwarded to all cores and cleared by the first core that acknowledges the IR (i.e. the ISR is processed only once).
 * The IR is broadcasted and has to be acknowledged (and processed) by each of the cores.

Interrupts can be masked for each core separately. 
The data path or the IRQMP unit is not pipelined, 
i.e. all operations can be performed within one clock cycle.

The GRLIB interrupt scheme comprises a 32-bit interrupt (IR) bus, 
which is routed in parallel to the AMBA bus signals. 
The 16 LSBs of the IR bus are associated to regular IRs and the 16 MSBs to extended IRs (EIR). 
In the SoCRocket library IRs are modeled using the TLM SignalKit.
Interrupts from any simulation model can be bound to the IRQMP using the connect method.

@todo is this still correct?

The following command connects the GPTimer interrupt output number 3 (`SignalKit::selector`) with the IRQMP interrupt input number 5.:
~~~{.cpp}
connect(gptimer.irq, irqmp.irq_in, 3, 5);
~~~

In case the sending device has only one interrupt output (`SignalKit::out`) only one channel number (for the IRQMP) must be defined:
~~~{.cpp}
connect(socwire.irq, irqmp.irq_in, 6);
~~~

The connection of the IRQMP towards the processors is implemented in very similar way.

@subsection irqmp_p1_2 Control Registers

The IRQMP can be configured and controlled by a set of memory-mapped registers (Table 32). 
All control registers are 32 bit wide and implemented in form of a register bank. 
Therefore, as described in 3.4, class `Irqmp` is a child of class `APBSlave`. 
An overview about the available registers is given in Table 32.

@table Table 32 – IRQMP Registers
| APB Address Offset | Register                                               |
|--------------------|--------------------------------------------------------|
| 0x00               | Interrupt Level Register                               |
| 0x04               | Interrupt Pending Register                             |
| 0x08               | Interrupt Force Register (NCPU = 0)                    |
| 0x0C               | Interrupt Clear Register                               |
| 0x10               | Multiprocessor Status Register                         |
| 0c14               | Broadcast Register                                     |
| 0x40 + 4 * n       | Processor n Interrupt Mask Register                    |
| 0x80 + 4 * n       | Processor n Interrupt Force Register                   |
| 0xC0 + 4 * n       | Processor n Extended Interrupt Identification Register |
@endtable

All registers can be written to configure or operate the IRQMP unit. 
Only the Extended Interrupt Identification Register is read-only. 
The function and configuration options of the registers are described in full detail in section 66 of [GRLIB IP Core User’s Manual](http://gaisler.com/products/grlib/grip.pdf). 
However, two differences between [GRLIB IP Core User’s Manual](http://gaisler.com/products/grlib/grip.pdf) and the SystemC implementation have to be noted:

1. The Interrupt Force Register for NCPU = 0 has been left out in the SystemC implementation. 
   In a single-processor system the function of the Interrupt Force Register is identical to that of the Interrupt Pending Register.

2. In [GRLIB IP Core User’s Manual](http://gaisler.com/products/grlib/grip.pdf) it is stated that the bits [31..17] of the Interrupt Clear Register are all constantly pulled down to ‘0’. 
   This differs from the VHDL implementation, in which these bits are used for extended interrupt clearance. 
   Respectively, an EIR can also be cleared by software. The SystemC implementation follows the VHDL implementation rather than the manual.

@subsection irqmp_p1_3 Interrupt Prioritization and Forwarding

The IRs are prioritized in a two-dimensional prioritization scheme. 
Both dimensions are referred to as “interrupt level” in [GRLIB IP Core User’s Manual](http://gaisler.com/products/grlib/grip.pdf). 
For clarification purposes, terms will be redefined in this document.

The Interrupt Level Register determines the first dimension of prioritization. 
For each IR line, the according bit in the IR Level Register can be set to level 0 or level 1. 
Each level 1 IR has got a higher priority than any level 0 IR. 
The first dimension of prioritization will be referred to as “interrupt level” throughout this document.

The 16 regular IR lines are modeled with a 16-bit vector. 
The most significant bit (IR15) has got the highest priority and IR1 has got the lowest priority. 
IR0 is reserved. This second dimension of prioritization will be referred to as “interrupt line” throughout this document.

When several IRs are pending, the highest priority IR will be calculated according to the scheme described above. 
Which core receives the interrupt request (IRQ) depends on the settings in the Broadcast Register and the Interrupt Mask Registers of the individual cores. 
As shown in Figure 6, the use of the IR Pending or IR Force Registers is determined by the Broadcast Register.

@todo add figure here from word file

Figure 6 – Interrupt Distribution Scheme

The Interrupt Broadcast Register can be set for each IR line individually. 
If the broadcast bit of an interrupt line is set, the IRQ is sent to all cores and also has to be acknowledged (i.e. the ISR has to be processed) all of them. 
This is accomplished by setting the Interrupt Force Registers of all the cores. 
Each core has to individually clear its Interrupt Force Register!

If the broadcast bit is not set, the IRQ is sent to all cores and has to be acknowledged only once, 
i.e. only the first core that acknowledges the IR has to process the ISR. 
This is done by setting the Interrupt Pending Register, 
which can be cleared by any of the cores. 
In uni-processor systems the Broadcast Register is disabled.

Interrupts can be masked for each core individually. 
If bit n of the Interrupt Mask Register of core m is set to 0, 
then interrupt n is masked for this core, 
i.e. core m will never receive IRQ n. 
As a matter of fact, the VHDL implementation does not prevent an interrupt n clearance by core m in this case. 
For now, the SystemC module has been aligned to this behavior.

Interrupt masking takes place before prioritization, 
so the highest priority unmasked IR is always forwarded to the processors.

Interrupt 15 cannot be masked by the LEON3 core and should be used with care. 
Most operating systems do not safely handle this IR.

@subsection irqmp_p1_4 Extended Interrupt Handling

In the IRQMP extended interrupts are cascaded, 
i.e. one of the regular IR lines may be defined as a cascade for the 16 EIR lines. 
The cascade is defined in bits 19..16 of the Multiprocessor Status Register. 

If EIRs are asserted and the cascade is the highest priority active regular IR, 
the cascade is forwarded to the cores. 
After receiving the interrupt acknowledge signal from a core, 
the IRQMP unit writes the number of the asserted EIR line into the Extended Interrupt Identification Register. 
Thus, the ISR of the cascade has to send the acknowledge signal and afterwards read the EIR ID Register to call the correct ISR of the asserted EIR.

@subsection irqmp_p1_5 Processor Status Monitoring

The processor status can be monitored through the Multiprocessor Status Register. 
The STATUS field [15..0] in this register indicates whether a processor is halted (`1`) or running (`0`). 
A halted processor can be reset and restarted by writing a `1` to its STATUS field. 

After reset, all processors except processor 0 are on halt. 
Once the system is properly initialized, processor 0 may start all other processors by switching on the respective STATUS bits.

To support this mechanism the LEON ISS, which is shipped with this library, has been modified. 
It provides a SignalKit input run and a SignalKit output status. 
Both signals are of type bool and must be routed to the `cpu_rst` and `cpu_stat` ports of the `Irqmp`. 
The Multiprocessor Status Register is kept in sync with the status information provided by the various processors. 

@subsection irqmp_p1_6 Power Modeling

Power monitoring can be enabled by setting the constructor parameter `pow_mon` to `true`. 
The model is annotated with default power information that has been gathered 
using a generic 90nm Standard-Cell Library and statistical power estimation at Gate-Level.

The accuracy of the build-in power models and the default switching energy settings cannot be guaranteed. 
In order to achieve the best possible results the user is recommended to annotate the design with custom target-technology dependent power information.

@todo verify this section and find out about the power modeling report

The power model of the `Irqmp`, all required parameters, and default settings are explained in the SoCRocket Power Modeling Report [RD11].

@section irqmp_p2 Interface

The GRLIB VHDL model of the IRQMP is configured using Generics. 
For the implementation of the TLM model most of these Generics were refactored to constructor parameters of class Irqmp. 
An overview about the available parameters is given in Table 33.

@table Table 33 - Template Parameters
| Parameter | Function                                                                     | Allowed Range   | Default|
|-----------|------------------------------------------------------------------------------|-----------------|--------|
| name      | SystemC name of the module                                                   |                 |        |
| pindex    | Selects which APB select signal (PSEL) will be used to access the IRQMP unit | 0 to NAPBMAX– 1 | 0      |
| paddr     | The 12-bit MSB APB address                                                   | 0 to 4095       | 0      |
| pmask     | The APB address mask                                                         | 0 to 4095       | 4095   |
| ncpu      | Number of processors in multicore systems                                    | 1 to 16         | 1      |
| eirq      | The cascade line of EIRs                                                     | 0 to 15         | 0      |
| pow_mon   | Enable power monitoring                                                      | 0 to 1          | 0      |
@endtable

The system-level interface of the module comprises an GreenSocs/Carbon APB slave socket and multiple SoCRocket SignalKit ports (Table 34). 

@table Table 34 - IRQMP SignalKit sockets
| Name     | Type     | In/Out   | Description                                          |
|----------|----------|----------|------------------------------------------------------|
| rst      | bool     | in       | Reset prescaler and all counters                     |
| clk      | sc_time  | in       | Annotates clock period                               |
| cpu_rst  | bool     | selector | Generate reset for the processor(s)                  |
| cpu_stat | bool     | infield  | Receive status inf. (halt/running) from processor(s) |
| irq_req  | uint32_t | selector | Interrupt requests for the processors(s)             |
| irq_ack  | uint32_t | infield  | Interrupt ackknowledge signals from processors(s)    |
| irq_in   | uint32_t | infield  | Muxed interrupts from IRQ sources                    |
@endtable

@section irqmp_3 Internal Structure

This section describes the internal structure of the `Irqmp`. 
The class hierarchy of the model is flat. 
All functionality is comprised in class `Irpmp`, which is described in the files irqmp.h and irqmp.cpp.

@subsection irqmp_p3_1 The irqmp.h file

The IRQMP unit consists of only one class. 
The irqmp.h file contains the module class definition. 
The parameterization options, implemented as generics in the VHDL model, are realized as constructor parameters of the class. 

Class `Irqmp` is a child of APBSlave. 
A APBSlave is an encapsulation for a complete functional unit and provides containment structures for other elements, e.g. registers. 
Moreover, `Irqmp` inherits the PNP configuration record of class APBDevice, and the clock and reset interface defined in `CLKDevice`.
The Irqmp class definition contains the module interface and the function prototypes of constructor, destructor, and callback functions. 
Next, to the well-known `SC_HAS_PROCESS` macro, the model call `SK_HAS_SIGNALS` for registration with the SignalKit. 

@subsection irqmp_p3_2 The irqmp.cpp file

The constructor of `Irqmp` configures the `APBDevice`, the `gr_device` and the bus interface. 
It constructs a register bank `r`, in which it implements all the registers listed in Table 32. 
The register bank is a C++ class implemented in the sc register libraries that provides memory management and interface functions. 
Within this register bank, a register may be instantiated like in the following code snippet:

~~~{.cpp}
r.create_register("pending", "Interrupt Pending Register",
                  0x04,
                  0x00000000,
                  IRQMP_IR_PENDING_EIP | IRQMP_IR_PENDING_IP,
            );
~~~

The arguments to the `create_register()` function are 
name, description, offset, init value, write mask. 
For a detailed description of these options, please refer to the `r_register` documentation. 

In addition to building the interface, the constructor registers the `SC_THREAD` `Irqmp::launch_irq`. 
The `Irqmp::launch_irq` thread is sensitive to the SystemC event `e_signals` and contains the behavioral core of the model. 
The `e_signals` event is triggered from three locations:

* incoming_irq: Handler bound to irq_in socket, receiving the interrupts from all interrupt sources in the system.
* ackknowledge_irq: Handler bound to irq_ack socket, receiving the ackknowledge  signals from the processors.
* clear_write: Callback bound to Interrupt Clear register
* force_write: Callback bound to Interrupt Force register
* pending_write: Callback bound to Interrupt Pending register

For every state change in one of the observed registers or sockets, 
the launch_irq function recalculates the IR lines for all connected processors. 
This is done in a loop starting from the processor with the highest ID. 
For each processor `launch_irq` combines the pending register with the processor interrupt mask, 
to check whether there is an IR pending. 
It also checks for extended and forced IRs. 
From the resulting mask of IRs, the thread selects the level 1 IR with the highest priority for submission. 
Level 0 IRs are only considered, if there is no level 1 IR waiting. 
The selected IR is written to the processor as follows:
~~~{.cpp}
irq_req.write(1 << cpu, std::pair<uint32_t, bool>(number, true));
~~~

The first argument of the expression selects the processor, 
the second is a `std::pair` consisting of the interrupt number and a `boolean` value. 
The latter defines whether the interrupt line is switched on or off. 
This feature is especially important for RTL co-simulation. 
For plain TLM simulation transmission of the IR number would be sufficient.

@section irqmp_p4 Compilation

For the compilation of the IRQMP unit, a WAF wscript file is provided and integrated in the superordinate build mechanism of the library.
All required objects for simulating the IRQMP are compiled in a sub-library named irqmp using following command:

    $ ./waf –target=irqmp

To utilize the IRQMP in simulations with other components, add irqmp to the use list of your wscript.

@section irqmp_p5 Example Instantiation

The example below demonstrates the instantiation of the IRQMP TL model is a sc_main or an arbitraty top-level class. 
The module is created in line 12. 
Line 15 connects the APB slave socket to the testbench (or APBCTRL). 
Lines 18 – 21 show how to bind the SignalKit sockets directed to the processor side. 
The Interrupt sources (irq_in) are connected in line 26. 
The timing is annotated in line 30. 

~~~{.cpp}
// Define Testbench
Testbench testbench;

// Define IRQMP
Irqmp irqmp;

// Constructor
Top(ModuleName mn) : sc_module(mn),

        // Create Testbench & IRQMP
        testbench("testbench", pindex, paddr, pmask, ncpu, eirq),
        irqmp("irqmp", paddr, pmask, ncpu, eirq, 0) {

        // Bind IRQMP APB socket to testbench
        testbench.apb_mst(irqmp.apb_slv);

        // Connect multiple virtual CPUs (testbenches) to IRQMP
        for(int i=0; i<ncpu; ++i) {
            connect(testbench.cpu_rst, irqmp.cpu_rst, i);
            connect(testbench.irq_req, irqmp.irq_req, i);
            connect(testbench.irq_ack, irqmp.irq_ack, i);
        }

        // Connect multiple interrupt sources to IRQMP
        for(int i=0; i<32; ++i) {
            connect(testbench.irq_out, irqmp.irq_in, i);
        }

        // Annotate timing
        irqmp.set_clk(10.0, SC_NS);
~~~
