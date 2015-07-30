MCtrl - Memory Controller {#mctrl_p}
====================================

[TOC]

@section mctrl_functionality Functionality and Features

The TLM model of the MCTRL unit models behaviour and timing
of the GRLIB MCTRL VHDL implementation described in RD04. 
It controls a memory subsystem comprising four different
types of memory: PROM, I/O, SRAM, and SDRAM. 
All these memories can be accessed through an AHB slave 
socket, using an internal address decoder.
The control register interface of the device is modeled as
a sr register bank, which is attached to an APB slave socket.
Hence, like any other APB device containing sr registers, 
class mctrl is derived from class APBSlave.

@todo what about this section 3.4 (memory mapped registers)?

The MCTRL is a slave on the AHB bus and on the APB bus.
Respectively, it inherits PNP configuration records from 
classes AHBDevice and APBDevice through
AHBSlave and APBSlave.
The timing of the model is approximated at two different
levels of abstraction (LT and AT).

@subsection mctrl_control_register Control Registers

The register control interface consists of four configuration 
registers (Table 14). All of them are 32 bits wide.

| APB Address Offset | Register                           |
|--------------------|------------------------------------|
| 0x00               | MCFG1 (PROM and I/O)               |
| 0x04               | MCFG2 (RAM)                        |
| 0x08               | MCFG3 (SDRAM Refresh Period)       |
| 0x0C               | MCFG4 (Power Saving Configuration) |

**Table 14 – MCTRL Registers**

Memory configuration register 1 is used to program the timing of rom and local I/O accesses.

@register mctrl_mcfg1 Memory Configuration Register 1 (MCFG1)
  [31:29](RESERVED) Reserved Bits
  [28:27](IOBUSW)   I/O bus width – Sets the data width of the I/O area (00–8bit, 01–16bit, 10–32bit)
  [26](IBRDY)       I/O bus ready enable – Enables bus ready signalling for the I/O area.
  [25](BEXCN)       Bus error enable – Enables bus error signalling.
  [23:20](IOWS)     I/O waitstates – Sets the number of waitstates during I/O access (0-15)
  [19](IOEN)        
  [18:12](RESERVED) Reserved Bits
  [11](PWEN)        PROM write enable – Enables write cycles to the PROM area
  [9:8](PROMWIDTH)  Data width of the PROM area (00-8bit, 0-16bit, 10-32bit)
  [7:4](PROM WWS)   PROM write waitstates (0-15)
  [3:0](PROM RWS)   PROM read waitstates (0-15)
@endregister
<BR>
Memory configuration register 2 is used to control the timing of the SRAM and SDRAM.

@register mctrl_mcfg2 Memory Configuration Register 2 (MCFG2)
  [31](SDRF) SDRAM refresh – Enables SDRAM refresh
  [30](TRP) SDRAM tRP will be equal to 2 or 3 system clocks (0/1)
  [29:27](TRFC) SDRAM tRFC will be equal to 3+field_value system clocks
  [26](TCAS) SDRAM CAS delay 2 or 3 cycles (0/1). Also sets tRCD
  [25:23](SDRAM BANKSZ) SDRAM bank size – Sets the bank size for SDRAM chip selects (000 – 4 MB, 001 – 8 MB, … 111 – 512 MB)
  [22:21](SDRAM COLSZ) SDRAM column size (00 – 256, 01 – 512, 10 – 1024, 11 – 4096)
  [20:19](SD CMD) SDRAM command – (01 – PRECHARGE, 10 – AUTO-REFRESH, 11 – LOAD-CMD)
  [18](D64) SDRAM data bit is 64 bit wide (0/1)
  [17](RESERVED)
  [16](MS) Mobile SDR support enabled (0/1)
  [15](RESERVED)
  [14](SE) SDRAM enable – Enables SDRAM (0/1)
  [13](SI) SRAM disable – Disables SRAM if SE is set to 1.
  [12:9](RAM BANK SIZE) Sets the size of each SRAM bank (0000 – 8kb, 0001 – 16kb, 1111 – 256 MB)
  [7](RBRDY) Enables bus ready signalling for the SRAM area
  [6](RMW) Enables read-modify-write cycles for sub-word writes to 16bit or 32bit areas
  [5:4](RAM WIDTH) Sets the data width of the SRAM area (00 – 8, 01 – 16, 1X – 32)
  [3:2](RAM WWS) Sets the number of wait states for SRAM read cycles (0 – 3)
  [1:0](RAM RWS) Sets the number of wait states for SRAM write cycles (0 – 3)
@endregister
<BR>
MCFG3 is dedicated to SDRAM control and MCFG4 to power saving options (see [GRLIB IP Core User’s Manual](http://gaisler.com/products/grlib/grip.pdf)).

@subsection mctrl_address_space Address Space

The address space is divided in the three partitions: PROM, I/O, and RAM. The division of the address space is static and cannot be modified after initialization of the MCTRL unit. In the VHDL implementation, the different parts of the address space are calculated from generics, which are implemented as constructor parameters in the TLM module.

The PROM address space is derived from the parameters romaddr and rommask , which define the start address and the size of the PROM address space. The romaddr is written to the 12 bit-wide ADDR field of the AHBDevice BAR0 register of the MCTRL. The rommask is written to the 12 bit-wide MASK field of the AHBDevice BAR0 register. The PROM address space is byte-addressable and has an address width of 32 bit.

The size of the PROM address space is:

(212 – MASK ) MByte

The address space is divided into two PROM banks of equal size.

The local I/O address space is calculated in the same way as the PROM address space. All calculations are based on the ioaddr and iomask parameters. The only difference to PROM is that no memory banks are defined.

The SRAM address space is derived from the ramaddr and rammask parameters. Again calculations are very similar to PROM and IO. Although, the partitioning of the resulting address space depends on the settings in the MCFG2 register. The register provides the fields SDRAM enable ( SE ) and SRAM disable( SI ) indicating the presence of SRAM, SDRAM, or SRAM & SDRAM. If the SE bit is low, SI has no effect.

for details and information on the organization of the SRAM address space, regarding the number of banks, bank locations, bank sizes, and – in case of SDRAM – number of row and column address bits, see the [GRLIB IP Core User’s Manual](http://gaisler.com/products/grlib/grip.pdf). Examples for possible partitionings of the RAM address space (default size of 1 GByte) are given in Figure 2.

@todo add address space figure

**Figure 2 – RAM address space**

The default configuration is the SRAM only configuration (Config 1). The entire RAM address space can be split into up to five SRAM banks. The number of SRAM banks is defined by the constructor parameter srbanks. By default four SRAM banks are configured. Banks 1-4 are always located in the lower half of the RAM address space. Their size is variable between 8 KByte – 256 MByte. It can be set using the RAM BANK SIZE field ( RAM BS ) of the MCFG2 register. If the bank size exceeds 128 MByte, the number of banks must be reduced or the size of the address space must be increased. In the SRAM only configuration, a fifth bank can be attached to take up the upper half of the RAM address space.

In the second configuration (Config 2) both SRAM and SDRAM are enabled. In this case, the lower half of the RAM address space is populated by up to 4 SRAM banks. SRAM bank 5 cannot be present, because two SDRAM banks are mapped to the upper half of the RAM address space. The size of the SDRAM banks is scalable between 4 MByte – 512 MByte, according to the SDRAM BANKSZ field of the MCFG2 register. If the SDRAM bank size exceeds 256 MByte, i.e. if the SDRAM bank size is set to 512MByte, the RAM address space needs to be extended to the size of 2GByte to fit the SDRAM in the upper half of the RAM address space. As this will also extend the SRAM address space to 1GByte giving room for the maximum number of four SRAM banks of the maximum supported size of 256 MByte, a size of 2GByte represents the maximum sensible RAM address space. Such a configuration would be reflected by rammask 0x800.

In the SDRAM only configuration (Config 3), the SDRAM banks are mapped into the lower half of the RAM address space. If the SDRAM bank size is set to 512MByte, the RAM address space needs to be extended to the size of 2GByte. The upper half of the address space remains reserved and unused.

In any configuration, the initial bank sizes are calculated to be the maximum possible size, which can be deduced from address space size and number of banks.

It is possible to switch between the three configurations shown in Figure 2 by overwriting the respective bits in the MCFG2 register. In such an event the MCTRL recalculates the start and end addresses of all SRAM and SDRAM memory banks. Because only the address decoding changes, the content of the memories is not affected. However, the bus master must take care to read from the correct memory banks after having caused a reorganization of the RAM address space. The bus master also has to take care of not exceeding the RAM address space when changing the SRAM or SDRAM bank size. If, for example, the RAM address space is 1GByte and the size of four SRAM banks is dynamically switched from 128 MByte to 256 GByte, the SRAM banks will take up the SDRAM address space, causing an overlap of SRAM and SDRAM device addresses. Due to the SystemC code structure, any access to SDRAM would then be redirected to SRAM causing system malfunction.

The way the MCTRL TLM model handles memory access depends on the type of memory addressed. This information is extracted by evaluating the target address. In the default case the delay of a transaction is fully modeled in the MCTRL unit, which holds information about all timing parameters involved. The timing parameters are given in the configuration registers. Additional delay information can be deduced from the streaming width and data length, in case of burst transactions. Optionally, the IORBY ( MCFG1 ) and RBRBY ( MCFG2 ) may be used to obtain additional timing information from the attached memory models and simulate bus-ready-signalling. All delay values are calculated as multiples of the bus clock period.

@subsection mctrl_prom_access PROM Access

In case of PROM, write access needs to be explicitly allowed by setting the PWEN bit of the MCFG1 register. Forbidden write operations, will be cancelled and produce a TLM_COMMAND_ERROR_RESPONSE as well as an error message printed to std::out.

A read access to PROM memory requires 4 bus cycles plus 0 – 15 wait states. A write access to PROM memory takes 3 bus cycles plus 0 – 15 wait states. The wait states can be configured via the PROM READ WS and PROM WRITE WS fields of the MCFG1 register. The attached PROM memory may have a data width of 8, 16 or 32 bits. This must be reflected by the settings in the PROM WIDTH field of MCFG1 . If the PROM WIDTH field is set to 16 or 8 bits, a read access to PROM will still result in loading a full 32 bit word from memory. The word will be transmitted in a burst of two half-words or four single bytes, adding a delay of two bus cycles (data1 and data2) in 16 bit mode or six bus cycles (3x data1 and 3x data2) in 8 bit mode.

@subsection mctrl_local_io_access Local I/O Access

The local I/O area supports access to 32 bit words, 16 bit half-words, and single bytes. A read access takes 4 bus cycles (lead-in, data1, data2, lead-out) and a write access takes 3 bus cycles (lead-in, data, lead-out). For both, read and write operations, the model provides a mechanism for dynamic _bus-ready-signaling_, which can induce an arbitrary number of wait states. It is the task of the attached I/O device to model this delay and add it to the delay parameter of the TLM transport function. The MCTRL unit will observe the delay parameter and add its value to the overall transaction delay.

@subsection mctrl_sram_access SRAM Access

The access to SRAM is similar to the PROM access, the difference being the number of wait states (0 – 3). For a read access, the number of wait states can be set via the RAM READ WS field of the MCFG2 register. Read accesses to SRAM bank 5 and write accesses to SRAM support dynamic wait states in the VHDL model. Similar to the dynamic _bus-ready-signaling_ in the I/O area, it is the task of the TLM memory device to add this delay to the delay variable of the TLM transaction.

@subsection mctrl_sdram_access SDRAM Access

In the VHDL model of the MCTRL SDRAM is accessed over a separate bus, if the sepbus parameter is set to one. This bus can have a width of 32 or 64 bit, as indicated by the D64 field of the MCFG2 register. For TLM communication this architectural detail (separate bus) is not relevant. However, the D64 bit is taken into account for delay calculation, because it affects the streaming width to and from memory.

At TLM level it is also not necessary to model the SDRAM commands, which are emitted by the SDRAM controller. It is only important to estimate the impact of the different command sequences on the memory access time. This especially accounts for opening and closing memory rows for read and write access. The delay of an ACTIVATE command is added to any operation that needs to buffer a new SDRAM row. Closing a row comes at the cost of the delay contributed by a PRECHARGE command.

A read access to SDRAM is always performed as a page burst access. Because a page bursts can be interrupted by a PRECHARGE command, it is possible to read an arbitrary number of data words. In the TLM model, the data length field of the generic payload can hence be set to any multiple of the SDRAM word length. The delay will be calculated for opening the row, sending one data word each clock cycle, and closing the row again. If the requested sequence of words starts at the end of a row and ends in the next row, the time for opening and closing the second row will be added.

The time required for opening a row is determined by the TCAS field of the MCFG2 register. If the TCAS field is changed, a real hardware memory device would require a notification. In the RTL model this is done by sending LMR command. In the TLM model, the MCTRL unit models the timing of each transaction and expects the memory model to behave correctly, i.e. an LMR command would not have any functional effect. Hence, the LMR command is not issued, but its delay is modeled by adding it to the next transaction.

A write access to SDRAM is always performed as a single word write, i.e. burst mode is not supported. A requested write burst from the bus will be transformed into a burst of writes.

To retain data in memory, refresh cycles are required. The MCTRL unit only supports devices capable of AUTO REFRESH, i.e. MCTRL only needs to periodically trigger the refresh, which is then organized by the memory internally. In the TLM implementation, the refresh has no functional effect, but influences the overall operational speed of the memory device. The model keeps track of the refresh period and locks the SDRAM for the duration of one refresh cycle after each refresh period. If an access to the SDRAM device is requested while SDRAM is locked, the transaction will be stalled for the rest of the refresh cycle. The other way round, a refresh can also be stalled by a transaction.

@subsection mctrl_sdream_modes SDRAM Modes of Operation

The MCTRL unit can configure the SDRAM device to operate in several modes. The various operation modes relate to different power saving options. Operation modes can be enabled using the **mobile** constructor parameter.

On system start, SDRAM is always initialized for "normal operation". The initialization sequence of the hardware model is emulated by adding delay to the first transaction.

In case mobile memory is not supported ( `mobile = '0'` ) the MS field of MCFG2 is set to zero. This disables the power saving features exposed by the MCFG4 register of the device. If the mobile parameter is set to one, mobile memory is supported, but disabled by default. The MS field of the MCFG2 register is set to one, but the ME field of the MCFG4 register is set to zero. None of the settings in MCFG4 has any effect as long ME is disabled. For `mobile = '2'` , mobile memory is supported and enabled by default. For `mobile = '3'` , mobile memory cannot be disabled, i.e. the ME field of MCFG4 becomes read only.

If mobile memory is enabled, the SDRAM device supports the following power saving modes: Power Down, Self-Refresh, Partial Array Self Refresh, and Deep Power Down. The actual mode can be selected by writing the PMODE field of MCFG4.

### Normal Operation Mode ###

In normal operation mode, the memory access functions as described in section @ref mctrl_sram_access "SRAM Access". In case of a change in the operation mode, a hardware SDRAM memory would require to be reconfigured. This would be done using the LOAD_EXTENDED_MODE_REGISTER (EMR) command. Like the LMR command, EMR does not have any functional effect. Moreover, the impact of EMR on the timing is neglectable. Therefore, at TLM-level EMR is ignored.

### Power Down Mode ###

To enter power down mode, mobile memory must be enabled and the PMODE field of the MCFG4 register must be changed to "001". In power down mode, the input and output buffers of the SDRAM device are deactivated after an idle period of 16 clock cycles. The buffers can be woken up within one clock cycle at any time. Respectively, each TLM memory access requires an additional delay of one bus clock cycle.

### Self-Refresh Mode ###

If the system is powered down, mobile SDRAM can retain its content by switching into self-refresh mode. Entering self-refresh mode is induced by setting the PMODE field of the MCFG4 register to "010".

In self-refresh mode the system is supposed to be shut down. Therefore, no accesses to memory are expected (despite theoretically possible). For TLM requests in self-refresh mode a warning will be send to std::out.

### Partial Array Self-Refresh Mode ###

In partial array self-refresh mode parts of the memory can be retained during power down. The mode is entered by setting the three-bit-wide PASR field of the MCFG4 register to a value not equal to zero.

The partial array can be defined as the half, quarter, eighth, or sixteenth part of the memory ( PASR = 001, 010, 101, or 110). lt is always associated to the lower bound of the SDRAM address space.

Entering partial array self-refresh mode immediately erases all parts of the TLM SDRAM memory, which are outside of the refresh array.

### Deep Power Down Mode ###

To enter deep power down mode, mobile memory must be enabled and the PMODE field of the MCFG4 register must be changed to "101". Thereafter, the content of the SDRAM memory is immediately deleted.

During deep powe down mode all accesses to SDRAM produce a TLM_ADDRESS_ERROR_RESPONSE.

Deep power down mode can be left by changing the PMODE field of MCFG4 to any other mode of operation.

@subsection mctrl_power_modeling Power Modeling

Power monitoring can be enabled by setting the constructor parameter pow_mon to true. The model is annotated with default power information that has been gathered using a generic 90nm Standard-Cell Library and statistical power estimation at Gate-Level.

The accuracy of the build-in power models and the default switching energy settings cannot be guaranteed. In order to achieve the best possible results the user is recommended to annotate the design with custom target-technology dependent power information.

The power model of the MCTRL, all required parameters, and default settings are explained in the SoCRocket Power Modeling Report.

@todo add the power modeling report?

@section mctrl_interface Interface

The GRLIB VHDL model of the MCTRL is configured using Generics. For the implementation of the TLM model most of these Generics were refactored to constructor parameters of class mctrl. An overview about the available parameters is given in Table 17.

| Parameter | Function | Allowed Range | Default |
| --- | --- | --- | --- |
| name | SystemC name of the module | | |  
| romasel | log2(PROM address space size) - 1. E.g. If size of the PROM area is 0x20000000 romasel is:log2(2^29)-1 = 28. | 0 – 31 | 28 |
| sdrasel | log2(RAM address space size) - 1. E.g If size of the RAM area is 0x40000000 sdrasel is: log2(2^30)-1= 29. | 0 – 31 | 29 |
| romaddr | ADDR field of BAR0 defining PROM address space. | 0 – 0xFFF | 0x000 |
| rommask | MASK field of BAR0 defining PROM address space size. rommask = PROM address space size in MByte | 0 – 0xFFF | 0xE00 |
| ioaddr | similar to romaddr | 0 – 0xFFF | 0x200 |
| iomask | similar to rommask | 0 – 0xFFF | 0xE00 |
| ramaddr | similar to romaddr | 0 – 0xFFF | 0x400 |
| rammask | similar to rommask | 0 – 0xFFF | 0xC00 |
| paddr | ADDR field of the APB BAR configuration registers address space | 0 – 0xFFF | 0x000 |
| pmask | MASK field of the APB BAR configuration registers address space | 0 – 0xFFF | 0xFFF |
| wprot | RAM write protection | 0 – 1 | 0 |
| srbanks | Number of SRAM banks | 0 – 5 | 4 |
| ram8 | Enable 8 bit PROM and SRAM access | 0 – 1 | 0 |
| ram16 | Enable 16 bit PROM and SRAM access | 0 – 1 | 0 |
| sden | Enable SDRAM controller | 0 – 1 | 0 |
| sepbus | SDRAM is located on separate bus | 0 – 1 | 1 |
| sdbits | 32 or 64 bit SDRAM data bus | 24, 64 | 32 |
| mobile | Enable Mobile SDRAM support0: Mobile SDR is not supported1: Mobile SDR is supported but disabled2: Mobile SDR is supported and default3: Mobile SDR support only | 0 – 3 | 0 |
| hindex | AHB slave index | 0 - NAHBSLV–1 | 0 |
| pindex | APB slave index | 0 - NAPBSLV–1 | 0 |
| pow_mon | Enable power monitoring | 0 – 1 | 0 |
| ambaLayer | TLM abstraction/coding style | LT/AT | LT |

**Table 17 – MCTRL Constructor Parameters**

The system-level interface of the TLM MCTRL comprises an AHB slave socket ( ahb ), an APB slave socket ( apb ) and a GreenSocks initiator multi-socket ( mem ). The AHB socket is intended to be bound to the TLM model of the AHBCTRL. The APB socket must be connected to the TLM model of the APBCTRL. The mem socket may be connected to any device implementing the memdevice memory interface (3.2). The SoCRocket library provides a generic memory complying with this condition (7). It can be used as a ROM, I/O, SRAM or SDRAM device.

Depending on the constructor parameter abstrLayer the AHB socket is configured for blocking (LT) or non-blocking (AT) communication. The APB socket is blocking transport (LT) only - indepent of abstrLayer. In the LT case the MCTRL registers two TLM blocking transport functions: one for ahb and one for apb. For the AT abstraction the model provides a TLM non-blocking forward transport function for the AHB socket and a blocking transport function for the APB socket. Additionally, the model contains debug transport functions for both sockets. The signatures of all transport functions are compliant with the TLM2.0 standard.

Next to the TLM sockets the model comes with SignalKit inputs for clock cycle time ( clk ) and reset ( rst ). Both of them are inherited from class CLKDevice , which is shared amongst most of the models in the library.

@section mctrl_internal_structure Internal Structure

This section describes the internal structure of the MCTRL. A basic overview is given in Figure 3. The class hierarchy of the model is flat. All functionality is comprised in class mctrl , which is described in the files mctrl.h and mctrl.cpp.

@todo add figure

**Figure 3 – Structure of the TLM MCTRL**

@subsection mctrl_decoder_initialization Decoder initialization

The internal address decoder represents the central behaviour of the MCTRL. The decoder decides how and under which conditions a transaction from the AHB socket is forwarded to one of the connected slave memories.

Before the start of a simulation the decoder must be initialized. This is done in two steps using SystemC callbacks. First, in the end_of_elaboration function, the memory mapped registers are created, bound and initialized. Afterwards, the start_of_simulation function iterates through all memories bound to the mem socket extracting their configuration information. This is done using the member functions of the memorydevice interface. The start_of_simulation function updates the settings of the MCTRL control registers and creates one PNP base address register entry per memory device ( BAR0-3 ). At simulation time the address information in the BAR records is the base for routing the TLM transactions from the ahb socket to the memories.

@subsection mctrl_lt_behaviour LT behaviour

In LT mode the constructor of the MCTRL registers a blocking transport function ( b_transport ) at the ahb slave socket. All payload objects arriving in b_transport are directly forwarded to the functional part of the model, which is encapsulated in function exec_func.

The exec_func function receives the payload object and the transaction delay pointer as input parameters. At the beginning of the function the payload is extracted. Address, length and data pointer are locally copied. In the next step the address is decoded using the get_ports function. Get_ports compares the address of the transaction with the settings in the decoder registers ( BAR0-3 ). It returns an object of type MEMPort. In case no slave could be found the MCTRL generates a TLM_ADDRESS_ERROR_RESPONSE and an error message. In the following a set of checks is performed to ensure the characteristics of the access are compatible with the addressed memory device. It is checked whether e.g. the transfer length of the transaction is compatible with the memory width, read-modify-write cycles must be inserted or if the targeted memory region is writeable (e.g. PROM). Please see the source code documentation for more detailed information. If one of the constraints is not met, the MCTRL generates a TLM_GENERIC_ERROR_RESPONSE.

Afterwards, the MCTRL calculates the base delay for transfering one word of data to the selected memory ( word_delay ) and the delay offset, which might be involved in the transaction ( trans_delay – e.g. for opening a SDRAM row). The calculations have various dependencies. For PROM, I/O and SRAM the number of wait-states are encoded in MCFG1 and MCFG2. The parameters for the SDRAM timing can be found in MCFG2 and MCFG3 ( TRP, TRFC, TCAS,… ). For mobile SDRAM, additional delay is accumulated for Power Down and Partial Array Self Refresh Mode. Operations directed to memory in Deep Power Down Mode create a TLM_GENERIC_ERROR_RESPONSE.

In the following exec_func creates and initializes a new generic payload. Thereby, the global target address is transformed into an absolute address for the selected memory. The communication between MCTRL and memory is always blocking:

~~~{.cpp}
mem[port_id]->b_transport(memgp, mem_delay);
~~~

If bus-ready signalling is enabled (IBRDY, RBRDY) the calculation of the actual transfer delay is left to the memory ( mem_delay ). Otherwise, mem_delay is ignored and the final delay is calculated using the transfer base delays ( word_delay , trans_delay ), the transfer length, the memory width and the clock cycle time.

After return from exec_func the model calls wait to consume the component delay. Optionally, this task can be shifted to the bus master.

@subsection mctrl_at_behaviour AT behaviour

The AT mode is intended to more accurately approximate the timing of the GRLIB MCTRL. This is achieved by modeling the pipelined nature of the AHB protocol, which allows address phase and data phase of consequtive transactions to overlap. The MCTRL registers two SC_THREAD s to support that feature: acceptTXN and processTXN. Moreover, it provides a TLM non-blocking forward transport function ( nb_transport_fw ). The operation of the module in AT-mode can be best understood by following the control flow of a transaction.

A new transaction arrives in nb_transport_fw with phase BEGIN_REQ. The function enters the transaction in the mAcceptPEQ payload event queue and returns to the caller with TLM_ACCEPTED. The mAcceptPEQ triggers the acceptTXN thread. In case of a read transaction acceptTXN directly copies the payload in the mTransactionPEQ - for notification of thread processTXN. Write transactions are not send to processTXN , before their data pointer becomes valid. This is ackknowledged by the master via a BEGIN_DATA on the forward path.

The processTXN thread calls the functional interface of the MCTRL, which is encapsulated in function exec_func. The behaviour of exec_func has already been described in 6.3.2. After return from exec_func the processTXN thread actives the busy flag, before consuming the accumulated component delay. The busy flag is used by thread acceptTXN in order to check whether a transaction is in progress. As long as the busy flag is true, new transactions are blocked. This means, they are accepted by nb_transport_fw , but will not receive END_REQ.

After unblocking acceptTXN ( busy=false, unlock_event ) the processTXN thread sends BEGIN_RESP for read transactions and END_DATA for write transactions. A final END_RESP on the forward path will be ignored.

@section mctrl_compilation Compilation

For the compilation of the MCTRL unit, a WAF wscript is provided and integrated in the superordinate build mechanism of the libary.

All required objects for simulating the MCTRL on platform level are compiled in a sub-library name mctrl using following command:

    $ ./waf –target=mctrl

For using the MCTRL in simulations with other component add **mctrl** to the use list of your WAF **wscript**.

@section mctrl_example_instantiantion Example Instantiation

An example is given in section 7.5, which jointly demonstrates the instantiation of MCTRL and GenericMemory.

@todo section generic memory systemc model hinzufuegen und verlinken
