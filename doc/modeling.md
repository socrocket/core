Modeling Concepts {#modeling}
=============================

@todo check, verify and beautify this document

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

                                             | Read Operation                      | Write Operation
-------------------------------------------- | ----------------------------------- | ------------------------------------
Begin of AHB address phase                   | `tlm::BEGIN_REQ` (Master -> Slave)  | `tlm::BEGIN_REQ` (Master -> Slave)
End of AHB address phase (incl. arbitration) | `tlm::END_REQ` (Slave -> Master)    | `tlm::END_REQ` (Slave -> Master)
Begin of AHB data phase                      | `tlm::BEGIN_RESP` (Slave -> Master) | `amba::BEGIN_DATA` (Master -> Slave)
End of AHB data phase                        | `tlm::END_REQ` (Master->Slave)      | `amba::END_DATA` (Slave -> Master)
*Table 4 - AT Phase AHB Protocol*

More information about the AT abstraction can be found in the respective section of the model descriptions (e.g. 4.3.3, 6.3.3, 8.3.3) and in RD09.

@section modeling2 Library base classes

Clock Device:

class CLKDevice   
models/utils/clkdevice.*  
lib utils

The class CLKDevice is used to consistently distribute clock/timing and reset amongst all IPs of the library. 
Devices that inherit from CLKDevice receive two SignalKit inputs: clk and rst. 
If the child requires reset behavior, it may implement the virtual function dorst(), which is triggered by the rst input. 
Moreover, CLKDevice provides a data member „clock_cycle“, which can be used by the child to determine the clock period for delay calculations. 
The value of clock_cycle is set by connecting a sc_time SignalKit signal to the clk input or by calling one of the various set_clk functions of the class.

AHB Device:

class AHBDevice
models/utils/ahbdevice.*
lib utils

All simulation models that are supposed to be connected to the TLM AHBCTRL must be derived from the class AHBDevice. 
Usually, this is indirectly done by inheriting from the AHB Master or AHB Slave classes (see below). 
The Aeroflex Gaisler AHBCTRL implements a Plug & Play mechanism, which relies on configuration information that is collected from the attached masters and slaves. 
AHBDevice models the respective configuration data records. 
The structure of these records is described in RD04. 
At start_of_simulation the TLM AHBCTRL iterates through all connected modules to retrieve AHB bar & mask and build up its internal routing table.

AHB Master:

class AHBMaster
models/utils/ahbmaster.*
lib utils

Almost all models implementing an AHB master interface (except busses) are derived from class AHBMaster. 
AHBMaster is a convenience class providing an AHB master socket and implementations of various access functions for reading/writing data over the bus. 
AHBMaster inherits AHBDevice and can be configured for loosely timed (LT) or approximately timed (AT) level of abstraction.

An overview about how to build own components based on AHBMaster is given in 3.7.

AHB Slave:

class AHBSlave
models/utils/ahbslave.*

Almost all models implementing an AHB slave interface (except busses) are derived from class AHBSlave. 
AHBSlave is a convenience class providing an AHB slave socket and callback functions for hooking up with the behaviour of user models. 
AHBSlave inherits AHBDevice and can be configured for loosely timed (LT) or approximately timed (AT) level of abstraction.

An overview about how to build own components based on AHBSlave is given in 3.7.

APB Device:

class APBDevice
models/utils/apbdevice.*
lib utils

All simulation models that are supposed to be connected to the TLM APBCTRL must be derived from class APBDevice. 
Similar to the concept of AHBDevice, the child inherits Plug & Play configuration records representing its device type and address. 
At start_of_simulation the APBCTRL iterates through the connected slaves collecting all APB bar and mask settings for building up its routing table.

Modules, like the MCTRL, which posses an AHB as well as an APB interface must be derived from AHBDevice and APBDevice.

Memory Device:

class MEMDevice
models/utils/memdevice.*
lib utils

The class MEMDevice is the base class of all memories to be connected to the MCTRL. 
The library provides a Generic Memory, which implements the given interface. 
The included functions are required to determine the features of the attached component for correct access and delay calculation.

Timing Monitor:

class timingmonitor
common/timingmonitor.*
lib common

Timingmonitor is a support class for timing verification. 
Within the library it is used in almost all testbench classes. 
During simulation it records the SystemC simulation time and the real execution time of test phases. 
For this purpose it provides a set of static control functions. 
A test phase starts with a call to phase_start_timing. 
The function expects a phase ID and a phase description as inputs. 
This will create a new entry in the internal timing map. 
After completion of the test phase, the testbench calls phase_end_timing to close the record. 
At the end of the test, the testbench may now call report_timing to generate a report showing the timing of all test phases. 
This is especially useful for comparing simulations at different levels of abstraction.

Verbosity:

class color, number, msgstream
common/verbose.*

The operators defined in verbose.h can be used to filter output messages respecting their severity. 
As explained in 2.2.2 the verbosity level of the simulations must be defined during configuration of the library (./waf configure –verbosity=1..5). 
Five levels may be chosen: error, warning, report, info and debug. The operators are used in a similar way to C++ stdout:

~~~
std::cout << value << std::endl;  // Regular C++ stdout   

v::error    << value << v::endl;    // Verbosity error stream
v::warn     << value << v::endl;    // Verbosity warn stream
v::report   << value << v::endl;   // Verbosity report stream
v::info     << value << v::endl;    // Verbosity info stream
v::debug    << value << v::endl;    // Verbosity debug stream
~~~

Defining the verbosity at configuration time has the advantage that undesired output is optimized way (compared to runtime switching).

Endianess:

class none
common/vendian.h
lib common

The header vendian.h provides endianess conversion functions for data types of different lengths. 
If the host system is little endian, CPU and unit tests must swap byte order. 
The latter is defined by the macro LITTLE_ENDIAN_BO. 

It has to be kept in mind that the LEON processor is a big endian CPU. 
Hence, memory images generated with the SPARC compiler (e.g. BCC) are also big endian. 
If the host system simulates the CPU, or testbench, in little endian byte order, all data items going to/from memory must be reordered!

VMAP:
class none
common/vmap.h
lib common

To save system memory and optimize simulation performance large, sparse memories should be implemented as maps. 
In this case the memory address represents the key and the actual data the entry. 
Because the performance of the various existing map implementation strongly depends on the system environment, the SoCRocket library provides the flexible type vmap. 
The vmap.h header contains a macro defining vmap as either std::map, hash_map or std::tr1::unordered_map.

An example for the usage of vmap is given by the MapMemory implementation of the Generic Memory (7).

@section modeling3 TLM Signal Communication Kit

Signal communication in TLM platforms is usually modeled using SystemC signals (sc_signals). 
SystemC signals are applied very similar to RTL signals and, more-or-less, represent hardware wires. 
To achieve the required level of accuracy, all reads and writes of sc_signals need to be scheduled by the SystemC kernel. 
For modeling at a higher level of abstraction this involves an unwanted overhead. 
One would prefer a fast function-call based (TLM-style) communication with a preference of retaining the natural, close to hardware, modeling style of sc_signals.

For this purpose this library provides an extra set of functions. 
The SoCRocket SignalKit can be found in the root directory of the project (./signalkit). 
Within the library it is mainly used to model the interrupt and reset distribution, but also for special purposes like dbus snooping. 
Syntax and application of SignalKit ports are very close to sc_signals. 
Although, signal transmission is performed by directed function calls, similar to TLM blocking transport. 
In contrast to TLM no payload handling is required. The general handling is very simple.

A module that is supposed to utilize SignalKit signals must include the signalkit.h header file and must call the SK_HAS_SIGNALS macro in its class definition. 
The following code example shows a SignalKit module with an outgoing port of type int:

~~~
 1  #include "signalkit.h"
 2
 3  class source : public sc_module {
 4
 5          SK_HAS_SIGNALS(source);
 6          SC_HAS_PROCESS(source);
 7
 8          signal<int>::out out;
 9
10          // Constructor
11          source(sc_module_name nm) : sc_module(nm), out("out") {
12
13                  SC_THREAD(run);
14
15          }
16
17          void run() {
18
19                  // ...
20                  out = i;
21                  // ...
22          }
23  }
24
~~~

The actual signal output is defined in line 8. 
The output is written in line 20. 
Alternatively to  to the shown direct data assignment, the write method of the port may be used (out.write(i)).

The next code block shows a signal receiver:

~~~
 1  #include "signalkit.h"
 2
 3  class dest : public sc_module {
 4
 5          SK_HAS_SIGNALS(dest);
 6
 7          signal<int>::in in;
 8
 9          // Constructor
10          dest(sc_module_name nm) : sc_module(nm), in(&dest::onsignal, "in") {
11
12          }
13
14          // Signal handler for input in
15          void onsignal(const int &value, const sc_time &time) {
16
17                  // do something
18
19          }
20
21  }
22
~~~

In line 10 the handler function onsignal is registered at SignalKit input in. 
If any call is received, this function will be triggered. Int value represents the data transmitted.

Sender and receiver can be connected using the SignalKit connect method. 
An example is given below:

~~~
 1  #include "source.h"
 2  #include "dest.h"
 3
 4  int sc_main(int argc, char *argv[]) {
 5
 6          source src;
 7          dest dst;
 8
 9          connect(src.out, dst.in);
10
11          ...
12
13          return 0;
14
15  }
~~~

Next to that trivial direct connection, the connect method is capable of handling broadcasting and muxing, and for converting between Signalkit and SystemC signals. 
For a broadcast the out signal may be directly connected to multiple ins. 
In the mux case, multiple transmitters are combined into one receiver. 
If required the transmitter may be identified by a channel number. 

@section modeling4 Memory mapped registers

GreenReg is used to model memory mapped registers throughout the library. 
Since almost every model requires a set of control registers, this unified scheme yields a high productivity gain. 
The following steps are required to define a register:

1. Include the GreenReg AMBA Socket header file.
   ~~~
   #include greenreg_ambasockets.h
   ~~~
2. Derive your class from GreenReg device 
   ~~~
   class foo : public gs::reg::gr_device
   ~~~
3. Tell the system that your registers will require callback function (using the build-in macro).
   ~~~
   GC_HAS_CALLBACKS() 
   ~~~
4. Create the socket the register is going to be connected to.
   ~~~
   gs::reg::greenreg_socket<gs::amba::amba_slave<32> > my_sock;
   ~~~
5. In the constructor of the module – initialize gr_device and socket.
   ~~~
   // This will create a register bank of size bank_size bytes
   gr_device(name, gs::reg::ALIGNED_ADDRESS, bank_size, NULL)
   ~~~
6. The initialization of gr_device (5) delivers a default pointer (r) to the newly generated memory bank. 
   The initialization of the socket requires this pointer along the address settings for the bank as input arguments.
   ~~~
   // Initialize socket and hook up register bank r
   my_sock(„sock“, r, start address, end address, protocol (e.g. amba::amba_APB), abstraction (e.g. amba::amba_LT), false) 
   ~~~
7. Create a register within the new memory bank.
   ~~~
   // New register in bank r at bank address + offset
   r.create_register(name, description, offset, type (e.g. gs::reg::STANDARD_REG), initial value, write mask, bit width, lock mask (not used))
   ~~~
8. Register a handler function for the register and make the handler sensitive to a register event.
   ~~~
   GR_FUNCTION(foo, my_handler);
   GR_SENSITIVE(r[offset].add_rule(gs::reg::POST_WRITE, „scaler_write“, gs::reg::NOTIFY))
   ~~~
9. In this example the handler will be called after completion of a write operation (POST_WRITE). The signature of the handler function is void:
   ~~~
   void my_handler()
   ~~~
10. A module that uses GreenReg registers needs to call the following macro in the destructor:
   ~~~
   GC_UNREGISTER_CALLBACKS
   ~~~

Remark: All modules making use of GreenReg memory mapped registers must inherit from class `gr_device`. 
In the SoCRocket library this accounts for all IPs containing an APB slave interface (e.g. `MCTRL`, `IRQMP`, `GPTimer`, `SoCWire`. 

@section modeling5 Power Modeling

The SoCRocket library provides methods and tools for transaction level power estimation. 
All information on this topic has been summoned in the SoCRocket Power Modeling Report [RD11].

@section modeling6 RTL Co-Simulation Transactors
The library provides a set of RTL co-simulation transactors, which have been developed to facilitate the verification of the simulation IP. 
These transactors may also be used as a starting point for integrating other, third party, RTL IP in the system. 

Regarding the AMBA interconnect the library provides 6 different TLM interfaces. 
It can be distinguished between master and slave components, AHB and APB components as well as bus and non-bus components. 

AHB TLM Master – The primary AHB master inferface; To be used for all AHB master components except busses; Can be found in MMU_CACHE and SoCWire IP.

AHB TLM Slave – The primary AHB slave interface; To be used for all AHB slave components except busses; Can be found in MCTRL, APBCTRL and AHBMEM.

AHB TLM Bus Master – The AHB master interface of the AHBCTRL (multi-socket); Stears the communication between bus and slave.

AHB TLM Bus Slave – The AHB slave interface of the AHBCTRL (multi-socket); All bus masters are to be bound to this interface.

APB TLM Master – The APB master interface of the APBCTRL (multi-socket)

APB TLM Slave – The APB slave interface; To be used for all APB slave components; Can be found in the MCTRL, IRQMP, GPTimer and SoCWire IPs.

In general it has to be kept in mind that in AHB systems the communication protocols between master/bus,  and bus/slave are different. This is reflected by the collection of transactors.

All type definitions for conversion between RTL and SystemC are summarized in the header file ahb_adapter_types.h. 

@subsection modeling6_1 AHB RTL Bus -> TLM Slave

Files: `ahb_rtlbus_tlmslave_transactor.{h,cpp}`

The RTL Bus -> TLM Slave transactor allows the GRLIB RTL AHBCTRL to be connected to SoCRocket AHB TLM Slaves. 
Within the library it is used in all co-simulation testbenches of the AHBCTRL IP.

Socket name | Type                     | Description
----------- | ------------------------ | -----------
ahbOUT      | amba_master_socket       | Bind to AHB TLM Slave
ahb_slv_in  | sc_in<ahb_slv_in_type>   | RTL input signals from slave to bus
ahb_slv_out | sc_out<ahb_slv_out_type> | RTL output signals from bus to slave
hirqi       | signal<bool>::infield    | IRQ signals from slave to bus
hirqo       | signal<bool>::selector   | IRQ signals from bus to slave
*Table 5 - RTL Bus to TLM Slave transactor*

@subsection modeling6_2 AHB RTL Master -> TLM Bus

Files: ahb_rtlmaster_tlmbus_transactor.{h,cpp}

The RTL Master -> TLM Bus transactor allows an AHB RTL master to be connected to the SoCRocket AHBCTRL IP. 
Within the library it is used in all co-simulation testbenches of the MMU_CACHE model.

Socket name | Type                       | Description
----------- | -------------------------- | -----------
ahbOUT      | amba_master_socket         | Bind to TLM AHBCTRL
ahb_mst_in  | sc_out<ahb_master_in_type> | RTL output signals from bus to master
ahb_mst_out | sc_in<ahb_master_out_type> | RTL input signals from master to bus
hirqi       | signal<bool>::infield      | IRQ signals from master to bus
hirqo       | signal<bool>::selector     | IRQ signals from bus to master
*Table 6 - AHB RTL Master to TLM Bus transactor*

@subsection modeling6_3 AHB TLM Bus -> RTL Slave

Files: ahb_tlmbus_rtlslave_transactor.{h,cpp}

The TLM Bus -> RTL Slave transactor allows AHB RTL slaves to be connected to the SoCRocket AHBCTRL IP. 
Within the library it is used in several co-simulations of the MCTRL model.

Socket name | Type                            | Description
----------- | ------------------------------- | -----------
ahbIN       | amba_slave_socket               | Bind to TLM AHBCTRL
ahbsi       | sc_out<ahb_slv_in_type>         | RTL output signals from bus to slave
ahbso       | sc_in<ahb_slv_out_type_adapter> | RTL input signals from slave to bus
hirqi       | signal<bool>::infield           | IRQ signals from slave to bus
hirqo       | signals<bool>::selector         |IRQ signals from bus to slave
*Table 7 - AHB TLM Bus to RTL Slave transactor*

@subsection modeling6_4 AHB TLM Master -> RTL Bus

Files: ahb_tlmmaster_rtlbus_transactor.{h,cpp}

The TLM Master -> RTL Bus transactor allows SoCRocket AHB TLM masters to be connected to the GRLIB RTL AHBCTRL. 
Within the library it is used in all co-simulation testbenches of the AHBCTRL IP.

Socket name | Type                             | Description
----------- | -------------------------------- | ------------
ahbIN       | amba_slave_socket                | Bind to TLM master (e.g. mmu_cache)
snoop       | signal<t_snoop>                  | snoop Snooping broadcast bus -> masters
ahb_mst_in  | sc_in<ahb_mst_in_type>           | RTL input signals from bus to master (collected master outputs)
ahb_mst_out | sc_out<ahb_mst_out_type_adapter> | RTL output signals from master to bus
ahb_slv_in  | sc_in<ahb_slv_in_type>           | RTL input signals from bus to master (collected slave outputs)
hirqi       | signal<bool>::infield            | IRQ signals from bus to master
hirqo       | signal<bool>::selector           | IRQ signals from master to bus
*Table 8 - AHB TLM Master to RTL Bus transactor*

@subsection modeling6_5 APB TLM Bus -> RTL Slave

Files: apb_tlmbus_rtlslave_transactor.{h,cpp}

The APB TLM Bus -> RTL Slave transactors allows GRLIB RTL APB slaves to be connected to the SoCRocket APBCTRL (AHB to APB bridge). 
Within the library it is used in various co-simulations of the MCTRL, the IRQMP and the GPTimer.

Socket name | Type                            | Description
----------- | ------------------------------- | -----------
apbIN       | amba_slave_socket               | Bind to TLM APBCTRL
apbi        | sc_out<apb_slv_in_type>         | RTL output signals from bridge to slave
apbo        | sc_in<apb_slv_out_type_adapter> | RTL input signals from slave to bridge
pirqi       | signal<bool>::infield           | IRQ signals from slave to bridge
pirqo       | signal<bool>::selector          |  IRQ signals from bridge to slave
Table 9 - APB TLM Bus to RTL Slave transactor

@subsection modeling6_6 TLM CPU -> RTL Cache

Files: tlmcpu_rtlcache_transactor.{h,cpp}

The TLM CPU -> RTL Cache transactor differs from the remaining transactors of the library. 
It does not aim for connecting models to the AMBA backbone interconnect, but is dedicated to the verification of the MMU_CACHE IP. 

Socket name | Type                   | Description
----------- | ---------------------- | -----------
icio        | simple_target_socket   | Bind to cpu instruction socket
dcio        | simple_target_socket   | Bind to cpu data socket
ici         | sc_out<icache_in_type> | RTL output signals from cpu to icache
ico         | sc_in<icache_out_type> | RTL input signals from icache to cpu
dci         | sc_out<dcache_in_type> | RTL output signals from cpu to dcache
dco         | sc_in<dcache_out_type> | RTL input signals from dcache to cpu
*Table 10 - TLM CPU to RTL Cache transactor*

The source code of all transactors is located in the ./adapters directory. 
The compilation of transactors is integrated in the SoCRocket build system:

~~~
./waf –target=rtladapters
~~~

In order to utilize a RTL transactor in a platform simulation add the rtladapters target to the use line of your build script (wscript).

@section modeling7 Extending the library 

The SoCRocket library can be easily extended, by creating own components.
The existing simulation models provide examples for almost all possible combinations of bus interfaces:

Model     | Bus interfaces
--------- | --------------
leon iss  | CPU instruction master, CPU data master, Interrupt slave
mmu_cache | AHB master, CPU instruction slave, CPU data slave, Interrupt master, Snooping input
ahbctrl   | AHB master (multi-socket), AHB slave (multi-socket), Snooping output
apbctrl   | AHB slave, APB master
ahbmem    | AHB slave
mctrl     | AHB slave, APB slave
socwire   | AHB master, Interrupt master
gptimer   | APB slave, Interrupt master
irqmp     | APB slave, Interrupt master (multi-socket), Interrupt slave (multi-socket)
*Table 11 - Overview models/interfaces*

To be integrated in the SoCRocket platform infrastructure, new models have to fulfill certain requirements. 
Most of them are encapsulated in a set of base classes. 
A detailed description of these base classes can be found in section 3.2.

AHB Masters: All AHB master components must inherit from class AHBMaster. 
AHBMaster is derived from class AHBDevice and template class <BASE>. 
<BASE> can be sc_module, which is the default case, or any other child of sc_module. 
AHBDevice provides the interface for identification of the device in the system. 
At start_of_simulation the AHBCTRL reads the configuration records of all connected AHBDevices (Masters and Slaves) for building up its internal routing table (PNP records). 
The actual master socket, all related functionality, and state machines are encapsulated in class AHBMaster iself.

The socket is defined as follows:
~~~
amba_master_socket<32> ahb
~~~
The socket can be accessed via a set of interface functions (see `./models/utils/ahbmaster.h`). 
Use the following function for reading from the master socket:
~~~
void ahbread(uint32_t addr, unsigned char * data, uint32_t len);
~~~
For writing data to the socket use:
~~~
void ahbwrite(uint32_t addr, unsigned char * data, uint32_t len);
~~~
Other functions are available for using debug transport, activating bus locking, obtaining cacheability information, handing over additional delay, or retrieving a response pointer.
The master can be configured for LT and AT abstraction via constructor parameter `ambaLayer`. 
At LT abstraction read-data can be obtained by evaluating the data pointer right after the interface returns control (blocking). 
At AT abstraction communication is non-blocking. 
This means, in a read operation the data pointer will usually not be valid right after return from the interface call. 
That’s why a callback function is provided for notifying the user about a valid response:
~~~
virtual void response_callback(tlm::tlm_generic_payload * trans) {};
~~~
The response_callback function is plain virtual and must be implemented by the user.

AHB Slaves: AHB slave components must inherit from class AHBSlave. 
AHBSlave is derived from class AHBDevice and template class `<BASE>`. 
`<BASE>` can be `sc_module`, which is the default case, or any other child of `sc_module`. 
Modules implementing memory mapped registers should set `<BASE>` to `gs::reg::gr_device` (Greenreg Device).
AHBDevice provides the interface for identification of the device in the system. 
At start_of_simulation the AHBCTRL reads the configuration records of all connected AHBDevices (Masters and Slaves) for building up its internal routing table (PNP records). 
The actual slave socket, all related functionality, and state machines are encapsulated in class AHBSlave itself.

The socket is defined as follows:
~~~
amba_slave_socket<32> ahb
~~~
Communication with the user class is implemented using a callback function:
~~~
virtual uint32_t exec_func(tlm::tlm_generic_payload &gp, sc_time &delay, bool debug = false) = 0;
~~~
The slave can be configured for LT and AT abstraction via constructor parameter ambaLayer. At LT abstraction exec_func is directly called from b_transport, and at AT abstraction directly after receiving BEGIN_REQ. The user model is expected to load the delay pointer with a response delay value! 
The response delay is the number of wait-states required for delivering the data multiplied with clock cycle time.

APB Slaves: All APB slaves must inherit from class APBDevice. Similar to AHBDevice the conveyed information is used to set up the routing table of the APBCtrl (PNP records).
If the new device is supposed to have memory mapped registers, it must inherit from class gs::reg::gr_device. A small guide for modeling registers with GreenReg can be found in section 3.4.
To enable the connection of the clock, the module should also inherit from class CLKDevice.
~~~
class my_apbcomponent : public gs::reg::gr_device, public APBDevice, public CLKDevice
~~
*Note: Classes that inherit from gr_device must not inherit from sc_module!!*

CPU Master/Slave: Building a component that acts as a CPU or is directly connected to the CPU does not require any base class. However, the transactions generated by the CPU are supposed to carry certain payload extensions. 

Instruction payload extensions: icio_payload_extension.h

Data payload extensions: dcio_payload_extension.h

Make sure to include the appropriate header/s in your design.

Interrupt Master/Slave: All models that send or receive interrupts must use the following macro:
~~~
SK_HAS_SIGNALS(class_name)
~~~
For more information have a look at the SignalKit documentation in section 3.3. 
 

