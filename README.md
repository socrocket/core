SoCRocket {#mainpage}
=========

Increasingly large portions of electronic systems are being implemented
in software, and its development cost starts dominating the overall
system's cost. Software is also becoming the critical part of the
development schedule, mainly because deploying and testing it on the
real target hardware is complicated.

![](socrocket-logo.png")

TLM can be used to describe both, timing and functionality, of system
components and their communication interfaces at a high abstraction
level. Embedded in a virtual platform, these models are sufficiently
accurate to not only allow early software development and verification
in a realistic environment but also functional verification of the
modeled hardware. The capability of early design-space exploration is
therefore a vital building block of full hardware/software co-design.

To archive these goals, we designed the _SoCRocket Framework_. Written in
_SystemC/TLM_, it is fitted to serve the space industry'sspecial needs and
builds the foundation of space-domain ESL design. For enabling the
construction virtual platforms, we tied together the following features:

 - **Models** - All models are designed to simulate their coresponding counterparts from the [Aeroflex Gaisler GRLib](http://www.gaisler.com/index.php/downloads/leongrlib)
 - **Analysis Tools** - Dump to Log, DB or Waveform
   - **Performance Counter** - Various counters are implemented in the modules for throughput and other activity monitoring
   - **Power Modeling** - The models feature dedicated power-consumption measuring
 - **Co-Simulation** - Direct comparrison to RTL is possible and was used in verification
 - **Platform Generator** - Easy configuration via GUI or from the command line
 - **Automation Tools** - To run big batches of design-space explorations
 - **Infrastructure** - Reusable components for building new components at ease
 - **Build System** - Extended build system for compiling models, platforms, target software, RTL co-simulations, and regression tests topic is also available at the Accellera page.
 - **Operating Systems** - Boots various Operating Systems like FreeRTOS, RTEMS, uC/OS II without recompilation

An introduction about TLM can be found at [Doulos](http://www.doulos.com/knowhow/systemc/tlm2/). 
More informations are always available at the [Accellera](http://www.accellera.org/home/) page.
Read further for more information. 

