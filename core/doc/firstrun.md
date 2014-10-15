Running Software {#firstrun}
============================

After having compiled the library and having generated a configuration the system is ready for simulation. 
Example software can be found in the `./software` sub-directory. 
Most of the code located here has already been pre-compiled for unit testing. 
For manual compilation follow the flow described in the Gaisler BCC – Bare-C Cross-Compiler User’s Manual [12].

A simple “Hello World” program in C language is given in:
~~~
./software/grlib_tests/hello.c
~~~~

To compile this program using the BCC compiler enter (at top-level):
~~~
./waf --target=hello.sparc
~~~
*Hint: Add the `-v` switch to this call to see the full compiler command.*

The generated SPARC binary will be written to:
~~~
./build/core/software/grlib_tests/hello.sparc
~~~

This binary is independent of the system configuration. 
To execute the program the generated image must be loaded into ROM at the beginning of the simulation. 
Therefore, the name and the location of the application must be entered in the system configuration file. 
Furthermore some bootcode is needed to start up the system and execute the program in RAM.
This can be achieved via setting of commandline options.
The simulation can then be started as follows:
~~~
./build/core/platforms/leon3mp/leon3mp.platform --option conf.mctrl.prom.elf=build/core/software/prom/sdram/sdram.prom --option conf.mctrl.ram.sdram.elf=build/core/software/grlib_tests/hello.sparc --option conf.apbuart.en=1
~~~

You are eventually asked to connect a terminal a startup. 
In this case open another terminal and enter (Linux):
~~~
telnet localhost <PORT NUMBER>.
~~~
*Hint: Call leon3mp --help to see all available command line options.*

