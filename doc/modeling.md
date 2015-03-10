Modeling Concepts {#modeling}
=============================

[TOC]

@todo check and verify this document

In this chapter we describe the underlying modeling concepts of the library. 
This comprises coding style/abstraction, as well as common base classes and modeling techniques. 
The goal is to enable the user to extend the library by developing and integrating own modules.

@section modeling1 Coding Style

The simulation models of the library are developed in SystemC language and build on the OSCI TLM2.0 standard. 
Like any TL model they abstract from cycle-timed accuracy by modeling communication in form of function calls. 
Depending on the use case this can be done in many different ways. 
The general aim is to save simulation time by sacrificing a certain amount of timing accuracy. 
Moreover, TL simulations usually give the user a bigger amount of leeway, compared to RTL. 

Two major use cases are covered: 
software development and architecture exploration. 
Consequently, all IPs of the library support loosely timed (LT) and approximately timed (AT) abstraction. 
The abstraction layer is selected using constructor parameters.

@subsection modeling1_1 Loosely Timed (LT)

The LT configuration of the simulation models is intendend for fast address-accurate simulation (SW development). 
Communication is modeled using blocking function calls and as little synchronization with the SystemC kernel as possible. Respectively, 
independent of the protocol, each data transmission completes in a single call to the TLM transport interface. 
The master stalls/blocks during the call. 
All involved transfer or target components increment the delay counter, which is carried along the payload. 
After return of control from the slave, the master may or may not consume the annotated delay. 
To reduce context switching the master is allowed to run ahead of time and to accumulate the delay of multiple transactions. 
LT models are supposed to provide ‘just enough’ timing accuracy to allow an operating system to boot.

@subsection modeling1_2 Approximately Timed (AT)

The AT configuration of the simulation models is intendend for architecture exploration. 
To provide the required accuracy it models selected features of the involved communication protocols. 
This mainly relates to the pipelined nature of AHB. 
In the AHB protocol the data phase of a request and the address phase of a succeeding request may overlap. 
Ignoring this fact in simulations leads to large timing deviations. 
Nevertheless, the AT abstraction of the IPs in this library do not model the AHB protocol in a cycle-accurate way. 
The AT mode is supposed to provide a reasonable speedup over RTL simulation, 
while still being accurate enough to facilitate architectural decisions. 
This is achieved by describing all kinds of AHB transfers using four TLM phases (Table 4):

@table Table 4 - AT Phase AHB Protocol
|                                            |Read Operation                     |Write Operation                     |
|--------------------------------------------|-----------------------------------|------------------------------------|
|Begin of AHB address phase                  |`tlm::BEGIN_REQ` (Master → Slave)  | `tlm::BEGIN_REQ` (Master → Slave)  |
|End of AHB address phase (incl. arbitration)|`tlm::END_REQ` (Slave → Master)    | `tlm::END_REQ` (Slave → Master)    |
|Begin of AHB data phase                     |`tlm::BEGIN_RESP` (Slave → Master) | `amba::BEGIN_DATA` (Master → Slave)|
|End of AHB data phase                       |`tlm::END_REQ` (Master → Slave)    | `amba::END_DATA` (Slave → Master)  |
@endtable 

@startcomment
→ is typed in vim with: ctrl+V followed by u2192 (in insert mode)
@endcomment

More information about the AT abstraction can be found in the respective section of the model descriptions (e.g. @ref ahbctrl_p "AHBCtrl", @ref mctrl_p "MCTRL", @ref mmu_cache_p "mmu_cache") and in @ref interconnect_methodology "Interconnect Methodology".

@section modeling2 Library base classes

This section can be found here: @subpage common_doc 

@section modeling3 TLM Signal Communication Kit

This section can be found here: @subpage signalkit_doc

@section modeling4 Memory mapped registers

sr registers are used to model memory mapped registers throughout the library. 
They are based on sc registers from Cadence and implement the full scireg interface.
Moreover the register creation is more flexible and runtime dynamic than in sc registers.
The register creation is modeled after the GreenLib way.
Since almost every model requires a set of control registers, this unified scheme yields a high productivity gain. 
The following steps are required to define a register:

1. Include the sr_register header file.
   ~~~
   #include "core/common/sr_register.h"
   ~~~
2. Instantiate a register bank in your device device 
   ~~~
   sr_register_bank<> r;
   ~~~
3. Create the socket the register is going to be connected to.
   ~~~
   sr_register_amba_socket<> my_sock;
   ~~~
4. In the constructor of the module – initialize register bank and the socket.
   The initialization of the socket requires this pointer along the address settings for the bank as input arguments.
   ~~~
   // This will create a register bank
   r(name)
   my_sock(„sock“, r, start address, end address, protocol (e.g. amba::amba_APB), abstraction (e.g. amba::amba_LT), false) 
   ~~~
5. Create a register within the new memory bank.
   Register handler functions for register event.
   ~~~
   void init_registers() {
     // New register in bank r at bank address + offset
     r.create_register(name, description, offset, initial value, write mask)
        .callback(SR_POST_WRITE, Object, &Class::my_handler);
   }
   ~~~
6. In this example the handler will be called after completion of a write operation (POST_WRITE). The signature of the handler function is void:
   ~~~
   void &Class::my_handler()
   ~~~

In the SoCRocket library this accounts for all IPs containing an APB slave interface (e.g. `MCTRL`, `IRQMP`, `GPTimer`, `SoCWire`. 

@section modeling5 Power Modeling

The SoCRocket library provides methods and tools for transaction level power estimation. 
All information on this topic has been summoned in the SoCRocket Power Modeling Report [RD11].

@todo where is the power modeling report?

@section modeling6 RTL Co-Simulation Transactors
The library provides a set of RTL co-simulation transactors, which have been developed to facilitate the verification of the simulation IP. 
These transactors may also be used as a starting point for integrating other, third party, RTL IP in the system. 

Regarding the AMBA interconnect the library provides 6 different TLM interfaces. 
It can be distinguished between master and slave components, AHB and APB components as well as bus and non-bus components. 

* AHB TLM Master – The primary AHB master inferface; To be used for all AHB master components except busses; Can be found in MMU_CACHE and SoCWire IP.

* AHB TLM Slave – The primary AHB slave interface; To be used for all AHB slave components except busses; Can be found in MCTRL, APBCTRL and AHBMEM.

* AHB TLM Bus Master – The AHB master interface of the AHBCTRL (multi-socket); Stears the communication between bus and slave.

* AHB TLM Bus Slave – The AHB slave interface of the AHBCTRL (multi-socket); All bus masters are to be bound to this interface.

* APB TLM Master – The APB master interface of the APBCTRL (multi-socket)

* APB TLM Slave – The APB slave interface; To be used for all APB slave components; Can be found in the MCTRL, IRQMP, GPTimer and SoCWire IPs.

In general it has to be kept in mind that in AHB systems the communication protocols between master/bus,  and bus/slave are different. This is reflected by the collection of transactors.

All type definitions for conversion between RTL and SystemC are summarized in the header `file ahb_adapter_types.h`. 

@todo does ahb_adapter_types.h still exist?

@subsection modeling6_1 AHB RTL Bus → TLM Slave

Files: `ahb_rtlbus_tlmslave_transactor.{h,cpp}`

The RTL Bus → TLM Slave transactor allows the GRLIB RTL AHBCTRL to be connected to SoCRocket AHB TLM Slaves. 
Within the library it is used in all co-simulation testbenches of the AHBCTRL IP.

@table Table 5 - RTL Bus to TLM Slave transactor
|Socket name | Type                     | Description                          |
|------------|--------------------------|--------------------------------------|
|ahbOUT      | amba_master_socket       | Bind to AHB TLM Slave                |
|ahb_slv_in  | sc_in<ahb_slv_in_type>   | RTL input signals from slave to bus  |
|ahb_slv_out | sc_out<ahb_slv_out_type> | RTL output signals from bus to slave |
|hirqi       | signal<bool>::infield    | IRQ signals from slave to bus        |
|hirqo       | signal<bool>::selector   | IRQ signals from bus to slave        |
@endtable

@subsection modeling6_2 AHB RTL Master → TLM Bus

Files: `ahb_rtlmaster_tlmbus_transactor.{h,cpp}`

The RTL Master → TLM Bus transactor allows an AHB RTL master to be connected to the SoCRocket AHBCTRL IP. 
Within the library it is used in all co-simulation testbenches of the MMU_CACHE model.

@table Table 6 - AHB RTL Master to TLM Bus transactor
|Socket name | Type                       | Description                          |
|------------|----------------------------|--------------------------------------|
|ahbOUT      | amba_master_socket         | Bind to TLM AHBCTRL                  |
|ahb_mst_in  | sc_out<ahb_master_in_type> | RTL output signals from bus to master|
|ahb_mst_out | sc_in<ahb_master_out_type> | RTL input signals from master to bus |
|hirqi       | signal<bool>::infield      | IRQ signals from master to bus       |
|hirqo       | signal<bool>::selector     | IRQ signals from bus to master       |
@endtable

@subsection modeling6_3 AHB TLM Bus → RTL Slave

Files: `ahb_tlmbus_rtlslave_transactor.{h,cpp}`

The TLM Bus → RTL Slave transactor allows AHB RTL slaves to be connected to the SoCRocket AHBCTRL IP. 
Within the library it is used in several co-simulations of the MCTRL model.

@table Table 7 - AHB TLM Bus to RTL Slave transactor
|Socket name | Type                            | Description                         |
|------------|---------------------------------|-------------------------------------|
|ahbIN       | amba_slave_socket               | Bind to TLM AHBCTRL                 |
|ahbsi       | sc_out<ahb_slv_in_type>         | RTL output signals from bus to slave|
|ahbso       | sc_in<ahb_slv_out_type_adapter> | RTL input signals from slave to bus |
|hirqi       | signal<bool>::infield           | IRQ signals from slave to bus       |
|hirqo       | signals<bool>::selector         | IRQ signals from bus to slave       |
@endtable 

@subsection modeling6_4 AHB TLM Master → RTL Bus

Files: `ahb_tlmmaster_rtlbus_transactor.{h,cpp}`

The TLM Master → RTL Bus transactor allows SoCRocket AHB TLM masters to be connected to the GRLIB RTL AHBCTRL. 
Within the library it is used in all co-simulation testbenches of the AHBCTRL IP.

@table Table 8 - AHB TLM Master to RTL Bus transactor
|Socket name | Type                             | Description                                                     |
|------------|----------------------------------|-----------------------------------------------------------------|
|ahbIN       | amba_slave_socket                | Bind to TLM master (e.g. mmu_cache)                             |
|snoop       | signal<t_snoop>                  | snoop Snooping broadcast bus → masters                         |
|ahb_mst_in  | sc_in<ahb_mst_in_type>           | RTL input signals from bus to master (collected master outputs) |
|ahb_mst_out | sc_out<ahb_mst_out_type_adapter> | RTL output signals from master to bus                           |
|ahb_slv_in  | sc_in<ahb_slv_in_type>           | RTL input signals from bus to master (collected slave outputs)  |
|hirqi       | signal<bool>::infield            | IRQ signals from bus to master                                  |
|hirqo       | signal<bool>::selector           | IRQ signals from master to bus                                  |
@endtable

@subsection modeling6_5 APB TLM Bus → RTL Slave

Files: `apb_tlmbus_rtlslave_transactor.{h,cpp}`

The APB TLM Bus → RTL Slave transactors allows GRLIB RTL APB slaves to be connected to the SoCRocket APBCTRL (AHB to APB bridge). 
Within the library it is used in various co-simulations of the MCTRL, the IRQMP and the GPTimer.

@table Table 9 - APB TLM Bus to RTL Slave transactor
|Socket name | Type                            | Description                             |
|------------|---------------------------------|-----------------------------------------|
|apbIN       | amba_slave_socket               | Bind to TLM APBCTRL                     |
|apbi        | sc_out<apb_slv_in_type>         | RTL output signals from bridge to slave |
|apbo        | sc_in<apb_slv_out_type_adapter> | RTL input signals from slave to bridge  |
|pirqi       | signal<bool>::infield           | IRQ signals from slave to bridge        |
|pirqo       | signal<bool>::selector          |  IRQ signals from bridge to slave       |
@endtable

@subsection modeling6_6 TLM CPU → RTL Cache

Files: `tlmcpu_rtlcache_transactor.{h,cpp}`

The TLM CPU → RTL Cache transactor differs from the remaining transactors of the library. 
It does not aim for connecting models to the AMBA backbone interconnect, but is dedicated to the verification of the MMU_CACHE IP. 

@table Table 10 - TLM CPU to RTL Cache transactor
|Socket name | Type                   | Description                           |
|------------|------------------------|---------------------------------------|
|icio        | simple_target_socket   | Bind to cpu instruction socket        |
|dcio        | simple_target_socket   | Bind to cpu data socket               |
|ici         | sc_out<icache_in_type> | RTL output signals from cpu to icache |
|ico         | sc_in<icache_out_type> | RTL input signals from icache to cpu  |
|dci         | sc_out<dcache_in_type> | RTL output signals from cpu to dcache |
|dco         | sc_in<dcache_out_type> | RTL input signals from dcache to cpu  |
@endtable

The source code of all transactors is located in the ./adapters directory.

@todo isit?
 
The compilation of transactors is integrated in the SoCRocket build system:

    $ ./waf –target=rtladapters


In order to utilize a RTL transactor in a platform simulation add the rtladapters target to the use line of your build script (wscript).

@section modeling7 Extending the library 

The SoCRocket library can be easily extended, by creating own components.
The existing simulation models provide examples for almost all possible combinations of bus interfaces:

@table Table 11 - Overview models/interfaces
|Model     | Bus interfaces                                                                     |
|----------|------------------------------------------------------------------------------------|
|leon iss  | CPU instruction master, CPU data master, Interrupt slave                           |
|mmu_cache | AHB master, CPU instruction slave, CPU data slave, Interrupt master, Snooping input|
|ahbctrl   | AHB master (multi-socket), AHB slave (multi-socket), Snooping output               |
|apbctrl   | AHB slave, APB master                                                              |
|ahbmem    | AHB slave                                                                          |
|mctrl     | AHB slave, APB slave                                                               |
|socwire   | AHB master, Interrupt master                                                       |
|gptimer   | APB slave, Interrupt master                                                        |
|irqmp     | APB slave, Interrupt master (multi-socket), Interrupt slave (multi-socket)         |
@endtable

To be integrated in the SoCRocket platform infrastructure, new models have to fulfill certain requirements. 
Most of them are encapsulated in a set of base classes. 
A detailed description of these base classes can be found in @ref common_doc "Library Base Classes".

AHB Masters: All AHB master components must inherit from class AHBMaster. 
AHBMaster is derived from class AHBDevice and template class `<BASE>`. 
`<BASE>` can be sc_module, which is the default case, or any other child of sc_module. 
AHBDevice provides the interface for identification of the device in the system. 
At start_of_simulation the AHBCTRL reads the configuration records of all connected AHBDevices (Masters and Slaves) for building up its internal routing table (PNP records). 
The actual master socket, all related functionality, and state machines are encapsulated in class AHBMaster iself.

The socket is defined as follows:
~~~{.cpp}
amba_master_socket<32> ahb
~~~
The socket can be accessed via a set of interface functions (see `./models/utils/ahbmaster.h`). 
Use the following function for reading from the master socket:
~~~{.cpp}
void ahbread(uint32_t addr, unsigned char * data, uint32_t len);
~~~
For writing data to the socket use:
~~~{.cpp}
void ahbwrite(uint32_t addr, unsigned char * data, uint32_t len);
~~~
Other functions are available for using debug transport, activating bus locking, obtaining cacheability information, handing over additional delay, or retrieving a response pointer.
The master can be configured for LT and AT abstraction via constructor parameter `ambaLayer`. 
At LT abstraction read-data can be obtained by evaluating the data pointer right after the interface returns control (blocking). 
At AT abstraction communication is non-blocking. 
This means, in a read operation the data pointer will usually not be valid right after return from the interface call. 
That’s why a callback function is provided for notifying the user about a valid response:
~~~{.cpp}
virtual void response_callback(tlm::tlm_generic_payload * trans) {};
~~~
The response_callback function is plain virtual and must be implemented by the user.

AHB Slaves: AHB slave components must inherit from class AHBSlave. 
AHBSlave is derived from class AHBDevice and template class `<BASE>`. 
`<BASE>` can be `sc_module`, which is the default case, or any other child of `sc_module`. 
AHBDevice provides the interface for identification of the device in the system. 
At start_of_simulation the AHBCTRL reads the configuration records of all connected AHBDevices (Masters and Slaves) for building up its internal routing table (PNP records). 
The actual slave socket, all related functionality, and state machines are encapsulated in class AHBSlave itself.

The socket is defined as follows:
~~~{.cpp}
amba_slave_socket<32> ahb
~~~
Communication with the user class is implemented using a callback function:
~~~{.cpp}
virtual uint32_t exec_func(tlm::tlm_generic_payload &gp, sc_time &delay, bool debug = false) = 0;
~~~
The slave can be configured for LT and AT abstraction via constructor parameter ambaLayer. At LT abstraction exec_func is directly called from b_transport, and at AT abstraction directly after receiving BEGIN_REQ. The user model is expected to load the delay pointer with a response delay value! 
The response delay is the number of wait-states required for delivering the data multiplied with clock cycle time.

APB Slaves: All APB slaves must inherit from class APBDevice. Similar to AHBDevice the conveyed information is used to set up the routing table of the APBCtrl (PNP records).
A small guide for modeling registers can be found in section @ref modeling4 "Memory mapped registers".
To enable the connection of the clock, the module should also inherit from class CLKDevice.
~~~{.cpp}
class my_apbcomponent : public APBDevice, public CLKDevice
~~~
*Note: Classes that inherit from gr_device must not inherit from sc_module!*

CPU Master/Slave: Building a component that acts as a CPU or is directly connected to the CPU does not require any base class. However, the transactions generated by the CPU are supposed to carry certain payload extensions. 

Instruction payload extensions: `icio_payload_extension.h`

Data payload extensions: `dcio_payload_extension.h`

Make sure to include the appropriate header/s in your design.

Interrupt Master/Slave: All models that send or receive interrupts must use the following macro:
~~~{.cpp}
SK_HAS_SIGNALS(class_name)
~~~
For more information have a look at the @ref signalkit_doc "SignalKit documentation". 
 

