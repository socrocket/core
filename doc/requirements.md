Required Software Packages / Dependencies {#requirements}
=========================================================

The SoCRocket Library can be checked out from our GIT repository at the following location:

    https://projects.c3e.cs.tu-bs.de/git/socrocket.git

To compile and simulate the comprised models, software and example platforms the following tools are required (Table 3):

Tool / Lib         | Version    | Vendor                    | Installation Path Variables
------------------ | ---------- | ------------------------- | ---------------------------
Python (+PyQT4)    | >2.4       | Python team               | On `$PATH`
GCC (x86)          | >4.1.0     | GCC team                  | On `$PATH`
GCC/BCC (Sparc  )  | >4.3.4     | GCC team                  | On `$PATH`
binutils           | >2.19      | GNU team                  | On `$PATH`
Doxygen*           | >=1.8.1    | Doxygen                   | On `$PATH`
GCOV/LCOV*         | >4.1.0     | GNU team                  | On `$PATH`
Boost              | >1_37_0    | Boost team                | `$BOOST_DIR` - header path; `$BOOST_LIB` - library path
SystemC            | =2.2.x     | OSCI                      | `$SYSTEMC_HOME` – installation root
libelf             | 0.152      | Elf Team                  | `$ELF_HOME` – installation root
TLM 2.0            | 2009-07-15 | OSCI                      | `$TLM2_HOME` – installation root
GreenSocs          | 4.2.0      | GreenSocs Ltd.            | `$GREENSOCS_HOME` – inst. root
AMBASockets        | 1.0        | Carbon Design Systems Inc | `$AMBA_HOME` – installation root
LUA                | >=5.1      | Lua Comunity              | `$LUA_HOME` – installation root
Modelsim*          | >6.0       | Mentor Graphics           | On `$PATH`
GRLIB*             | 1.0.21     | Aeroflex Gaisler          | `$GRLIB_HOME` – installation root; `GRLIB_TECH` – Path to compiled demo design: `/designs/leon3-gr-xc3s-1500/modelsim`

*Table 3 - Software Dependencies (* optional)*


Please make sure that all the software packages mentioned above are properly installed, before proceeding with building the library. 

Compiling software for the LEON ISS requires a SPARC compiler. 
We recommend using the GCC/BCC provided by Aeroflex Gaisler. 
It can be downloaded in different preconfigured packages depending on host system and software layout (e.g. bare-metal, rtems).

    http://www.gaisler.com/doc/libio/bcc.html

The Mentor Modelsim simulator and the Aeroflex Gaisler GRLIB are required for SystemC/VHDL co-simulation. 
This feature can be optionally disabled (see 2.2.2).
Gcov/Lcov and Doxygen are also optional components. The build system will not check for them. 
If the packages are not present, test coverage calculation and the generation of additional documentation are not possible.
For the setup of the GreenSocs Software and the Carbon AMBA Sockets some additional instructions are given below (2.1.1, 2.1.2). 
Those are only intended to complement the documentation of the tools not to replace them.

@section requirements1 Making SystemC 2.2 working with newer environments

The SystemC distribution brings along its own headers for BOOST. 
For some new compilers this is a potential problem when compiling GreenSocs or other components, which also rely on BOOST. 
To avoid this problem the SystemC environment must be patched (disabled internal BOOST).

To do so follow these steps:
- Apply the contrib/systemc-2.2_boost.patch to your SystemC 2.2 sources
- Re-generate the configure and `Makefile.in` files with the following commands:
  + `$ aclocal`
  + `$ automake --add-missing --copy`
  + `$ autoconf`

Note that the `-fpermissive` flag is inserted into `configure.in`. 
This enables SystemC 2.2 to compile with recent GCC compilers, but might destroy compatibility to older versions.

@section requirements2 GreenSocs

Before installing GreenSocs SytemC, TLM2.0 and BOOST should be available on the target machine. 

1. *Download GreenSocs*
   The actual GreenSocs software can be downloaded from the following location:
   `http://www.greensocs.com/files/greensocs-4.2.0.tar.gz`
2. *Extract the tarball:*
   ~~~
   $ tar -xvzf greensocs-4.2.0.tar.gz
   ~~~
3. *Set the following environment variables:*
   ~~~
   $ export SYSTEMC_HOME=<THE ROOT OF YOUR SYSTEMC INSTALLATION>
   $ export TLM_HOME=<THE ROOT OF YOUR TLM2.0 INSTALLATION>
   $ export BOOST_DIR=<THE INCLUDE DIR OF YOUR BOOST INSTALLATION>
   $ export GREENSOCS_HOME=<THE FOLDER YOU EXTRACTED GREENSOCS>
   $ export GSGPSOCKET_DIR=$GREENSOCS_HOME/gsgpsocket
   $ export GREENSOCKET_DIR=$GREENSOCS_HOME/greensocket
   $ export GREENREG_DIR=$GREENSOCS_HOME/greenreg
   $ export GREENCONTROL_DIR=$GREENSOCS_HOME
   $ export LUA_HOME=<THE ROOT OF YOUR LUA5.x INSTALLATION>
   ~~~
4. *Fix the GreenSocs `Makefile`*
   Change to the `greensocs-4.2.0/greenreg` folder and apply a small change to `Makefile.config`:
   Comment out lines 2-4 (with `#`):
   ~~~
   2 TOP := $(dir $(lastword $(MAKEFILE_LIST)))
   3 
   4 GREENREG_DIR=$(TOP)
   ~~~
   to:
   ~~~
   2 #TOP := $(dir $(lastword $(MAKEFILE_LIST)))
   3 #
   4 #GREENREG_DIR=$(TOP)
   ~~~
   On 64bit machines you need to remove -m32 from line 23 to compile.
   This will make the GreenSocs build system compatible to a wider range of Make versions and might save some trouble.
5. *Compile the library:*
   The biggest part of the GreenSocs software consists of C language header files, which do not need to be compiled. 
   The only exception is GreenReg. 
   Call make to compile everything that has to be compiled:
   `$ make`
6. *Apply Patches:*
   The contrib folder of the SoCRocket library contains two patches, which need to be applied to GreenSocs (even to version 4.2.0).
   `greenreg-4.0.0-writemask.patch` – Fixes a bug concerning correct setting of writemasks in GreenReg
   `greensocs-4.0.0.patch` – Adds an additional socket variant to the system (Enables GreenReg registers to be bound to Carbon AMBA Sockets).

GreenSocs is now ready to be used.

@section requirements3 GreenSocs/Carbon AMBA Sockets

Based on the GreenSocket TLM2.0 sockets (installed with 2.1.1), GreenSocs Ltd. also developed a dedicated AMBA socket. 
The latter contains payload and protocol extensions for modeling AHB and APB transfer. 
The socket has been approved by ARM and can be downloaded from the website of Carbon Design Systems without charge (Carbon License):

    https://portal.carbondesignsystems.com/ALLIp.aspx?Category=Free%20Downloads

After downloading extract the software to a location of your choice:

    $ tar -xvzf AMBAKit-trunk.tar.gz

Like the biggest share of GreenSocs, the Carbon AMBA Sockets are header-only and do not need to be compiled. 
After exporting the location of the source files the software is ready to be used:

    $ export AMBA_HOME=<THE FOLDER WHERE YOU EXTRACTED THE AMBAKIT>

@section requirements4 Aeroflex Gaisler GRLIB and Mentor Modelsim Simulator

Some of the unit tests of the library feature SystemC/VHDL co-simulation. 
These tests require Mentor Modelsim to be installed and the GRLIB hardware library to be pre-compiled.
For installing Modelsim please follow the instruction provided by the tool vendor. 
Make sure to add the vsim, vlib, sccom and vcom executables to your search path.
To obtain a pre-compiled version of the GRLIB library we recommend compiling the build-in `leon3-gr-xc3s-1500` example design:

~~~
$ cd ($GRLIB_HOME/designs/leon3-gr-xc3s-1500) 
$ make vsim
~~~

A description of this design and all its components can be found in the Quick-Start section of the GRLIB IP Library User’s Manual (`$GRLIB_HOME/doc/grlib.pdf`).

Please additionally compile the following files from within the leon3-gr-xc3s-1500 library:

~~~
vcom -93 -work gaisler ${GRLIB_HOME}/lib/gaisler/ambatest/ahbtbp.vhd
vcom -93 -work gaisler ${GRLIB_HOME}/lib/gaisler/ambatest/ahbmst_em.vhd
vcom -93 -work gaisler ${GRLIB_HOME}/lib/gaisler/ambatest/ahbslv_em.vhd
vcom -93 -work gaisler ${GRLIB_HOME}/lib/gaisler/ambatest/ahb_tbfunct.vhd
vcom -93 -work gaisler ${GRLIB_HOME}/lib/gaisler/ambatest/ahbtbs.vhd
vcom -93 -work gaisler ${GRLIB_HOME}/lib/gaisler/ambatest/ahbtbm.vhd
~~~

Set the following environment variables to enable co-simulation:

~~~
$ export GRLIB_HOME=<ROOT DIRECTORY OF THE GRLIB INSTALLATION>
$ export GRLIB_TECH=<FOLDER CONTAINING COMPILED DESIGN>
~~~

Co-simulation can be explicitly disabled in the library configuration. To exclude all co-simulation targets from compilation configure as follows:

~~~
./waf configure --nomodelsim
~~~
