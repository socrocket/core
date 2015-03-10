Interconnect Methodology {#interconnect_methodology}
===========================================================

[TOC]

This document is the Interconnect Methodology Summary of the SystemC Co-Simulation SoC Validation Platform (SoCRocket). It describes the IP interconnect infrastructure of the library, particularly the modeling of the AMBA protocol at loosely timed and approximately timed levels of abstraction.

Almost all components of the library are equipped with an AMBA interface. At TLM level these interfaces are represented by sockets, which are customized for interchanging payload of a certain format. The AMBA sockets used for modeling the SoCRocket communication interfaces are supplied by a TLM AMBA Modeling Kit, which has been developed under the roof of GreenSoCs. The kit is distributed by Carbon Design Systems Inc. and can be freely downloaded and used. 

@section interconnect_methodology_ahb AHB Modeling

The AHB protocol is modeled at loosely timed and approximately timed level of abstraction. Focus is on high-level, functionally accurate transaction modeling. Low-level signals such as, for example, channel handshakes are not important at this level. Modeling the AHB protocol at transaction level requires a customized payload (@ref im_ahb_payload "AHB Payload") and a mapping of TLM phases to protocol synchronization points (@ref im_ahb_protocol "AHB Protocol Mapping").

@subsection im_ahb_payload AHB Payload 

For modeling AHB bus communication transfer information is either mapped to fields of the TLM generic payload or to a sub-set of the payload extensions provided by the Carbon/GreenSoCs TLM AMBA Modeling Kit (Table 2).

| **AHB Signal** | **Description** | **Mapping** |
| --- | --- | --- |
| HMASTER | ID of the AHB master the initializes the transaction | amba_ext::amba_ID |
| HADDR | Target address of the transaction | tlm_generic_payload::address |
| HTRANS | Type of current transfer: nonseq, seq, busy | Burst implicitly identified by length of payload.tlm_generic_payload::data_length |
| HWRITE | Write or read transfer | tlm_generic_payload::tlm_command |
| HSIZE | Size of the transfer in bytes | tlm_generic_payload::data_length |
| HBURST | Type of burst (e.g. incremental, wrapping) | Burst implicitly identified by length of payload.tlm_generic_payload::data_length |
| HPROT | Protection against illegal transactions | cacheability: amba_ext::cacheablenot supported: instr/data access, normal/privileged access, bufferable |
| HWDATA | Write data | tlm_generic_payload::data_ptr |
| HRDATA | Read data | tlm_generic_payload::data_ptr |
| HRESP | Transfer response | tlm_generic_payload::tlm_response |
| HLOCK | Indicates that master requires locked access to bus | amba_ext::lock |
| HSPLIT | Indicates split transfer | not supported |
| HREADY | Target ready to receive/send data | Low cycles are modeled by delaying BEGIN_RESP |
| HVALID | Initiator ready to receive/send data | Low cycles are modeled by delaying END_RESP |

**Table 2 - AHB TLM payload mapping**

@subsection im_ahb_protocol AHB Protocol mapping

#### LT Abstraction

At LT abstraction AHB transfers are modeled using plain TLM 2.0 blocking transfer calls (b_transport). The initiator starts the transaction at the beginning of the AHB address phase. The delay of the interconnect components and the target is aggregated in the transaction and returned to the initiator. The initiator is responsible for synchronization and may or may not decide to run ahead of time.

#### AT Abstraction

At AT abstraction AHB transfers are modeling using four timing points. The timing points relate to the phases to the TLM 2.0 standard protocol. However, in order to model timing in a more accurate way one additional phase transition was required. In following we will explain all relevant use cases in detail.

**AHB write/read transfer (single)**

@htmlonly
<DIV class="row">
<DIV class="col-md-6">
@endhtmlonly

Figure 1 shows the TLM phase assignment for a single-beat write transfer:

- Initiator sends BEGIN_REQ at beginning of AHB address phase
- Target sends END_REQ at the end of the AHB address phase (address sampled by slave)
- Target sends BEGIN_RESP at the beginning of the AHB data phase; corresponds to HREADY becoming high (slave ready - wait states over). The delay of BEGIN_RESP should also reflect the wait states produced by the target during the data phase (not just the once before accepting the first data item).
- Initiator sends END_RESP at the end of the AHB data phase. END_RESP must be delayed by the number of cycles VALID was low (transmission delayed by master).

@htmlonly
</DIV>
<DIV class="col-md-6">
@endhtmlonly

@image html ahb_at_timing_single.svg "Figure 1 - AHB write transfer (single)"

@htmlonly
</DIV>
</DIV>
@endhtmlonly

**AHB read burst**

@htmlonly
<DIV class="row">
<DIV class="col-md-6">
@endhtmlonly

Figure 2 shows the phase protocol mapping for an AHB read burst:

- The initiator sends BEGIN_REQ at the beginning of the AHB address phase
- The target will send BEGIN_RESP at the beginning of the AHB data phase. BEGIN_RESP is delayed by the number of wait states. This includes both: the wait states before delivering the first data item and the intermediate wait states inserted during transfer (target blocking).
- Because address phase and data phase overlap during an AHB burst transfer, END_REQ will usually be send after BEGIN_RESP. END_REQ marks the end of the AHB address phase. It relates to the time when the target samples the last address of the burst.
- The initiator sends END_RESP at the end of the AHB data phase. END_RESP will be delayed by the number of initiator stall cycles (VALID low).

@htmlonly
</DIV>
<DIV class="col-md-6">
@endhtmlonly

@image html ahb_at_timing_burst.svg "Figure 2 - AHB read/write burst"

@htmlonly
</DIV>
</DIV>
@endhtmlonly


@todo maybe split up the graphic in read and write?

**AHB write burst**

The synchronization points of AHB write bursts are similar to AHB read bursts. So writing can also bee seen in Figure 2:

- The initiator sends BEGIN_REQ at the beginning of the AHB address phase.
- The target sends BEGIN_RESP at the time the first data item is sampled, delayed by the total number of wait states involved in the transactions (cycles HREADY is low). This includes both: the wait states before delivering the first data item and the intermediate wait states inserted during transfer (target blocking).
- Because address phase and data phase overlap during an AHB burst transfer, END_REQ will usually be send after BEGIN_RESP. END_REQ marks the end of the AHB address phase. It relates to the time when the target samples the last address of the burst.
- The initiator sends END_RESP at the end of the AHB data phase. END_RESP will be delayed by the number of initiator stall cycles (VALID low).

@subsection im_ahb_features Not supported or partially supported features

The model does not support early burst termination.

In case of a data split the slave is supposed to delay BEGIN\_RESP. The arbiter (ahbctrl) will not use the additional delay to schedule another master.

@subsection im_ahb_creating Creating/Binding AHB sockets

The easiest way to create custom components with AHB sockets is to inherit from one of the AHB modeling base classes: AHBMaster, AHBSlave. The procedure is explained in detail in the SoCRocket @ref modeling "User Manual".

@todo verify section

For manual instantiation include header amba.h from Carbon TLM AMBA modeling kit.

**Master socket declaration (single):**

~~~{.cpp}
amba::amba_master_socket<32> ahb;
~~~

**Instantiation (module constructor):**

~~~{.cpp}
ahb("ahb", amba::amba_AHB, ambaLayer, false);
# ambaLayer = amba::amba_AT or amba::amba_LT
# false is only used for CT modeling
~~~

**Slave socket declaration (single):**

~~~{.cpp}
amba::amba_slave_socket<32> ahb;
~~~

**Instantiation (module constructor):**

~~~{.cpp}
ahb("ahb", amba::amba_AHB, ambaLayer, false);
# ambaLayer = amba::amba_AT or amba::amba_LT
# false is only used for CT modeling
~~~

**Multi-master socket declaration:**

~~~{.cpp}
amba::amba_master_socket<32, N> ahbIN;
# N: Number of channels for this socket - 0 meaning no limit
~~~

**Instantiation (module constructor):**

~~~{.cpp}
ahbIN("ahbIN", amba::amba_AHB, ambaLayer, false)
# ambaLayer = amba::amba_AT or amba::amba_LT
# false is only used for CT modeling
~~~

**Multi-slave socket declaration:**

~~~{.cpp}
amba::amba_slave_socket<32, N> ahbOUT;
# N: Number of channels for this socket - 0 meaning no limit
~~~

**Instantiation (module constructor):**

~~~{.cpp}
ahbOUT("ahbOUT", amba::amba_AHB, ambaLayer*, false**)
# ambaLayer = amba::amba_AT or amba::amba_LT
# false is only used for CT modeling
~~~

**Binding sockets:**

Binding AHB sockets follows the rules for TLM2.0 socket binding.

~~~{.cpp}
initiator.ahb(target.ahb)
~~~

See core/platforms/leon3mp/sc_main.cpp for more examples.

@section interconnect_methodology_apb APB Modeling

The APB protocol is modeled at loosely timed level of abstraction. Focus is on high-level, functionally accurate transaction modeling. Low-level signals such as, for example, channel handshakes are not important at this level. Modeling the APB protocol at transaction level requires a customized payload (@ref im_apb_payload "APB Payload") and adequate target delay estimation (@ref im_apb_mapping "APB Protocol mapping").

@subsection im_apb_payload APB Payload

For modeling APB bus communication transfer information is either mapped to fields of the TLM generic payload or to a sub-set of the payload extensions provided by the Carbon/GreenSoCs TLM AMBA Modeling Kit (Table 3).

| APB Signal | Description | Mapping |
| --- | --- | --- |
| PADDR | Target address of transfer | tlm_generic_payload::address |
| PSEL/PENABLE | Bridge selects slave (decoder signal) | not required |
| PWRITE | Read or write operation | tlm_generic_payload::tlm_command |
| PWDATA | Write data | tlm_generic_payload::data_ptr |
| PRDATA | Read data | tlm_generic_payload::data_ptr |
| PREADY | Target ready to deliver data (extension of data phase) | Implicitly modeled Target increments transactions delay in blocking transport. |

**Table 3 - APB TLM payload mapping**

@subsection im_apb_mapping APB Protocol mapping

The APB protocol is intended for low-bandwidth communication with I/O components or memory mapped control registers. In contrast to AHB or AXI, APB is not pipelined and can therefore be sufficiently modeled using blocking communication.

@htmlonly
<DIV class="row">
<DIV class="col-md-6">
@endhtmlonly

Figure 4 shows a simple APB write transfer and its abstraction using a blocking transport call:

- The initiator calls b_transport at TLM start. 
- The target will instantly return the aggregated component and setup delays (2 cycles). 
- The initiator receives the delay a blocks the bus. 
- The next transaction may not be issued before TLM end.

@htmlonly
</DIV>
<DIV class="col-md-6">
@endhtmlonly

@image html ahb_apb_timing.svg "Figure 4 - APB blocking transport delay"

@htmlonly
</DIV>
</DIV>
@endhtmlonly

@subsection im_apb_creating Creating/Binding APB sockets

The easiest way to create custom APB components is to instantiate a APBSlave. The come with APB sockets as extended AMBA sockets, which allow the specification of register interfaces.

Please include the core/common/amba.h header to include the Carbon AMBA TLM Modeling Kit and core/common/apbslave.h header in your design.

@todo verify section

A detailed description on how to set up APB components and register interfaces is given in the SoCRocket @ref modeling "User Manual".

**Master socket declaration (single):**

~~~{.cpp}
class Model : public APBSlave {
~~~

@section interconnect_methodology_axi AXI Modeling

@todo is this needed?
