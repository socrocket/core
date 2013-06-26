Installation {#install}
=======================

The build system shipped with the SoCRocket library is written in `waf`. 
It requires at least Python 2.3 to run. 
The `waf` executable is located in the root directory of the library. 

@section install1 WAF Command Overview

Execute `./waf –h` to get an overview of and help on all available commands and options:

~~~
waf [command] [options]

The following main commands are supported:

build    : executes the build
clean    : cleans the project
configure: configures the project
coverage : If configured with -G, and lcov/gcof installed, generates 
           a code coverage report
dist     : makes a tarball for redistributing the sources
distcheck: checks if the project compiles (tarball from 'dist')
distclean: removes the build directory
docs      : build source code documentation* (if doxygen installed)
install  : installs the targets on the system
list     : lists the targets to execute
macclean : Clean garbage files from source tree
setprops : Set SVN properties on all source files
step     : executes tasks in a step-by-step fashion, for debugging
uninstall: removes the targets installed
update   : updates the plugins from the *waflib/extras* directory

generate : opens the configuration 
~~~

*Doxygen support is currently under development. The generated documents may be incomplete.*

@section install2 Building the library

Building the project requires following steps:

1. *Execute `./waf configure` to configure the build environment*
   The configuration step succeeds in case all the required software packages are available. 
   Otherwise, it fails and shows the broken dependency. 
   If so, the install path variable must be corrected. 
   It is also possible to specify the location of a missing package. 
   Use `./waf –h` to see all the different options (e.g. `--systemc`, `--tlm`).
   As mentioned in \ref{requirements4}, SystemC/VHDL co-simulation can be optionally disabled, in order to be independent of commercial tools or, eventually, save compilation time (`./waf configure –nomodelsim`).
  Another important switch controls the verbosity of the output that is directed to stdout during simulation (`--verbosity=1..5`). 
  Verbosity level one only display error messages. 
  Level two includes warnings that are issued during simulation. 
  Level three prints execution statistics and analysis reports. 
  The recommended setting (default) is Level four. 
  It additionally shows configuration reports and information about the progress of tests. 
  The highest verbosity is bound to level 5. 
  It displays a message for each state-change of a transaction, which tremendously slows down simulation and is therefore only recommended for debugging.
  (Add the `–G` switch to the configure command in case you plan on running coverage calculation. This will have a penalty on the system performance.)
2. *Compile library and run unit tests*
  Execute `./waf` to compile all targets. Optionally, the `–jN` flag can be used to define to maximum number of parallel threads. 
  If co-simulation is configured make sure you have enough licenses to execute N instances of Modelsim.
  As an alternative, you may select a specific target (test or library) for compilation. 
  A list of targets can be generated with `./waf list`. Selective compile is done using `./waf –targets=”comma,separated,list,of,targets”`.
  After successful compilation the system automatically starts the respective unit test(s) and displays the result on the screen.
3. *Optional Doxygen and Gcov*
  In the final step you may generate additional documentation using Doxygen (`./waf docs`) or perform a test coverage calculation using Gcov/Lcov (`./waf coverage`). 
  The configuration step does not check the presence of these tools. 
  In case they are not present, the commands have no effect.

Moreover, to enable coverage calculation the system must be configure with the `–G` option. 
Compile the library and execute all unit tests in order to achieve meaningful results.

@section install3 Models separate compilation

The models of the library can be compiled separately and independently of each other. 
The following minimum set of source files is required for successful compilation:

~~~
./common
./contrib
./Doxyfile
./models/<selected_model>
./models/utils
./models/wscript
./signalkit
./tools
wscript
waf 
~~~
