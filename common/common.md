Library base classes {#common_doc}
=================================

@todo verify and beautify his document

Clock Device:

class CLKDevice  	
models/utils/clkdevice.*	
lib utils

The class CLKDevice is used to consistently distribute clock/timing and reset amongst all IPs of the library. Devices that inherit from CLKDevice receive two SignalKit inputs: clk and rst. If the child requires reset behavior, it may implement the virtual function dorst(), which is triggered by the rst input. Moreover, CLKDevice provides a data member „clock_cycle“, which can be used by the child to determine the clock period for delay calculations. The value of clock_cycle is set by connecting a sc_time SignalKit signal to the clk input or by calling one of the various set_clk functions of the class.

AHB Device:

class AHBDevice
models/utils/ahbdevice.*
lib utils

All simulation models that are supposed to be connected to the TLM AHBCTRL must be derived from the class AHBDevice. Usually, this is indirectly done by inheriting from the AHB Master or AHB Slave classes (see below). The Aeroflex Gaisler AHBCTRL implements a Plug & Play mechanism, which relies on configuration information that is collected from the attached masters and slaves. AHBDevice models the respective configuration data records. The structure of these records is described in RD04. At start_of_simulation the TLM AHBCTRL iterates through all connected modules to retrieve AHB bar & mask and build up its internal routing table.

AHB Master:
class AHBMaster
models/utils/ahbmaster.*
lib utils

Almost all models implementing an AHB master interface (except busses) are derived from class AHBMaster. AHBMaster is a convenience class providing an AHB master socket and implementations of various access functions for reading/writing data over the bus. AHBMaster inherits AHBDevice and can be configured for loosely timed (LT) or approximately timed (AT) level of abstraction.

An overview about how to build own components based on AHBMaster is given in 3.7.
AHB Slave:
class AHBSlave
models/utils/ahbslave.*

Almost all models implementing an AHB slave interface (except busses) are derived from class AHBSlave. AHBSlave is a convenience class providing an AHB slave socket and callback functions for hooking up with the behaviour of user models. AHBSlave inherits AHBDevice and can be configured for loosely timed (LT) or approximately timed (AT) level of abstraction.

An overview about how to build own components based on AHBSlave is given in 3.7.

APB Device:

class APBDevice
models/utils/apbdevice.*
lib utils

All simulation models that are supposed to be connected to the TLM APBCTRL must be derived from class APBDevice. Similar to the concept of AHBDevice, the child inherits Plug & Play configuration records representing its device type and address. At start_of_simulation the APBCTRL iterates through the connected slaves collecting all APB bar and mask settings for building up its routing table.
Modules, like the MCTRL, which posses an AHB as well as an APB interface must be derived from AHBDevice and APBDevice.

Memory Device:

class MEMDevice
models/utils/memdevice.*
lib utils

The class MEMDevice is the base class of all memories to be connected to the MCTRL. The library provides a Generic Memory, which implements the given interface. The included functions are required to determine the features of the attached component for correct access and delay calculation.

Timing Monitor:

class timingmonitor
common/timingmonitor.*
lib common

Timingmonitor is a support class for timing verification. Within the library it is used in almost all testbench classes. During simulation it records the SystemC simulation time and the real execution time of test phases. For this purpose it provides a set of static control functions. A test phase starts with a call to phase_start_timing. The function expects a phase ID and a phase description as inputs. This will create a new entry in the internal timing map. After completion of the test phase, the testbench calls phase_end_timing to close the record. At the end of the test, the testbench may now call report_timing to generate a report showing the timing of all test phases. This is especially useful for comparing simulations at different levels of abstraction.

Verbosity:

class color, number, msgstream
common/verbose.*

The operators defined in verbose.h can be used to filter output messages respecting their severity. As explained in 2.2.2 the verbosity level of the simulations must be defined during configuration of the library (./waf configure –verbosity=1..5). Five levels may be chosen: error, warning, report, info and debug. The operators are used in a similar way to C++ stdout:

std::cout	<< value << std::endl;	// Regular C++ stdout		

v::error 		<< value << v::endl;		// Verbosity error stream
v::warn 		<< value << v::endl;		// Verbosity warn stream
v::report 	<< value << v::endl;   // Verbosity report stream
v::info 		<< value << v::endl;		// Verbosity info stream
v::debug 		<< value << v::endl;		// Verbosity debug stream
Defining the verbosity at configuration time has the advantage that undesired output is optimized way (compared to runtime switching).

Endianess:

class none
common/vendian.h
lib common

The header vendian.h provides endianess conversion functions for data types of different lengths. If the host system is little endian, CPU and unit tests must swap byte order. The latter is defined by the macro LITTLE_ENDIAN_BO. 
It has to be kept in mind that the LEON processor is a big endian CPU. Hence, memory images generated with the SPARC compiler (e.g. BCC) are also big endian. If the host system simulates the CPU, or testbench, in little endian byte order, all data items going to/from memory must be reordered!

VMAP:
class none
common/vmap.h
lib common

To save system memory and optimize simulation performance large, sparse memories should be implemented as maps. In this case the memory address represents the key and the actual data the entry. Because the performance of the various existing map implementation strongly depends on the system environment, the SoCRocket library provides the flexible type vmap. The vmap.h header contains a macro defining vmap as either std::map, hash_map or std::tr1::unordered_map.
An example for the usage of vmap is given by the MapMemory implementation of the Generic Memory (7).

