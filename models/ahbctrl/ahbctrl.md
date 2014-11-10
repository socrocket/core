AHBCtrl - AHB Controler {#ahbctrl_p}
====================================

[TOC]

@section ahbctrl_p1 Functionality and Features

@subsection ahbctrl_p1_1 Overview

The AHBCTRL TLM model can be used to simulate behavior and timing of the GRLIB AHB Controller VHDL IP. 
The model is available at two levels of abstractions (LT and AT). 
All details regarding AHB protocol modeling at transaction level (payload structure, TLM phase mapping) can be found in the @ref interconnect_methodology_ahb "SoCRocket Interconnect Methodology".

@subsection ahbctrl_p1_2 Address Decoding

For address decoding the TLM AHBCTRL uses the same arithmetic as the GRLIB VHDL model. 
Each slave in the system provides a configuration record identifying its address range. 
This is done using two parameters: `haddr` and `hmask`. 
The `haddr` parameter represents the 12bit MSB base address of the device. 
The `hmask` parameter indicates the size of the address range. 
If `addr` is the 12 bit MSB address of a transaction following logic equation must be solved:

~~~
select = (addr ^ haddr) & hmask
~~~

Address `addr` falls in the address range of the slave if `select` equals zero.

@subsection ahbctrl_p1_3 Arbitration

At AT abstraction the AHBCTRL supports two modes of arbitration: round robin and priority based. 
Arbitration mode can be selected by setting the `rrobin` constructor parameter. 
In fixed priority mode (`rrobin = 0`), the bus request priority is equal to the masters’s bus index: 
the lower the index, the higher the priority. 
In round robin mode, priority is rotated one step after each AHB transfer. 
This is implemented as a modulo counter, which can be found in function `AHBCtrl::arbitrate_me`.

@subsection ahbctrl_p1_4 Plug & Play Support

The TLM AHBCTRL supports the Plug & Play (PNP) mechanism described in [GRLIB IP Core User’s Manual](http://gaisler.com/products/grlib/grip.pdf). 
AHB configuration records and access functions are implemented in class `AHBDevice`. 
Each master and slave to be connected to the bus model must be derived from this class. 
The PNP information of the slaves is collected at `AHBCtrl::start_of_simulation` (@ref ahbctrl_p1_3 "Arbitration"). 
The combined information is mapped to the address range defined by the constructor parameters `cfgaddr` and `cfgmask`. 
By default, this relates to addresses `0xfffff000 – 0xffffffff`. 
The master information is placed in the first 2kB block and the slave information in the second 2kB block of the device. 
For internal use all master information is aligned in integer array `mMasters`, while slave information can be found in `mSlaves`.

@subsection ahbctrl_p1_5 Snooping

The TLM AHBCTRL supports dbus snooping. 
Address, length and master id of any write access will be broadcasted through the SignalKit output AHBCtrl::snoop. 
In LT mode this is done in the blocking transport function (`AHBCtrl::b_transport`), which is registered at socket `AHBCtrl::ahbIN`. 
The AT mode implements snooping within the `SC_THREAD` `AHBCtrl::DataThread`. 
The `DataThread` is triggered by the non-blocking transport forward function (`nb_transport_fw`) on reception of phase `BEGIN_DATA`.
The snooping information can be broadcasted to all relevant bus masters, 
by binding the snoop output of the ahbctrl to multiple snooping inputs (similar to multi-socket). 
The example below shows how to connect the snooping signal to four instances of type mmu_cache:

~~~{.cpp}
// Connect snooping (broadcast) 
ahbctrl.snoop(cache0.snoop); 
ahbctrl.snoop(cache1.snoop); 
ahbctrl.snoop(cache2.snoop); 
ahbctrl.snoop(cache3.snoop); 
~~~

@subsection ahbctrl_p1_6 Power Monitoring

Power monitoring can be enabled by setting the constructor parameter `pow_mon` to `true`. 
The model is annotated with default power information that has been gathered 
using a generic 90nm Standard-Cell Library and statistical power estimation at Gate-Level.
The accuracy of the built-in power models and the default switching energy settings cannot be guaranteed. 
In order to achieve the best possible results the user is recommended to annotate the design with custom target-technology dependent power information.

@todo is this power modeling report still accurate?

The power model of the AHBCTRL, all required parameters, and default settings are explained in the SoCRocket Power Modeling Report (RD11).

@section ahbctrl_p2 Interface

The GRLIB VHDL model of the AHBCTRL is configured using Generics. 
For the implementation of the TLM model most of these Generics were refactored to constructor parameters of class AHBCtrl. 
An overview about the available parameters is given in Table 12.

Parameter | Description
--------- | -----------------------------------------------------------------
nm        | SystemC name
ioaddr    | The 12bit MSB address of the AHB I/O area
iomask    | The 12bit address mask of the AHB I/O area
cfgaddr   | The 12bit MSB address of the AHB configuration area (PNP)
cfgmask   | The 12bit address mask of the AHB configuration area (PNP)
rrobin    | Arbitration mode: 1 – round robin, 0 – priorities (AT only)
split     | Enables AHB SPLIT response (AT only)
defmast   | ID of the default master
ioen      | Enable AHB I/O area
fixbrst   | Enable support for fixed-length bursts
fpnpen    | Enable full decoding of PNP configuration records
mcheck    | Check if there are any intersections between core memory regions.
pow_mon   | Enable power monitoring
ambaLayer | Coding style/abstraction of model (LT or AT)
*Table 12 - AHBCTRL Constructor Parameters*

The system-level interface of the TLM AHBCTRL comprises an AHB master (`AHBCtrl::ahbOUT`) and an AHB slave socket (`AHBCtrl::ahbIN`). 
Both of them enable the connection to multiple masters and slaves (multi-sockets). 
Depending on the constructor parameter `ambaLayer` the sockets are configured for blocking (LT) or non-blocking (AT) communication. 
In the LT case the module registers a TLM blocking transport function at `ahbIN`. 
For the AT abstraction the model provides a TLM non-blocking forward transport function for the `ahbIN` socket and a TLM non-blocking backward transport function for the `ahbOUT` socket. 
In any case the model registers a TLM debug transport function. 
Within the current release of the library, debug transport is mainly used for non-intrusive code execution from the LEON-ISS. 
The signatures of all transport functions are compliant with the TLM2.0 standard.

Next to the TLM sockets the model comes with SignalKit inputs for clock cycle time (`AHBCtrl::clk`) and reset (`AHBCtrl::rst`), as well as a SignalKit output for snooping (`AHBCtrl::snoop`). 
The `AHBCtrl::clk` and `AHBCtrl::rst` inputs are inherited from class `CLKDevice`, while `AHBCtrl::snoop` is directly defined in `AHBCtrl`.

@section ahbctrl_p3 Internal Structure

This section describes the internal structure of the AHBCTRL. 
The class hierarchy of the model is flat. 
All functionality is comprised in class `AHBCtrl`, which is described in the files ahbctrl.h and ahbctrl.cpp.

@subsection ahbctrl_p3_1 Decoder initialization

The address decoder of the TLM AHBCTRL is based on a routing table implemented as a std::map. 
The std::map slave_map contains the index and address information of all the slaves connected to the AHBCTRL. 
It is initialized in function `AHBCtrl::start_of_simulation`. 
The function iterates through all slaves bound to socket `AHBCtrl::ahbOUT`. 
If the slave is a valid AHB Device (must be derived from class `AHBDevice`) the module creates one address entry in slave_map per base address register (BAR). 
There can be at most four sub-devices/BARs per slave. 
If the constructor parameter fpenen is enabled, the start_of_simulation function also copies the PNP information of any connected module (masters and slaves) into two 32bit wide arrays (mSlaves / mMasters). 
These arrays are mapped into the configuration area of the AHBCTRL (as described in [GRLIB IP Core User’s Manual](http://gaisler.com/products/grlib/grip.pdf)), where they can be accessed by any bus master.

@subsection ahbctrl_p3_2 LT behaviour

In LT mode the AHBCTRL is a simple address decoder. 
All incoming transactions will be directly forwarded to their targets, without any arbitration done. 
The decoder is located in the b_transport function. 
Transactions may be directed to the internal configuration area (PNP) or to one of the connected slaves. 
The configuration area is read-only. For access to the slave memory range, AHBCtrl::b_transport calls AHBCtrl::get_index. 
The get_index function receives the address of the transaction as an input argument and returns the id of the slave binding (index). 
For this reason get_index iterates through the previously described slave_map. 
In case no slave can be found the function returns -1. 
This produces a TLM_ADDRESS_ERROR_RESPONSE and an error message will be written to stdout. 
In case of success, the transaction is send to the identified slave by calling its b_transport function:

~~~{.cpp}
ahbOUT[index]->b_transport(trans, delay);
~~~

The LT AHBCTRL adds one cycle of delay to the transaction in order to approximate the delay of the AHB address phase. 
The delay may be consumed by the slave device or added to the latency of the target. 
The LT AHBCTRL does not synchronize with the SystemC kernel. 
The transaction delay is returned to the master, who is responsible for consuming the passed time.

@subsection ahbctrl_p3_3 AT behaviour

The AT mode is intended to more accurately approximate the timing of the GRLIB AHBCTRL hardware model. 
To facilitate architecture exploration features like arbitration and pipelining are taken into account. 
Therefore, the AT mode of the AHBCTRL is more complex. 
It e.g. requires multiple parallel SC_THREADs. 
The operation of the module can be best understood by following the control flow of a transaction.

A new transaction arrives in nb_transport_fw with phase BEGIN_REQ. 
The function will first create a new connection record. 
A connection record consists of the master_id (bus id of master), the slave_id (bus id of slave) and a connection state. 
While the master_id is known, the slave_id still needs to be determined during decode. 
Hence, at this point in time, slave_id is set to zero. 
The initial connection state is PENDING. 
The AHBCTRL keeps track of all transactions using the data structure pending_map. 
New entries are created by function addPendingTransaction. 

In the next step the thread arbitrate_me decides which master will receive the bus in the current cycle. 
This will be done at intervals of clock_cycle ns. 
The default clock_cycle time is 10 ns. 
This setting can be overwritten by connecting a clock to input clk or by one of the set_clk functions of class CLKDevice. 
Depending on constructor parameter rrobin the transaction with the highest priority (lowest index) or the one pointed by the robin counter is selected. 
All other transactions have to wait. 
If there is a winner, the respective transaction is entered in the mRequestPEQ payload event queue. 
Their transaction state is set to BUSY.

Now the transaction is ready for address decoding. 
This is done in thread RequestThread. 
The same mechanisms are used as for LT operation (get_index). 
The connection record is updated with the index of the slave device. 
If the transaction is not directed towards the configuration area and a valid slave could be found,  it is forwarded to socket ahbOUT:

~~~{.cpp}
status = ahbOUT[index]->nb_transport_fw( *trans, pase, delay)
~~~

The slave may now respond in multiple different ways. 
The modules of this library either return TLM_UPDATED with phase END_REQ or TLM_ACCEPTED with phase BEGIN_REQ. 
In the first case the RequestThread sends END_REQ to the master. 
In the second it waits for event mEndRequest, which will be triggered as soon nb_transport_bw receives END_REQ from the slave. 
This completes the address phase of the protocol.

In case of read transaction the slave is expected to continue by sending BEGIN_RESP. 
If BEGIN_RESP is received by nb_transport_bw, the transaction unblocks the ResponseThread via the mResponsePEQ payload event queue. 
The ResponseThread uses the pending_map to find back the respective connection record including the index of the master. 
Afterwards, BEGIN_RESP is send to the master. 
The master can now copy the data and reply with either TLM_ACCEPTED and BEGIN_RESP, TLM_UPDATE and END_RESP or TLM_COMPLETED. 
In the first case the thread will wait for END_RESP to be send on the forward path. 
This is indicated by event mEndResponseEvent. 
In all other cases the transaction is considered completed and removed from the pending_map.

For more information on the AHB AT implementation please see @ref interconnect_methodology_ahb "Interconnect Methodology".

@section ahbctrl_p4 Compilation

For the compilation of the AHBCTRL unit, a WAF wscript file is provided and integrated in the superordinate build mechanism of the library.
All required objects for simulating the AHBCTRL on platform level are compiled in a sub-library named ahbctrl using following build command:

    $ ./waf -–target=ahbctrl

To utilize ahbctrl in simulations with other components, add ahbctrl to the use list of your wscript.

@section ahbctrl_p5 Example Instantiation

The example below demonstrates the instantiation of the AHBCTRL inside a sc_main method or an arbitrary top-level class. 
The instantiating module needs to include at least ahbctrl.h and amba.h. 
The AHBCTRL is created in line 19-32.  
In line 40 the slave port (`AHBCtrl::ahbIN`) of the bus is bound to a testbench master. 
Line 43 shows how to bind a slave to the master socket (`AHBCtrl::ahbOUT`). 
Both bus master and slave socket support multiple bindings. 

All additional components are to be connected in equal way. 
How to bind the snoop Signalkit output is shown in line 46. 
Since the AHBCTRL has some internal storage (config area), 
it needs a notion of time. 
In this example the clock cycle time is set in line 49. 
For the set_clk function multiple prototypes exist. 
Have a look at class `CLKDevice` to learn more.

~~~{.cpp}
#include "tlm.h"
#include "core/common/amba.h"
#include "core/common/socrocket.h"
#include "power_monitor.h"

#include "core/common/signalkit.h"
#include "testbench.h"
#include "core/models/ahbctrl/ahbctrl.h"
#include "core/models/ahbmem/ahbmem.h"

int sc_main(int argc, char** argv) {

  // *** CREATE MODULES

  // Create testbench
  testbench tbm("Master", 0x400, 0xfff, 0, sc_core::sc_time(10, SC_NS), amba::amba_LT);

  // Create ahbctrl
  AHBCtrl ahbctrl("ahbctrl",
                   0xfff,         // ioaddr
                   0xfff,         // iomask
                   0xff0,         // cfgaddr
                   0xff0,         // cfgmask
                   0,             // rrobin (no effect at LT)
                   0,             // split (no effect at Lt)
                   0,             // defmask
                   0,             // ioen
                   0,             // fixbrst
                   1,             // fpnpen
                   1,             // mcheck
                   1,             // pow_mon
                   amba::amba_LT);

  // Create simulation memory
  AHBMem ahbmem("ahbmem", 0x400, 0xfff, amba::amba_LT, 0);

  // *** BIND SOCKETS

  // Connect testbench master to ahbctrl
  tbm.ahb(ahbctrl.ahbIN);

  // Connect ahbctrl to simulation memory
  ahbctrl.ahbOUT(ahbmem.ahb);

  // Connect snooping ports
  ahbctrl.snoop(tbm.snoop);

  // Set ahbctrl cycle-delay
  ahbctrl.set_clk(10, SC_NS);

  // Start of simulation
  // -------------------
  sc_core::sc_start();

  // Call power analyzer
  PM::analyze("../../../models/","main-power.dat","ahbctrl.1.lt.power");

  return 0;

}
~~~

