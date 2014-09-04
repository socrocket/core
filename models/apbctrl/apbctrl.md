APBCtrl - AHB to APB Bridge {#apbctrl_p}
========================================
[TOC]

@section apbctrl_p1 Functionality and Features

@subsection apbctrl_p1_1 Overview

The APBCTRL TLM model can be used to simulate behavior and timing of the GRLIB APBCTRL AHB-to-APB Bridge VHDL IP. 
The model is available at two levels of abstractions (LT and AT). 
For modeling the APBCTRL we mostly follow the recommendations given in RD06.
All details regarding APB protocol modeling at transaction level 
(payload structure, TLM phase mapping) can be found in the SoCRocket Interconnect Methodology [RD9].

@subsection apbctrl_p1_2 Address Decoding

For address decoding the TLM APBCTRL uses the same arithmetic as the GRLIB VHDL model. 
Each APB slave provides a configuration record identifying its address range. 
This is done using two parameters: `paddr` and `pmask`. 
The `paddr` represents the 12bit APB base address of the device. 
The `pmask` parameter indicates the size of the address range. 
If `addr` is the 12 bit APB address (bits 20 – 8 of absolute address) 
of a transaction following logic equation must be solved:

~~~
select = (addr ^ paddr) & pmask
~~~

Address addr falls in the range of the slave, if select equals zero.

@subsection apbctrl_p1_3 Plug & Play Support

The TLM APBCTRL supports the Plug & Play (PNP) mechanism described in *RD04*. 
APB configuration records and access functions are implemented in class APBDevice. 
Each slave connected to the APBCTRL must be derived from this class. 
The PNP information of the slaves is collected at `start_of_simulation()`. 
The combined information is mapped on a read-only area at the top 4kBytes of the bridge address space. 

@subsection apbctrl_p1_4 Power Monitoring

Power monitoring can be enabled by setting the constructor parameter `pow_mon` to `true`. 
The model is annotated with default power information that has been gathered using a 
generic 90nm Standard-Cell Library and statistical power estimation at Gate-Level.

The accuracy of the build-in power models and the default switching energy settings cannot be guaranteed. 
In order to achieve the best possible results the user is recommended 
to annotate the design with custom target-technology dependent power information.

The power model of the APBCTRL, all required parameters, 
and default settings are explained in the SoCRocket Power Modeling Report [RD11].

@section apbctrl_p2 Interface

The GRLIB VHDL model of the APBCTRL is configured using Generics. 
For the implementation of the TLM model most of these Generics were 
refactored to constructor parameters of class APBCtrl. 
An overview about the available parameters is given in Table 13.

Parameter | Description
--------- | -----------------------------------------------------------------------
nm        | SystemC name of the module
haddr     | The 12bit MSB address at the AHB bus
hmask     | The 12bit address mask for the AHB bus
mcheck    | Check if there are any intersections between APB slave memory regions
hindex    | The AHB bus index
pow_mon   | Enable power monitoring
ambaLayer | Coding style/abstraction of the model (LT or AT)
*Table 13 - APBCTRL Constructor Parameters*

The system-level interface of the APBCTRL comprises an AHB slave socket (`ahb`) and an APB master socket (`apb`). 
The APB socket can be bound to multiple slaves (multi-socket), while the AHB socket may be bound to only one master. 
Depending on the constructor parameter `ambaLayer` the ahb socket is configured for blocking (LT) or non-blocking (AT) communication. 
The `ambaLayer` parameter has no effect on the apb socket. For the sake of performance the APB communication is modeled using blocking transport only. 
In case of LT configuration a TLM blocking transport function is registered at the ahb socket. 
For the AT abstraction the model provides a TLM non-blocking forward transport function. 
Additionally, the model contains a debug transport function for non-intrusive code execution (TRAP) and checking. 
The signatures of all transport functions are compliant with the TLM2.0 standard. 
Moreover, the module inherits SignalKit inputs for clock cycle time (`clk`) and reset (`rst`) from class CLKDevice. 
APBCtrl is also derived from class AHBDevice. 
Hence, it exposes a PNP configuration record, which is mapped into the configuration area of AHBCtrl.

@section apbctrl_p3 Internal Structure

This section describes the internal structure of the APBCtrl. 
The class hierarchy of the model is flat. 
All functionality is comprised in class APBCtrl, 
which is described in the files `apbctrl.h` and `apbctrl.cpp`.

@subsection apbctrl_p3_1 Decoder initialization

Similar to the AHBCTRL, the address decoder of the APBCTRL is based on a routing table implemented in form of a `std::map`. 
The `std::map APBCtrl::slave_map` is initialized in function APBCtrl::start_of_simulation(). 
The function iterates through all slaves bound to socket `apb`. 
If the slave is a valid APB Device (must be derived from class APBDevice) the module creates a new address entry in `APBCtrl::slave_map`. 
The function also copies the configurartion information of the attached slaves into a 32bit wide array (`mSlaves`). 
This array is mapped in the configuration area of the APBCTRL (as described in RD04), where any bus master can access it.

@subsection apbctrl_p3_2 LT behaviour

Compared to AHB, APB is a rather simple protocol. 
From the perspective of an AHB bus master the APBCTRL is an ordinary slave device. 
The APBCTRL does not do any arbitration. 
Moreover, APB communication is not pipelined. 
Therefore, the `ambaLayer` constructor parameter only affects the AHB slave interface of the APBCTRL. 
The APB socket uses blocking communication. 

Most of the behaviour of the APBCTRL is encapsulated in a single function (`APBCtrl::exec_decoder`). 
In LT mode this function is directly called from `b_transport`. 
The `exec_decoder` function first checks whether the incoming transaction is directed toward the configuration area or not. 
In the first case the `APBCtrl::getPNPReg` function is used to access the APB configuration records (`APBCtrl::mSlaves`). 
The APB configuration area is read-only. 
Write operations cause a `TLM_COMMAND_ERROR_RESPONSE`. 
In the second case `APBCtrl::exec_decoder` calls `APBCtrl::get_index`. 
The `APBCtrl::get_index` function receives the address of the transaction as an input argument and returns the id of the slave binding (`index`). 
For this reason `get_index` iterates through the previously described `slave_map`. 
In case no slave can be found the function returns `-1`. 
This produces a `TLM::TLM_ADDRESS_ERROR_RESPONSE` and an error message will be written to `stdout`. 
In case of success the transaction is send to the identified slave by calling its `APBCtrl::b_transport` function:

~~~{.cpp}
apb[index]->b_transport( *trans, delay);
~~~

Since APBCTRL is a bus bridge, the payload event needs to be copied. 
In this process the segment address of the bridge (`haddr`) is removed from address field of the transaction.

The LT APBCTRL adds one cycle of delay to the transaction in order to approximate the delay of the APB setup phase. 
The delay may be consumed by the slave or added to the latency of the target. 
The LT APBCTRL does not synchronize with the SystemC kernel. 
The transaction delay is returned to the master, who is responsible for consuming the passed time.

@subsection apbctrl_p3_3 AT behaviour

The AT mode is intended to more accurately approximate the timing of the GRLIB APBCTRL hardware model. 
This is achieved by respecting the pipelined nature of the AHB protocol. 
In AT mode the APBCTRL contains two SystemC threads. 
A routing table is not required, because the communication on the APB side is always blocking. 
Hence, no more than one transaction can be active on the APB at any time.

A new transaction arrives in `nb_transport_fw` with phase `BEGIN_REQ`. 
The function enters the transaction in the `mAcceptPEQ` payload event queue. 
After consumption of the component accept delay, `mAcceptPEQ` triggers the `acceptTXN` thread. 
The latter is responsible for sending `END_REQ` to the AHBCTRL. 
This is the signal for the AHBCTRL that the AHB address phase is completed. 

In case of a read transaction, `acceptTXN` forwards the transaction to the `processTXN` thread (via the `mTransactionPEQ` payload event queue). 
`ProcessTXN` calls the `exec_decoder` function, which has already been described above (see LT behaviour). 
After the control has returned from the slave device, `processTXN` sends `BEGIN_RESP` on the backward path. 
Afterwards, the transaction is considered complete. 
An eventual `END_REQ` from the master will be ignored.

If the transaction indicates a write operation, the `mTransactionPEQ` is written from the `nb_transport_fw` function, after reception of `BEGIN_DATA`. 
This also triggers the `processTXN` thread and a call to `exec_decoder`. 
After return from `exec_decoder` `END_DATA` is send on the backward path. 
This completes the AHB data phase.

For more informationon about the AHB AT implementation please see RD09.

@section apbctrl_p4 Compilation

For the compilation of the APBCTRL unit, a WAF `wscript` file is provided and integrated in the superordinate build mechanism of the library.
All required objects for simulating the APBCTRL on platform level are compiled in a sub-library name apbctrl using following build command:

~~~{.sh}
./waf –target=apbctrl
~~~

To utilize APBCtrl in simulations with other components, add APBCtrl the use list of your `wscript`.

@section apbctrl_p5 Example Instantiation

This example shows how to instantiate the module APBCTRL. The APBCTRL is a bridge between the AHB and the APB portion of the AMBA bus system. The component is created in lines 36-41. In line 46 the module is bound to the master socket of the AHBCTRL. Line 49 binds a slave, here the control interface of the MCTRL, to the master socket of the APBCTRL. Similar to the AHBCTRL the APBCTRL needs a notion of time. Hence, it inherits the clock interface of class CLKDevice. In this examples the clock cycle time is set in line 55.

~~~{.cpp}
#include "core/common/amba.h"

#include "core/models/ahbctrl/ahbctrl.h"
#include "core/models/apbctrl/apbctrl.h"
#include "genericmemory.h"
#include "core/models/mctrl/mctrl.h"
#include "testbench.h"

#include "core/common/systemc.h"

using namespace std;
using namespace sc_core;

class Top : public sc_module {
  public:

    // *** DECLARE MODULES

    // Testbench master
    Testbench testbench;

    // AHB bus model
    AHBCtrl ahbctrl;

    // APB Bridge
    APBCtrl apbctrl;

    ...

    // Constructor
    Top(sc_module_name nm) : sc_module(nm),

      ...
      ahbctrl("ahbctrl", 0xfff, 0xfff, 0xff0, 0xff0, 0, 0, 0, 0, 0, 1, 0, 0, amba::amba_LT),

      apbctrl("apbctrl",   // SystemC name
               0x800,      // AHB base address
               0xfff,      // AHB address maks
               true,       // mcheck - Check consistency of address map
               1,          // hindex - AHB bus index
               amba::amba_LT) {

        ...

        // APB bridge to AHB bus
        ahbctrl.ahbOUT(apbctrl.ahb);

        // Memory controller to APB bus
        apbctrl.apb(mctrl.apb);

        ..

        // Set clock
        ahbctrl.set_clk(10,SC_NS);
        apbctrl.set_clk(10,SC_NS);

    }

    virtual ~Top() {}
};
~~~
