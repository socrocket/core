User Manual {#mainpage}
===========

@section Introduction
@subsection purpose Purpose and Scope
This document is a user manual (UM) of the SystemC transaction level models developed in the HW-SW SystemC Co-Simulation SoC Validation Platform project. 

In compliance with the SoW, the „UM describes the IP interface and functions and its use from the perspective of the system architect and the programmer, including examples.“

@subsection referencedocuments Reference Documents
The following table will be updated during the development of the UM.

Reference | Document Number    | Document Title, Author                                                   
--------- | ------------------ | -------------------------------------------------------------------------
RD01      | TEC-EDM/2008.27/BG | Statement of Work to ITT- AO/1-6025/09/NL/JK, ESA                        
RD02      | IDA-PPS-0309-2     | HW-SW Co-Simulation SystemC SoC Validation Platform – Technical Proposal 
RD03      | IDA-PPS 0309-3     | HW-SW Co-Simulation SystemC SoC Validation Platform – Management Proposal
RD04      |                    | GRLIB IP Core User’s Manual                                              
RD05      |                    | GRLIB IP Library User’s Manual                                           
RD06      |                    | GreenSocs AMBA LT/AT concepts                                            
RD07      |                    | GreenSocs AMBA TLM 2.0 Extensions                                        
RD08      |                    | SPARC V8 Reference Manual                                                

@subsubsection revisions Revisions
Version | Date     | Description                    
------- | -------- | --------------------------------
1.0     | 01/09/10 | Initial submission             
1.1     | 17/09/10 | Version prior to the MDR meeting
1.2     | 03/05/11 |                                

@subsection install "Installation and Dependencies"
@subsubsection required "Required Software Packages"

The Model Library can be checked out from our SVN repository at following location:

<tt>https://ntserv1.ida.ing.tu-bs.de/svn/hwswcosim/trunk</tt>

To compile and simulate the models following external dependencies are needed (Table 3).

@table Software Dependencies
| Tool / Lib | Version    | Vendor         | Installation Path Variables                         |
| ---------- | ---------- | -------------- | --------------------------------------------------- |
| Python     | >2.3       | Python team    | On $PATH                                            |
| GCC (x86)  | 4.1.0      | GCC team       | On $PATH                                            |
| GMP        | 5.0.0      | GCC team       | On $PATH                                            |
| MPFR       | 2.4.2      | GCC team       | On $PATH                                            |
| binutils   | 2.19       | GNU team       | On $PATH                                            |
| Boost      | 1_37_0     | Boost team     | $BOOST_DIR - header path, $BOOST_LIB - library path |
| SystemC    | 2.2.0      | OSCI           | $SYSTEMC_HOME – installation root                   |
| SCV        |            | OSCI           | $SCV_HOME – installation root                       |
| TLM  2.0   | 2009-07-15 | OSCI           | $TLM2_HOME – installation root                      |
| GreenSocs  | 4.2.0      | GreenSocs Ltd. | $GREENSOCS_HOME – installation root                 |
| AMBAKit    | trunk      | GreenSocs Ltd. | $AMBA_HOME – installation root                      |
@endtable

Please make sure that all the software packages mentioned above are properly installed before the library ist build.

@subsection greensocs "Installing GreenSocs Software"

@subsubsection greensocs_inst GreenSocs

The GreenSocs Library can be downloaded from the following location:

http://www.greensocs.com/files/greensocs-4.2.0.tar.gz

Extract the tarball to a folder on your build system:

~~~{.sh}
$ tar -xvzf greensocs-4.0.0.tar.gz
~~~

Make sure the following shell variables hold paths to the corresponding libraries:

~~~{.sh}
$ export SYSTEMC_HOME=<THE ROOT OF YOUR SYSTEMC INSTALLATION>
$ export TLM_HOME=<THE ROOT OF YOUR TLM2.0 INSTALLATION>
$ export BOOST_DIR=<THE INCLUDE DIR OF YOUR BOOST INSTALLATION>
$ export GREENSOCS_HOME=<THE FOLDER YOU EXTRACTED GREENSOCS>
$ export GSGPSOCKET_DIR=$GREENSOCS_HOME/gsgpsocket
$ export GREENSOCKET_DIR=$GREENSOCS_HOME/greensocket
$ export GREENREG_DIR=$GREENSOCS_HOME/greenreg
$ export GREENCONTROL_DIR=$GREENSOCS_HOME
~~~

Change to the greensocs-4.0.0/greenreg folder and make a small change in the Makefile.conf:
Comment out the 2nd to 4th lines with #:

~~~{Makefile}
 2 TOP := $(dir $(lastword $(MAKEFILE_LIST)))
 3 
 4 GREENREG_DIR=$(TOP)
~~~

to:

~~~{Makefile}
 2 #TOP := $(dir $(lastword $(MAKEFILE_LIST)))
 3 #
 4 #GREENREG_DIR=$(TOP)
~~~

Compile the GreenReg library:

~~~{.sh}
$ make
~~~

A static library is build in the GreenReg directory. This is needed by the model library for the APB modules.

Now the GreenSocs library is ready to use with our model library.
Simply export the GreenSocs root directory as GREENSOCS_HOME:

~~~{.sh}
$ export GREENSOCS_HOME=<THE FOLDER YOU EXTRACTED GREENSOCS>
~~~

Or use the waf configuration parameter –greensocs with the greensocs root directory of the model library:

~~~{.sh}
$ ./waf configure –greensocs=<THE FOLDER YOU EXTRACTED GREENSOCS>
~~~

It is recommended to apply the AMBAKit Sockets patch to the GreenSocs library. This enables you to build your own models with GreenReg and AMBA Sockets outside the model library. Anyway the model library arrives with a workaround.

@subsubsection amba "GreenSocs/Carbon AMBA Sockets"

The GreenSocs/Carbon AMBA Sockets are available from the Website of Carbon Design Systems …
Extract the tarball to a folder on your build system:

~~~{.sh}
$ tar -xvzf AMBAKit-trunk.tar.gz
~~~

The AMBAKit is like the TLM Library only a collection of headers. Just export the location to the build system of de model library:

~~~{.sh}
$ export AMBA_HOME=<THE FOLDER YOU EXTRACTED THE AMBAKIT>
~~~

Or use the waf configuration parameter –amba with the greensocs root directory of the model library:

@code
$ ./waf configure –amba=<THE FOLDER YOU EXTRACTED THE AMBAKIT>
@endcode

@subsection build "Building IP Model Library and Tests"

The build system is written in waf. All dependencies will be checked before the compilation of the project begins. The waf binary is located in the project root. It requires at least python 2.3 to run.

Do a “./waf –h” to get help on all available commands and options.

@code
waf [command] [options]
@endcode

Building the project requires following steps:

1.  Execute “<tt>./waf configure</tt>” – to configure the build environment:
    The configuration step succeeds in case all the required software packages are available. Otherwise, it fails and shows the broken dependency. If so, the install path variable must be corrected. It is also possible to specify the location of a missing package by one of the following options:

    ~~~
      Common Configuration Options:
        -b BLDDIR, --blddir=BLDDIR
                            build dir for the project (configuration)
        -s SRCDIR, --srcdir=SRCDIR
                            src dir for the project (configuration)
        --prefix=PREFIX     installation prefix (configuration) [default: '/usr/local/']
        --download          try to download the tools if missing
        --cxxflags=CXXFLAGS
                            C++ compiler flags

      C++ Compiler Configuration Options:
        --check-cxx-compiler=CHECK_CXX_COMPILER
                            On this platform (linux) the following C++ Compiler will be checked by default: "g++ icpc sunc++"

      Boost Configuration Options:
        --boost=BOOST_HOME  Basedir of your Boost installation
        --boost_inc=BOOST_INC
                            Include dir of your Boost installation
        --boost_lib=BOOST_LIB
                            Library dir of your Boost installation

      SystemC configuration Options:
        --systemc=SYSTEMC_HOME
                            Basedir of your SystemC installation
        --tlm2=TLM2_HOME    Basedir of your TLM-2.0 distribution
        --tlm2tests=TLM2_TESTS
                            Example of your TLM-2.0 distribution
        --scv=SCV_HOME      Basedir of your SCV distribution

      GreenSoCs Configuration Options:
        --greensocs=GREENSOCS_HOME
                            Basedir of your GreenSoCs instalation
        --amba=AMBA_HOME    Basedir of your AMBAKit distribution
        --grlib=GRLIB_HOME  Basedir of your grlib distribution
        --ambaexamples=AMBA_EXAMPLES
                            Basedir of your AMBAKit Examples
   ~~~

2.  Execute “<tt>./waf</tt>” – to build all models and tests.

  As an alternative you may select a specific target (test or library) using 
  “./waf --targets=”list,of,targets””. 

  A list of targets can be generated with “./waf list”.

3.  Execute “./waf docs” – to generate the source doxygen documentation. 

4.  Execute "./waf generate" to start Platform Generator.

5.  Execute "./waf" again to compile the created platforms.

