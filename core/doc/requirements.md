Required Software Packages {#requirements}
=========================================================

The SoCRocket Library can be checked out from our GIT repository at the following location:

    $ git clone https://github.com/socrocket/core.git

To compile and simulate the included models, software and example platforms the following tools are required (items marked with * are optional):

Tool / Lib         | Version    | Vendor                    | Installation Path Variables
------------------ | ---------- | ------------------------- | ---------------------------
Python (+PyQT4)    | >2.7       | Python team               | On `$PATH`
GCC (x86)          | >4.6.0     | GCC team                  | On `$PATH`
Doxygen*           | >1.8.2     | Doxygen                   | On `$PATH`
GCOV/LCOV*         | >4.1.0     | GNU team                  | On `$PATH`
Boost              | >1_37_0    | Boost team                | On system library path and system include path 
Questasim*         | >6.0       | Mentor Graphics           | On `$PATH`
GRLIB*             | 1.0.21     | Aeroflex Gaisler          | `$GRLIB_HOME` – installation root; `GRLIB_TECH` – Path to compiled demo design: `/designs/leon3-gr-xc3s-1500/modelsim`

On a Debian/Ubuntu (64bit) System you can install most of the dependencies with `apt-get`:

    $ sudo apt-get install build-essential python2.7-dev libboost-dev libboost-thread-dev libboost-regex-dev libboost-program-options-dev libboost-filesystem-dev libreadline-dev libc6-i386 lib32z1 lib32ncurses5 lib32bz2-1.0 swig gfortran libatlas-base-dev libhdf5-dev

Please make sure that all the software packages mentioned above are properly installed, before proceeding with building the library. 

Compiling software for the LEON ISS requires a SPARC compiler. 
We recommend using the GCC/BCC provided by Aeroflex Gaisler. 
It will be downloaded automatically when you run `./waf configure`.

The Mentor Modelsim simulator and the Aeroflex Gaisler GRLIB are required for SystemC/VHDL co-simulation. 
This feature can be optionally disabled (see @ref requirements4).
Gcov/Lcov and Doxygen are also optional components. The build system will not check for them. 
If the packages are not present, test coverage calculation and the generation of additional documentation are not possible.
@startcomment
For the setup of the GreenSocs Software and the Carbon AMBA Sockets some additional instructions are given below (see @ref requirements2, @ref requirements3). 
Those are only intended to complement the documentation of the tools not to replace them.
@endcomment

@startcomment
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
@endcomment

@startcomment
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

TODO: does this still work this way?!

Some of the unit tests of the library feature SystemC/VHDL co-simulation. 
These tests require Mentor Modelsim to be installed and the GRLIB hardware library to be pre-compiled.
For installing Modelsim please follow the instruction provided by the tool vendor. 
Make sure to add the vsim, vlib, sccom and vcom executables to your search path.
To obtain a pre-compiled version of the GRLIB library we recommend compiling the build-in `leon3-gr-xc3s-1500` example design:

    $ cd ($GRLIB_HOME/designs/leon3-gr-xc3s-1500) 
    $ make vsim

A description of this design and all its components can be found in the Quick-Start section of the [GRLIB IP Library User’s Manual](http://gaisler.com/products/grlib/grlib.pdf).

Please additionally compile the following files from within the leon3-gr-xc3s-1500 library:

    $ vcom -93 -work gaisler ${GRLIB_HOME}/lib/gaisler/ambatest/ahbtbp.vhd
    $ vcom -93 -work gaisler ${GRLIB_HOME}/lib/gaisler/ambatest/ahbmst_em.vhd
    $ vcom -93 -work gaisler ${GRLIB_HOME}/lib/gaisler/ambatest/ahbslv_em.vhd
    $ vcom -93 -work gaisler ${GRLIB_HOME}/lib/gaisler/ambatest/ahb_tbfunct.vhd
    $ vcom -93 -work gaisler ${GRLIB_HOME}/lib/gaisler/ambatest/ahbtbs.vhd
    $ vcom -93 -work gaisler ${GRLIB_HOME}/lib/gaisler/ambatest/ahbtbm.vhd

Set the following environment variables to enable co-simulation:

    $ export GRLIB_HOME=<ROOT DIRECTORY OF THE GRLIB INSTALLATION>
    $ export GRLIB_TECH=<FOLDER CONTAINING COMPILED DESIGN>

Co-simulation can be explicitly disabled in the library configuration. To exclude all co-simulation targets from compilation configure as follows:

    $ ./waf configure --nomodelsim
@endcomment
