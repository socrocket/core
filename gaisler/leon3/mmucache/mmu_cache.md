mmu_cache - Memory Management Unit and Cache {#mmu_cache_p}
===========================================================

[TOC]

@section mmu_cache_p_1 Functionality and Features

@subsection mmu_cache_p_1_1 Overview

The `MMU_CACHE` SystemC IP models behaviour and timing of the Gaisler GRLIB Harvard L1 Cache, Localrams and GRLIB Memory Management Unit (MMU). The respective VHDL reference is the `mmu_cache` entity of the GRLIB hardware library.

The structure of the Cache Sub-System is depicted in Figure 4. The top-level class `mmu_cache` provides two TLM 2.0 `simple_target_sockets` ( `icio` , `dcio` ) for communication with the LEON ISS and one Carbon/GreenSocs `amba_master_socket` for the connection to the AHBCTRL. All the sub-components, such as the mmu, the caches and the localrams are implemented in plain C++.

Equivalent to the hardware model the caches can be directly mapped, 2-way, 3-way or 4-way set associative. For multi-set configurations LRU, LRR and pseudo-random replacement are supported. The size of the cache sets can be between 1 and 64 kBytes, with 16 or 32 bytes per line. The caches can be flushed, frozen or locked on a line-by-line basis. The write policy of the data cache is write-through with no-allocate on write miss. The caches can be separately disabled. In that case requests from the ISS are directly forwarded to the AHB master or the MMU (if enabled).

The model also provides instruction and data scratchpads (localrams), with zero-waitstate access to up to 512 kByte of memory.

The MMU can also be optionally enabled. The MMU page size is 4, 8, 16 or 32 kByte. The TLBs can hold between 2 and 32 page descriptors. In case of a page miss a 3-level table walk is carried out on main memory. Similar to the localrams, instantiation of the mmu is done by late binding depending on configuration parameters. The caches connect to the mmu through `tlb_adaptor` objects. The `tlb_adaptors` present a unified memory interface towards the caches ( `mem_if` ). The same memory interface is used to provide access to the AHB master socket on top-level. This way it can be dynamically decided whether a request from one of the caches shall be forwarded to a shared or common TLB (virtual addressing), or directly go to the AHB interface (physical addressing).

@todo insert missing picture

**Figure 4 - Structure of Cache Sub-System**

@subsection mmu_cache_p_1_2 Address Space Identifiers (ASI)

SPARC processors generate an 8-bit address space identifier (ASI), to provide access to up to 256 separate 32-bit address spaces. A big share of the ASIs is used for control of the cache sub-system. A list of the ASIs supported by the TLM model is given in Table 19.

@table Table 19 - Supported ASIs
| ASI                   | Address                  | Usage                                        |
|-----------------------|--------------------------|----------------------------------------------|
| 0x01                  | any                      | Forced cache miss                            |
| 0x02                  | 0x00                     | Cache control register                       |
|                       | 0x04                     | Reserved                                     |
|                       | 0x08                     | Instruction cache configuration register     |
|                       | 0x0c                     | Data cache configuration register            |
|                       | 0xff                     | Trigger debug output                         |
| 0x08,0x09,0x0A,0x0B   | any                      | Normal cache access                          |
| 0x0c                  | see @ref mmu_cache_p_1_8 | Access instruction cache tags                |
| 0x0d                  | see @ref mmu_cache_p_1_8 | Access instruction cache data                |
| 0x0e                  | see @ref mmu_cache_p_1_8 | Access data cache tags                       |
| 0x0f                  | see @ref mmu_cache_p_1_8 | Access data cache data                       |
| 0x15                  | see @ref mmu_cache_p_1_8 | Flush instruction cache                      |
| 0x16                  | see @ref mmu_cache_p_1_8 | Flush data cache                             |
| 0x19                  | 0x000                    | MMU control register                         |
|                       | 0x100                    | MMU Context pointer register                 |
|                       | 0x200                    | MMU Context register                         |
|                       | 0x300                    | MMU Fault status register                    |
|                       | 0x400                    | MMU Fault address register                   |
@endtable

ASIs are emitted by the data interface of the processor. For this purpose an extension has been linked to the data cache payload object ( `dcio_payload_extensions` ). For more information about payload extensions see @ref mmu_cache_p_1_9.

The ASIs are decoded in the `exec_data` function of class `mmu_cache`. The decoder maps the ASIs to API functions of the corresponding sub-components (caches, mmu). The API functions are described in section @ref mmu_cache_p_2.

@subsection mmu_cache_p_1_3 System and Control Registers

The cache sub-system is controlled by a set of system registers, which can be accessed using ASIs.

Three of the mentioned registers are dedicated to the caches (ASI 0x02). The Cache Control Register (CCR, see below) effects both, data and instruction cache. Therefore, it is implemented on top-level ( `mmu_cache` ). Moreover, each of the caches has its own private Configuration Register (CR, see below). The CRs describe structure and size of the caches and are read-only.

@register mmu_cache_CCR CACHE CONTROL REGISTER
[31:24](empty) empty
[23](DS) Data cache snoop enable: If set, will enable data cache snooping (todo).
[22](FD) Flush data cache: If set, will flush the instruction cache. Always reads zero.
[21](FI) Flush instruction cache: If set, will flush the instruction cache. Always reads zero.
[20:17](empty) empty
[16](IB) Instruction burst fetch: This bit enables burst fill during instruction fetch.
[15](IP) Instruction cache flush pending (not supported)
[14](DP) Data cache freeze on interrupt (not supported)
[13:6](empty) empty
[5](DF)
[4](IF) Instruction cache freeze on interrupt (not supported)
[3:2](DCS) Data cache state: Indicates the current data cache state according to the following:
X0 = disable, 01 = frozen, 11 = enabled.
[1:0](ICS) Instruction cache state: Indicates the current instruction cache state according to the following:
X0 = disabled, 01 = frozen, 11 = enabled.
@endregister

@register mmu_cache_CR ICACHE & DCACHE Configuration Register
[31](CL) Cache looking: If set, cache locking is implemented
[30]() 
[29:28](REPL) Cache replacement policy:
00 = non (direct mapped), 01 = least recently used (LRU), 10 = least recently used (LRR), 11 = random
[27](SN) Data cache snooping: Set if snooping is implemented
[26:24](SETS) Number of sets in the cache:
000 = direct mapped, 001 = 2-way associative, 010 = 3-way associative, 011 = 4-way associative
[23:20](SSIZE) Set size: Indicates the size (Kbytes) of each cache set (Size = 2^SSIZE).
[19](LR) Local RAM: Set if local scratchpad is present.
[18:16](LSIZE) Line size: Indicates the size (words) of each cache line (Line size = 2^LSIZE).
[15:12](LRSIZE) Local RAM size: Indicates the size (Kbytes) of the implemented scratchpad RAM (Size = 2^LRSIZE).
[11:4](LRSTART) Local RAM start address: Indicates the 8 most significant bits of the local RAM start address.
[3](M) MMU present: Set if MMU is present
[2:0]() 
@endregister

The MMU is controlled by five 32-bit registers, which can be accessed with ASI 0x19 (see below). All of them are implemented in class `mmu`.

@register mmu_cache_r1 MMU Control Register
[31:1](not used)
[0](E) From the MMU Control Register only one bit is implemented in the TLM model. It is used to enable and disable the MMU.
@endregister

@register mmu_cache_r2 Context Pointer Register
[31:2](Context Table Pointer) The Context Pointer Register points to the Context Table in main memory. It forms bit 35 – 6 of the physical address. The table is indexed by the contents of the Context Register.
@endregister

@register mmu_cache_r3 Context Register
[31:0](Context Number) The Context Register contains the number of the current context and defines which of the possible address spaces is used for address translation.
@endregister

@register mmu_cache_r4 Fault Status Register
[31:0](Not implemented yet) The Fault Status Register provides information on exceptions (faults) issued by the MMU. It is currently not implemented and reads as 0.
@endregister

@todo is this implemented now?

@todo fix issue fix bracket handling in register table

@register mmu_cache_r5 Fault Address Register
[31:0](Fault address (not implemented yet)) The Fault Address Register contains the virtual memory address of the fault recorded in the Fault Status Register. It is currently not implemented and reads as 0.
@endregister

@subsection mmu_cache_p_1_4 Data Cache Snooping

The `mmu_cache` IP supports data cache snooping. Snooping can be enabled by setting bit 23 of the Cache Control Register. The model provides a SignalKit input `snoop`. The constructor of `mmu_cache` registers a callback function ( `snoopingCallBack` ). The AHBCTRL triggers this function on every write operation. Via the `snoop` input the callback receives the bus id of the responsible master, the target address and the length of the write operation. If the master id does not equal the own id and dcache as well as dcache snooping are enabled, `mmu_cache` calls the `snoop_invalidate` function of dcache. The latter checks, whether the access is directed to a locally cached address. If the address is cached, the affected entries are invalidated.

@subsection mmu_cache_p_1_5 Instruction burst fetch

Instruction burst fetch can be enabled by setting the **IB** bit of the Cache Control Register (@ref mmu_cache_CCR "CCR"). In burst fetch mode the respective cache line is filled from main memory starting at the missed address until the end of the line. For this purpose the AHB master executes a burst transfer. The RTL reference model forwards the incoming instructions directly to the processor (streaming). In case of internal dependencies or multi-cycle instructions, the processor stalls until the whole line is cached. The TLM model has a slightly simplified behavour. It will always complete the burst, before sending instructions to the ISS. Burst fetch with disabled instruction cache is not supported. If the instruction cache is disabled, instructions are always being fetch using single transfers (NONSEQ).

@subsection mmu_cache_p_1_6 Cache Flushing

The instruction cache and the data cache can be flushed in multiple ways. If the processor sends a `flush` instruction, both caches are flushed simultaneously. The `mmu_cache` recognizes a flush via the `flush` payload extension. The hardware model of the `mmu_cache` additionally provides two more input fields `flushl` and `fline`. They seems to be intended to flush certain cache lines, but are currently not used. The TLM model provides respective payload extension, to be future-proof.

A flush of only the instruction cache can be triggered by setting bit 21 (FI) of the Cache Control Register or by any write operation with ASI 0x15. Equally, the data cache may be flushed by setting bit 22 (FD) of the Cache Control Register or by any write operation with ASI 0x16.

@subsection mmu_cache_p_1_7 Freezing and Locking

The instruction cache and/or the data cache can be frozen by setting the `ICS` and/or the `DCS` field of the Cache Control Register to "01" (@ref mmu_cache_CCR "CCR"). A cache in frozen state is accessed and kept in sync with the main memory as if it was enabled, but no new lines are allocated on read misses.

Bit 31 of the two Cache Configuration Registers configures cache line locking. A cache line can be locked by setting the lock bit of a line. This can be done by a diagnostic write to the cache tag (@ref mmu_cache_p_1_8 "Diagnostic Access"). Locked cache lines will be updated on read-miss and will remain in the cache until the line is unlocked.

@subsection mmu_cache_p_1_8 Diagnostic Access

Most of the internal data structures of cache and mmu can be accessed for diagnostic purpose via dedicated ASIs.

The tag and data RAMs of instruction and data cache can be read and written using ASI 0x0C – 0x0F (see @ref mmu_cache_p_1_2). Addressing and alignment of data are equivalent to the mechanism described in section 55.5.2. of [GRLIB IP Library User’s Manual](http://gaisler.com/products/grlib/grlib.pdf).

~~~
ADDRESS = WAY & LINE & DATA & "00"
~~~

@subsection mmu_cache_p_1_9 Payload Extensions

The communication between the processor and the Cache Sub-System requires additional information to be attached to the TLM 2.0 generic payload. The extensions are modeled in two classes:

* icio_payload_extension.{h,cpp} - extensions for instruction cache socket
* dcio_payload_extension.{h,cpp} - extensions for data cache socket

Both classes declare a debug extension, which is modeled as a 32bit unsigned integer. The usage of the debug extension is explained in @ref mmu_cache_p_1_10.

The `dcio` extension additionally contains fields for cache flushing, bus locking and the address space identifier ( `asi` ). All are represented by 32 bit unsigned integers.

The `mmu_cache` checks all incoming transactions for the presence of the payload extensions. For data transactions this is done in function `exec_data` , for instruction transactions in function `exec_instr`. An error message is generated, if the extensions are not available.

@subsection mmu_cache_p_1_10 Debug Mechanism

The cache sub-system is a rather complex model. Hence, for assertion-based verification, it is not sufficient to simply check whether the data response on a request is correct. It is also important to know in which way the result was produced (e.g. cache hit/miss).

For this purpose a 32bit unsigned integer extension has been attached to the generic payload of the `icio` / `dcio` sockets. A set of macros is provided for handling the debug extension. The encoding the `debug` field is shown below. The macro definitions can be found in the file `defines.h` of the `mmu_cache` library.

@register mmu_cache_debug Debug Extension
[31:22](reserved)
[21](MMUS) MMU state: 0 – TLB hit; 1- TLB miss
[20:16](TLBN) TLB number:
for TLB hit – number of the TLB that delivered the hit
for TLB miss – number of the TLB that was refilled by the miss
[15](reserved)
[14](FM) Frozen miss: If the cache is frozen, no new lines are allocated on a read miss. However, unvalid data will be replaced as long the tag of the line does not change. In case the results of a read miss is not cached, the FM bit is switched on.
[13](CB) Cache bypass: Is set to 1, if cache bypass was used (cache disabled in CCR)
[12](SP) Scratchpad: Is set to 1, if the request was answered by the local scratchpad RAM
[11:4](reserved)
[3:2](CST) Cache State: 00 – read hit, 01 – read miss, 10 – write hit, 11 – write miss
[1:0](CS) Cache Set: for read/write hit – number of set which produced the hit
for read miss – number of set refilled by miss processing
for write miss – 0b00 (no allocate on write miss)
@endregister

@subsection mmu_cache_p_1_11 Power Monitoring

Power monitoring can be enabled by setting the constructor parameter `pow_mon` to true. Enabling power monitoring for the top-level mmu_cache instance also enables power monitoring for all sub-components. The model is annotated with default power information that has been gathered using a generic 90nm Standard-Cell Library and statistical power estimation at Gate-Level.

The accuracy of the build-in power models and the default switching energy settings cannot be guaranteed. In order to achieve the best possible results the user is recommended to annotate the design with custom target-technology dependent power information.

The power models of the mmu_cache, the i/d caches, the localrams and the mmu; all required parameters, and default settings are explained in the SoCRocket Power Modeling Report [RD11].

@todo power modeling report?
 
@section mmu_cache_p_2 Interface

The GRLIB VHDL model of the MMU_CACHE is configured using Generics. For the implementation of the TLM model most of these Generics were refactored to constructor parameters of class `mmu_cache` (Table 24). The parameters of the top-level class are used for the configuration of all sub-components (caches, localrams, mmu).

@table Table 24 - Constructor Configuration Parameters
| Parameter          | Description                                                                                               |
|--------------------|-----------------------------------------------------------------------------------------------------------|
| icen               | Enable instruction cache                                                                                  |
| irepl              | Icache replacement strategy 00 = non, 01 = LRU, 10 = LRR, 11 = random                                     |
| isets              | Number of instruction cache sets (1-4)                                                                    |
| ilinesize          | Indicates size of instruction cache line in words (line size = 2^ilinesize, ilinesize <= 3)               |
| isetsize           | Indicates size (kbytes) of instruction cache set(set size = 2^isetsize, isetsize <= 6 (max 64 kbytes))    |
| isetlock           | Enable instruction cache locking                                                                          |
| dcen               | Enable data cache                                                                                         |
| drepl              | Dcache replacement strategy00 = non, 01 = LRU, 10 = LRR, 11 = random                                      |
| dsets              | Number of data cache sets (1-4)                                                                           |
| dlinesize          | Indicates size of data cache line in words(line size = 2^dlinesize, dlinesize <= 3 )                      |
| dsetsize           | Indicates size (kbytes) of data cache set(set size = 2^dsetsize, dsetsize <= 6 (max. 64 kbytes))          |
| dsetlock           | Enable data cache locking                                                                                 |
| dsnoop             | Enable data cache snooping                                                                                |
| ilram              | Enable instruction scratchpad                                                                             |
| ilramsize          | Indicates size of instruction scratchpad in kbytes(size = 2^ilramsize, ilramsize <= 9 (max. 512 kbytes))  |
| ilramstart         | 8 MSB bits used to decode local instruction RAM area (16 MB segm.)                                        |
| dlram              | Enable data scratchpad                                                                                    |
| dlramsize          | Indicates size of data scratchpad in kbytes(size = 2^dlramsize, dlramsize <= 9 (max. 512 kbytes))         |
| dlramstart         | 8 MSB bits used to decode local data RAM area (16 MB segment)                                             |
| cached             | Fixed cacheability mask (overrides AMBA Plug & Play settings)                                             |
| mmu_en             | Enable MMU                                                                                                |
| itlb_num           | Indicates number of instruction TLBs(tlb number = 2^itlb_num, itlb_num <= 5 (max. 32))                    |
| dtlb_num           | Indicates number of data TLBs(tlb number = 2^dtlb_num, dtlb_num <= 5 (max. 32))                           |
| tlb_type           | TLB implementation type0 = separate, 1 = shared instruction and data TLB                                  |
| tlb_rep            | TLB replacement policy0 = LRU, 1 = random                                                                 |
| mmupgsz            | MMU page size0, 2 = 4kbytes, 3 = 8kbytes, 4 = 16kbytes, 5 = 32kbytes                                      |
| name               | SystemC name of module                                                                                    |
| id                 | ID of the AHB bus master                                                                                  |
| pow_mon            | Enable power monitoring                                                                                   |
| abstractionLayer   | Abstraction/Coding style of the model (LT or AT)                                                          |
@endtable

The system-level interface of the model comprises two TLM 2.0 `simple_target_sockets` ( `icio` , `dcio` ) and one GreenSocs/Carbon AHB master socket ( `ahb_master` ).

~~~{.cpp}
tlm_utils::simple_target_socket<mmu_cache> icio / bind to CPU instruction socket
tlm_utils::simple_target_socket<mmu_cache> dcio / bind to CPU data socket
amba::amba_master_socket<32> ahb_master         / bind to AMBA system bus
~~~

Depending on the constructor parameter `abstractionLayer` the sockets are configured for blocking (LT) or non-blocking (AT) communication. In the LT case the module registers one TLM blocking transport function for the `dcio` and one for the `icio` socket. In the AT case the model registers one TLM non-blocking forward transport function for the `dcio` and one for the `icio` socket, and one TLM non-blocking backward transport function for the `ahb_master` socket. Additionally, the model comes with debug transport functions, for non-intrusive code execution (TRAP) and checking. The signatures of all transport functions are compliant with the TLM2.0 standard. Next to the TLM sockets the model contains SignalKit inputs for data bus snooping ( `snoop` ), clock cycle time ( `clk` ) and reset ( `rst` ). The clk and `rst` inputs are inherited from class `CLKDevice` , while `snoop` is directly defined in `mmu_cache`.

@section mmu_cache_p_3 Internal Structure

@todo move this section to the respective files?

This section describes the internal structure and the behavior of the MMU_CACHE SystemC IP. The model consists of multiple classes, which are spread over a number of source files, all of which can be found in the `models/leon3/mmucache` directory.

@subsection mmu_cache_p_3_1 Files of the mmu_cache library

@subsubsection mmu_cache_p_3_1_1 The defines.h file

This file contains data type definitions and macros, and is included by almost all the other files of the library.

It defines the structure of the cachelines ( `t_cache_line` ), the data cache entries ( `t_cache_data` ), the cache tags ( `t_cache_tag` ), the mmu page table entries ( `t_PTE_context` ) and the virtual address tags ( `t_VAT` ). Moreover, the file contains macros for handling the debug payload extension (@ref mmu_cache_p_1_10).

@subsubsection mmu_cache_p_3_1_2 The payload_extension files

The TLM MMU_CACHE owns two `tlm::simple_target_sockets` for connection to the instruction and data sockets of the processor simulator. These connections implement a simple point-to-point communication, which can be widely realized relying on TLM 2.0 generic payload. Only a few optional payload extensions are required.

The payload extensions for the instruction cache input/output socket ( `icio` ) are implemented in the files `icio_payload_extensions.h/cpp` :

~~~{.cpp}
// extensions
// ----------
/// flush instruction cache
unsigned int flush;
/// flush instruction cache line
unsigned int flushl;
/// line offset in cache flush
unsigned int fline;
/// debug information  
unsigned int * debug;
~~~

The payload extensions for the data cache input/output socket ( `dcio` ) are implemented in the files `dcio_payload_extensions.h/cpp` :

~~~{.cpp}
// extensions
// ----------
/// address space identifier
unsigned int asi;
/// flush data cache
unsigned int flush;
/// flush data cache line
unsigned int flushl;
/// lock cache line
unsigned int lock;
/// debug information
unsigned int * debug;
~~~

@subsubsection mmu_cache_p_3_1_3 The mem_if.h file

The `mem_if.h` file defines a generic memory interface that is directly or indirectly implemented by almost all the classes of the MMU_CACHE (Figure 5).

The class `mem_if` is an abstract class with two virtual member functions:

~~~{.cpp}
virtual void mem_write(unsigned int addr, unsigned char * data, unsigned int length, sc_core::sc_time * t, unsigned int * debug, bool is_dbg) {};
virtual void mem_read(unsigned int addr, unsigned char * data, unsigned int length, sc_core::sc_time * t, unsigned int * debug, bool is_dbg) {};
~~~

The interface is implemented by the top-level class `mmu_cache` , the caches, the localrams and the mmu ( `tlb_adapters` ). As a consequence the modules of the `mmu_cache` library can be bound to each other like building blocks. For example, depending on the `dcen` and `mmu_en` constructor parameters, transactions from the data socket ( `dcio` ) can be directed to the cache, to the mmu or to the ahb master. Transactions from the dcache or icache are directly forwarded to the ahb master or to the mmu.

@todo inlucde image from word doc

**Figure 5 - Generic Memory Interface / Dependencies**

@subsubsection mmu_cache_p_3_1_4 The mmu_cache_if.h file

The `mmu_cache_if` class extends the `mem_if` class by two functions for reading and writing the Cache Control Register (CCR).

~~~{.cpp}
virtual unsigned int read_ccr();
virtual void write_ccr(unsigned char * data, unsigned int len,   
 sc_time *delay, bool is_dbg);
~~~

The CCR is implemented at the top-level of class `mmu_cache` _._ The caches and the mmu require access to the CCR at runtime. Therefore, they receive a pointer of type `mmu_cache_if` as a constructor argument.

@subsubsection mmu_cache_p_3_1_5 The cache_if.h file

The `cache_if` class is another extension of class `mem_if`. It describes the interface of all cache models in the system. Next to reading or writing data ( `mem_if` ), caches must allow to flush data, to read/write cache tags/entries, to access configuration registers and to handle snooping:

~~~{.cpp}
/// flush cache
virtual void flush(sc_core::sc_time * t, unsigned int * debug, bool is_dbg) = 0;
/// read data cache tags (ASI 0xe)
virtual void read_cache_tag(unsigned int address, unsigned int * data,
sc_core::sc_time *t) = 0;
/// write data cache tags (ASI 0xe)
virtual void write_cache_tag(unsigned int address, unsigned int * data,
sc_core::sc_time *t) = 0;
/// read data cache entries/data (ASI 0xf)
virtual void read_cache_entry(unsigned int address, unsigned int * data,
sc_core::sc_time *t) = 0;
/// write data cache entries/data (ASI 0xf)
virtual void write_cache_entry(unsigned int address, unsigned int * data,
sc_core::sc_time *t) = 0;
/// read cache configuration register (ASI 0x2)
virtual unsigned int read_config_reg(sc_core::sc_time *t) = 0;
/// returns the mode bits of the cache
virtual unsigned int check_mode() = 0;
/// Snooping function (invalidates cache line(s))
virtual void snoop_invalidate(const t_snoop &snoop,
const sc_core::sc_time& delay) = 0;
/// Helper functions for definition of clock cycle
virtual void clkcng(sc_core::sc_time &clk) = 0;
/// Display of cache lines for debug
virtual void dbg_out(unsigned int line) = 0;
~~~

Next to the data cache ( `dvectorcache` ) and the instruction cache ( `ivectorcache` ), the interface is implemented by the plain structural module `nocache`. In case one of the caches is not present in the system (disabled via `i/dcen` constructor parameters), the `mmu_cache` binds one or two instances of `nocache`. The nocache class implements stubs for all `cache_if` functions. Forbidden operations generate an error message.

@subsubsection mmu_cache_p_3_1_6 The mmu_cache.h/cpp files

The files declare and implement the top-level class of the MMU_CACHE. The class `mmu_cache` implements the `mmu_cache_if` interface and instantiates all sub-modules depending on the selected configuration. All sub-components are dynamically created in the constructor of the class. The instantiation depends on parametrization options (see Fehler! Verweisquelle konnte nicht gefunden werden.). In case a certain module is not required, a NULL pointer will be assigned. If the mmu is enabled, the caches use the memory interfaces ( `mem_if` ) of the instruction `tlb_adapters` and data `tlb_adapters` for miss processing, otherwise they are directly connect to the ahb master.

The following pointers provide access to the APIs of all subordinate components:

* ivectorcache* icache - instruction cache pointer
* dvectorcache* dcache - data cache pointer
* mmu* m_mmu - memory management unit
* localram* ilocalram - instruction scratchpad
* localram* dlocalram - data scratchpad

All sub-components are implemented in plain C++, for highest possible simulation speed.

MMU_CACHE also inherits from class `AHBDevice` and class `CLKDevice`. From `AHBDevice mmu_cache` receives a PNP configuration record for identification as an AHB master. Class `CLKDevice` provides an unified interface for clock and reset distribution, that is shared with most of the components in the SoCRocket library. The timing information received via the `clk` SignalKit input is distributed to all sub-components by function `clkcng`.

As the top-level class, `mmu_cache` implements the interface to the outside TLM world. Next to a GreenSocs/Carbon AHB master socket ( `ahb_master` ), the class contains two TLM 2.0 `simple_target_sockets` for connection to the instruction and data ports of the processor simulator (Table 25).

@table Table 25 - TLM Sockets Cache Sub-System
| Name       | Type                                | Description              |
|------------|-------------------------------------|--------------------------|
| icio       | TLM2 / simple_target_socket (LT)    | Instruction cache in/out |
| dcio       | TLM2 / simple_target_socket (LT)    | Data cache in/out        |
| ahb_master | GreenSocs / amba_master_socket (LT) | AHB bus master           |
@endtable

With respect to the TLM 2.0 standard the TLM interface of the `mmu_cache` supports two levels of accuracy: loosely timed (LT) and approximately timed (AT). The abstraction level can be selected via the `abstractionLayer` constructor parameter. The LT interface is described in @ref mmu_cache_p_3_2 and the AT interface in @ref mmu_cache_p_3_3. Both interfaces map the incoming transactions to functions that encapsulate the behaviour of the model. Instruction transactions invoke `exec_instr` and data transactions `exec_data`.

The `exec_instr` function is very compact. After extracting the payload object and verifying the extension, the `tlm_command` attribute is checked. TLM read commands are translated into calls to the `read` ( `mem_if)` function of the `icache` or `ilocalram`. Because the instruction cache is read only, TLM write requests cause a TLM_COMMAND_ERROR_RESPONSE and an error message to be printed on the screen.

The `exec_data` function is more deeply structured. The main reason is the decoder for the address space identifiers (ASIs – see @ref mmu_cache_p_1_2). The ASI is implemented as a mandatory extension to the TLM 2.0 generic payload. Depending on the ASI the `exec_data` function maps the incoming transactions to the APIs of the different sub-components. Default cache access is performed for ASIs 0x8, 0x9, 0xa and 0xb. Other modes are used to access system registers (0x2), tag rams (0xc, 0xe), cache data blocks (0xd, 0xf), mmu internal registers (ASI 0x19) and more. For every transaction the payload extensions are checked. TLM read commands are translated into calls to the `read` ( `mem_if` ) function of the `dcache` or the `dlocalram`. If the `dcache` is disabled the transactions are forwarded to the `mmu` or to the `ahb_master` socket.

The class `mmu_cache` also contains the Cache Control Register (CCR) and its access functions `read_ccr` and `write_ccr` (see Table 20).

@subsubsection mmu_cache_p_3_1_7 The vectorcache.h/cpp files

The files `vectorcache.h` and `vectorcache.cpp` form the base class for the implementation of the instruction cache (ivectorcache) and the data cache (dvectorcache). Class vectorcache implements the `cache_if` API and provides almost all the functionality required by both caches. In the following these functions are briefly described:

~~~{.cpp}
/// read from cache  
void read(unsigned int address, unsigned char * data, unsigned int len, sc_core::sc_time * t, unsigned int * debug, bool is_dbg);
~~~

The `read` ( `mem_if` ) function is called for any type of load operation (byte, short, word, dword). The length of the access in bytes is given by the `len` parameter. The `address` is split into a cache tag and a cache index portion. The respective line is loaded from all sets and compared against the index. If one of the tags equals the index and the valid bit is set, the cache entry is copied to the `*data` pointer (read hit). In case the tags do not match or the valid bit is not set, the request is forwarded to the ahb interface or to the mmu (read miss). After miss processing, the fresh data is filled into the cache and copied to the `*data` pointer.

The `is_dbg` flag signals that the read function was called in a TLM debug transport.

~~~{.cpp}
/// write through cache  
void write(unsigned int address, unsigned char \* data, unsigned int len, sc_core::sc_time \* t, unsigned int \* debug, bool is_dbg);
~~~

The `write` ( `mem_if` ) function is called for any type of store operation (byte, short, word, dword). The length of the access in bytes is given by the `len` parameter. The address is split into a cache tag and a cache index portion. The respective line is loaded from all sets and compared against the index. If one of the tags equals the index and the valid bit is set, the respective data entry is updated and the request is forwarded to the mmu or the ahb interface (write hit). If the tag does not match or the valid bit is not set the request directly goes to mmu or ahb interface (write miss). The cache will not be updated on a write miss. The write policy is write-through with no-allocate on write miss.

~~~{.cpp}
/// flush cache  
void flush(sc_core::sc_time * t, unsigned int * debug);
Flushes the cache_. _During a cache flush all valid data in the cache is transferred to main memory for synchronization.
/// read data cache tags (ASI 0xe)
void read_cache_tag(unsigned int address, unsigned int * data, sc_time *t);
/// write data cache tags (ASI 0xe)
void write_cache_tag(unsigned int address, unsigned int * data, sc_time *t);
/// read data cache entries/data (ASI 0xf)
void read_cache_entry(unsigned int address, unsigned int * data, sc_core::sc_time *t);
/// write data cache entries/data (ASI 0xf)
void write_cache_entry(unsigned int address, unsigned int * data, sc_time *t);
~~~

These functions are used for diagnostic access to cache tags and cache entries (see @ref mmu_cache_p_1_8).

~~~{.cpp}
unsigned int read_config_reg(sc_core::sc_time \*t);
~~~

Returns the configuration register of the cache. The Cache Configuration Register is initialized in the constructor of class **mmu_cache**. The register is read only.

~~~{.cpp}
virtual unsigned int check_mode() = 0;
~~~

A cache can be in one of three different modes of operation: enabled, disabled or frozen. The current mode can be deterimend by checking the Cache Control Register, which is implemented in the top-level class `mmu_cache`. Depending on the type of cache (instruction or data) the DCS or ICS bits of the CCR must be checked. Therefore, the `check_mode` function is plain virtual. The function must be overwritten by the actual `icache` or `dcache` implementation.

@subsubsection mmu_cache_p_3_1_8 The ivectorcache.h/cpp files

The class `ivectorcache` contains the actual implementation of the instruction cache. The class inherits from class `vectorcache`. The write function is overwritten, because the instruction cache is not writable. A call to the write function produces an error message and stops the simulation.

The class implements the virtual function `check_mode`. For checking the mode of operation the ICS bits of the Cache Control Register are used.

@subsubsection mmu_cache_p_3_1_9 The dvectorcache.h/cpp files

The class `dvectorcache` contains the actual implementation of the data cache. The class inherits from class `vectorcache`.

The virtual `check_mode` function is implemented. For checking the mode of operation the DCS bits of the Cache Control Register are used.

@subsubsection mmu_cache_p_3_1_10 The localram.h/cpp files

The class `localram` models a fast scratchpad memory that can be attached to both instruction and data cache controllers. It implements the generic memory interface `mem_if`. The actual memory is implemented as a character array ( `scratchpad` ).

@subsubsection mmu_cache_p_3_1_11 The mmu.h/cpp files

@todo what is RD08?

The files implement the memory management unit of the MMU_CACHE. The component was modeled following the recommendations for the SparcV8 reference MMU given in [RD08]. The class `mmu` receives the number of instruction TLBs, the number of data TLBs, the TLB type, the TLB replacement policy and the mmu page size as constructor arguments. Depending on the TLB type two split TLBs or one shared TLB is generated for instructions and data. The TLBs are implemented as a `std::map`. The key for a TLB lookup is a virtual address tag ( `t_VAT` ). The caches connect to the mmu through `tlb_adapter` objects (@ref mmu_cache_p_3_1_12). In shared TLB mode only one adapter is generated. Next to the adapter objects the class `mmu` offers a set of API functions. The most important of these functions is:

~~~{.cpp}
unsigned int tlb_lookup(unsigned int addr, std::map<t_VAT, t_PTE_context> * tlb, unsigned int tlb_size, sc_core::sc_time * t, unsigned int * debug);
~~~

The `tlb_lookup` function is responsible for translating virtual addresses into physical addresses. It receives the virtual address and a TLB pointer as input arguments. In the body of the function the virtual address is split into three indices. The bit width of these indices depends on the virtual page size. The following page sizes and index combinations are supported (Table 26):

@table Table 26 - Page size / index combinations
| virt. page size | idx 1 | idx 2 | idx 3 |
|-----------------|-------|-------|-------|
| 4kb             | 8 bit | 6 bit | 6 bit |
| 8kb             | 7 bit | 6 bit | 6 bit |
| 16kb            | 6 bit | 6 bit | 6 bit |
| 32kb            | 4 bit | 7 bit | 6 bit |
@endtable

In case of a TLB miss the indices are used for addressing the page tables in main memory. A successful read of a page table returns either a page table descriptor (PTD) or a page table entry (PTE). A PDC is a pointer to the next-level page table, while a PTE corresponds to an actual TLB entry. Up to three page table levels are supported.

The `mmu` contains a set of internal control registers. These registers can be accessed through ASI 0x19 (Table 22). Respectivly read and write requests are translated into calls to following functions:

~~~{.cpp}
// read mmu internal registers (ASI 0x19)
unsigned int read_mcr();
unsigned int read_mctpr();
unsigned int read_mctxr();
unsigned int read_mfsr();
unsigned int read_mfar();
// write mmu internal registers (ASI 0x19)
void write_mcr(unsigned int * data);
void write_mctpr(unsigned int * data);
void write_mctxr(unsigned int * data);
~~~

Another group of member functions is dedicated to diagnostic TLB access. The addressing of the different bit fields can be taken from [RD08].

~~~{.cpp}
// diagnostic read/write of instruction PDC (ASI 0x5)
void diag_read_itlb(unsigned int addr, unsigned int * data);
void diag_write_itlb(unsigned int addr, unsigned int * data);
// diagno. read/write of data PDC or shared instruction and data PDC (ASI 0x6)
void diag_read_dctlb(unsigned int addr, unsigned int * data);  
void diag_write_dctlb(unsigned int addr, unsigned int * data);
~~~

@subsubsection mmu_cache_p_3_1_12 The tlb_adaptor.h file

The class `tlb_adapter` implements the generic memory interface `mem_if`. Depending on the configuration the `mmu` creates one or two objects of type `tlb_adapter` , which provide access to the instruction and/or data tlb. Pointers to these objects can be obtained by calling the mmu API functions `get_itlb_if` and `get_dtlb_if`.

@subsection mmu_cache_p_3_2 LT Behaviour

The LT mode of the MMU_CACHE is intended for fast register accurate simulation (programmers view).

@subsubsection mmu_cache_p_3_2_1 Instruction transactions

For instruction fetch the model provides the `icio`` simple_target_socket `. In LT mode this socket is bound to the` icio_b_transport `blocking transport function. Incoming transactions are directly forwarded to the` exec_instr `function (@ref mmu_cache_p_3_1_6), which models the functional interface of the` mmu_cache `IP. Depending on the configuration` exec_instr `performs a lookup of the instruction cache or loads data from the instruction scratchpad. Cache misses or bypass operations create a transaction on the` ahb_master `socket. If` mmu_en `is set all addresses are considered virtual and will be translated to physical addresses by the mmu. In the meantime the processor is blocked. The` exec_instr `function returns the accumulated delay of all involved sub-components. Before unblocking the master the` icio_b_transport` function calls wait to consume the component delay.

@subsubsection mmu_cache_p_3_2_2 Data transactions

For data load/store the model provides the `dcio simple_target_socket`. In LT mode this socket is bound to the `dcio_b_transport` blocking transport function. Similar to instruction fetch, incoming transactions are directly forwarded to a function encapsulating the behaviour of the data cache. Depending on the configuration and the settings contained in the payload extensions the `exec_data` function performs a lookup of the data cache, loads/store of the instruction or data scratchpad or read/writes of internal registers. Cache misses or bypass operations create a transaction on the `ahb_master` socket. If `mmu_en` is set all addresses are considered virtual and will be translated to physical addresses by the mmu. In the meantime the processor is blocked. The `exec_data` function returns the accumulated delay of all involved sub-components. Before unblocking the master the `dcio_b_transport` function calls wait to consume the component delay.

@subsection mmu_cache_p_3_3 AT Behaviour

The AT mode of the MMU_CACHE is intended for architecture exploration and RTL co-simulation. It contains multiple parallel threads, which are not present in LT mode.

@subsubsection mmu_cache_p_3_3_1 Instruction transactions

Instruction transactions arrive in the `icio_nb_transport_fw` function with phase `BEGIN_REQ`. The function enters the transaction in the `icio_PEQ` payload event queue and returns to the master with `END_REQ` and `TLM_UPDATED`. The SC_THREAD `icio_service_thread` is sensitive to the default event of `icio_PEQ`. It invokes the `exec_instr` function for every transaction from the queue. As already mentioned, `exec_instr` encapsulates the functional part of the model. Within `exec_instr` the payload is processed in the same way as described for LT mode. After return from `exec_instr` the `icio_service_thread` consumes the accumulated delay of all involved mmu_cache sub-components. Afterwards, the master is notified by sending `BEGIN_RESP` on the backward path. The master may reply with `TLM_COMPLETED` or `TLM_ACCEPTED`. A final `END_RESP` from the master will be accepted, but is not required.

@subsubsection mmu_cache_p_3_3_2 Data transactions

In AT mode the constructor of `mmu_cache` registers a non-blocking transport function at the `dcio simple_target_socket (dcio_nb_transport_fw)`. The `dcio_nb_transport_fw` function is called at every phase change of a data transaction. New transactions arrive with phase` BEGIN_REQ `. The transport function enters the transaction in the` dcio_PEQ `payload event queue and returns to the master with` END_REQ `and` TLM_UPDATED `. The` dcio_PEQ `is used to forward the transaction to the SC_THREAD` dcio_service_thread `. It invokes the` exec_data `function for every transaction from the queue. Within` exec_data `the payload is processed in the same way as described for LT mode. After return from` exec_data `the` dcio_service_thread `consumes the accumulated delay of all involved` mmu_cache `sub-components. Afterwards, the master is notified by sending` BEGIN_RESP `on the backward path. The master may reply with` TLM_COMPLETED `or` TLM_ACCEPTED `. Similar to instruction transactions, a final` END_RESP` from the master will be accepted, but is not required.

@section mmu_cache_p_4 The AHB master

Class `mmu_cache` implements the `mem_if` memory interface, to provide access to the `ahb_master` socket, for all modules of the library.

For read transaction this invokes function `mem_read`. Every call to mem_read creates a new payload object. The payload is taken from the transaction pool provided by the GreenSocs/Carbon ahb socket. Target address and payload pointer of the original transaction are copied. Next to the default payload attributes, the function initializes a set of ahb specific extensions:

* `amba::amba_burst_size` - Relates to the streaming width of the AHB bus. The actual size of a burst (in bytes) is given by the length parameter.
* `amba::amba_id` - The AHB master id of the module.
* `amba::amba_trans_type` - AHB transfer type extension. Since all transfers are modeled in a single transaction trans_type is always NON_SEQUENTIAL.

After setting up the payload the `mem_read` function checks the `is_dbg` flag and the `abstractionLayer` parameter. In debug mode the transaction is send using the untimed TLM debug transport interface:

~~~{.cpp}
ahb->transport_dbg(*trans)
~~~

In LT mode mem_read invokes a blocking transport:

~~~{.cpp}
ahb->b_transport(*trans, delay)
~~~

After returning from `b_transport` the model synchronizes with the SystemC scheduler by calling `wait`. This consumes the accumulated delay of the AHB transfer.

In AT mode the bus transfer is modeled using multiple phases. This requires a non-blocking backward transport function ( `ahb_nb_transport_bw` ) to be bound to the `ahb_master` socket and a number of SC_THREADs. If `mem_read` is called in AT mode the AHB transfer is initialized by sending `BEGIN_REQ` on the forward path:

~~~{.cpp}
ahb->nb_transport_fw(*trans, phase, delay);
~~~

In the AHBCTRL this causes the transaction to be scheduled for arbitration. The bus model will reply with TLM_ACCEPTED. The signal for successful arbitration is END_REQ being received on the backward path. At the time of END_REQ the ahb_nb_transport_bw function notifies the mEndRequestEvent, which unblocks the mem_read function. For read operations END_REQ is directly followed by BEGIN_RESP. A BEGIN_RESP from the AHBCTRL triggers the ResponseThread (via mResponsePEQ). The ResponseThread is responsible for sending END_RESP to the AHBCTRL. Moreover, it forwards the transaction to the cleanUP thread. The latter returns the transaction to the memory pool with a delay of 100 `clock_cycles`. The additional lifetime of the transaction guarantees that the data pointer can be savely copied by the master.

Write transactions are processed in a very similar way. Modules writing to the `ahb_master` socket use the `mem_write` ( `mem_if` ) interface function. The `mem_write` function obtains a payload object from the memory pool and initializes all data members including the mentioned ahb specific extensions. The `mem_write` function also distinguishes between debug, blocking (LT) and non-blocking (AT) communication. While debug and blocking communication are trivial, the non-blocking communication differs from the standard TLM protocol. This is due to the pipelined nature of AHB.

AHB communication is split into two phases: address and data. RTL slaves sample the address at the first clock edge and the data at the second. The data phase of the first transaction equals the address phase of the second (succeeding) one. Especially for write transactions from RTL masters to TLM slaves, the TLM standard protocol is insufficient. The slave can never know, when the data pointer of a transaction becomes valid. Therefore, the `BEGIN_RESP` phase of the standard protocol has been replaced by phase `BEGIN_DATA` , which is directed from the master to the slave. The `END_RESP` phase is replaced by phase `END_DATA`. `END_DATA` is send by the slave and indicates the end of a write operation.

The `mem_write` function initiates a bus transfer by sending BEGIN_REQ on the forward path. Equal to read operations the AHBCTRL will reply with END_REQ (backward path), as soon the master has won arbitration. After receiving END_REQ the `ahb_nb_transport_bw` function notifies the `mEndRequestEven` t. Moreover, the transaction is forwarded to SC_THREAD `DataThread` (via `mDataPEQ` ). The `DataThread` sends `BEGIN_DATA` to the AHBCTRL. As soon as the bus has sent `END_DATA` (via backward or return path), the transaction is considered complete. To make sure all pointers can be properly saved, the payload is returned to the memory pool with a delay of 100 `clock_cycles` ( `mEndTransactionPEQ` ).

@section mmu_cache_p_5 Compilation

For the compilation of the MMU_CACHE IP, a WAF wscript is provided and integrated in the superordinate build mechanism of the library.

All required objects for simulating the MMU_CACHE on platform level are compiled in a sub-library named `mmu_cache` using following command:

    $ ./waf –target=mmu_cache

To utilize `mmu_cache` in simulations with other components, add `mmu_cache` to the use list of your wscript.

@section mmu_cache_p_6 Example Instantiation

The example below demonstrates the instantiation of the MMU_CACHE inside an `sc_main` or an arbitrary top-level class. The instantiating module needs to include at least `mmu_cache.h` and `amba.h`. The MMU_CACHE is created in line 3 – 32. In lines 39 – 40 the `icio` and `dcio` slave sockets are bound to the master sockets of the testbench (or processor). The ahb master port is bound to the AHBCTRL in line 43. Line 46 shows how to connect the snooping. Since MMU_CACHE has internal storage, it needs a notion of time. In this example the clock cycle time is set in line 49.

~~~{.cpp}
// CREATE MMU Cache
// ----------------
mmu_cache mmu_cache(1, // int icen = 1 (icache enabled)
        2, // int irepl = 2 (icache random replacement)
        4, //  int isets = 4 (4 instruction cache sets)
        4, //  int ilinesize = 4 (4 words per icache line)
        1, //  int isetsize = 1 (1kB per icache set)
        0, //  int isetlock = 0 (no icache locking)
        1, //  int dcen = 1 (dcache enabled)
        2, //  int drepl = 2 (dcache random replacement)
        4, //  int dsets = 4 (4 data cache sets)
        4, //  int dlinesize = 4 (4 words per dcache line)
        1, //  int dsetsize = 1 (1kB per dcache set)
        0, //  int dsetlock = 0 (no dcache locking)
        0, //  int dsnoop = 0 (no cache snooping)
        0, //  int ilram = 0 (instr. localram disable)
        1, //  int ilramsize = 1 (1kB ilram size - disabled)
        0x0000008e, //  int ilramstart = 8e (0x8e000000 default ilram start address)
        0, //  int dlram = 0 (data localram disable)
        1, //  int dlramsize = 1 (1kB dlram size - disabled)
        0x0000008f, //  int dlramstart = 8f (0x8f000000 default dlram start address)
        0xffff, //  int cached = 0 (fixed cacheability mask) 
        0, //  int mmu_en = 0 (mmu not present)
        8, //  int itlb_num = 8 (8 itlbs - not present)
        8, //  int dtlb_num = 8 (8 dtlbs - not present)
        0, //  int tlb_type = 0 (split tlb mode - not present)
        1, //  int tlb_rep = 1 (random replacement)
        0, //  int mmupgsz = 0 (4kB mmu page size)>
        "mmu_cache", // name of sysc module
        0, //  id of the AHB master
        0, //  bool pow_mon = 1 (disable power monitoring) 
        amba::amba_LT);   // select LT or AT abstraction

...

// *** BIND SOCKETS

// Connect testbench (cpu) to mmu-cache
tbm.icio(mmu_cache.icio);
tbm.dcio(mmu_cache.dcio);

// Connect mmu_cache to TLM bus
mmu_cache.ahb(ahbctrl.ahbIN);

// Connect snooping
ahbctrl.snoop(mmu_cache.snoop);

// Set timing (clock cycle)
mmu_cache.set_clk(10, SC_NS);
~~~
