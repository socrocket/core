Running Software {#firstrun}
============================

After having compiled the library and having generated a configuration the system is ready for simulation. 
Example software can be found in the `core/software` sub-directory. 
Most of the code located here has already been pre-compiled for unit testing. 
For manual compilation follow the flow described in the [Gaisler BCC – Bare-C Cross-Compiler User’s Manual](http://www.gaisler.com/doc/bcc.pdf).

A simple “Hello World” program in C language is given in: core/software/grlib_tests/hello.c

To compile this program using the BCC compiler enter (at top-level):
    
    $ ./waf --target=hello.sparc

*Hint: Add the `-v` switch to this call to see the full compiler command.*

The generated SPARC binary will be written to:
    
    build/core/software/grlib_tests/hello.sparc

This binary is independent of the system configuration. 
To execute the program the generated image must be loaded into ROM at the beginning of the simulation. 
Therefore, the name and the location of the application must be entered in the system configuration file. 
Furthermore some bootcode is needed to start up the system and execute the program in RAM.
This can be achieved via setting of commandline options.
The simulation can then be started as follows:
    
    $ core/tools/execute leon3mp sdram hello

*Hint: Call `build/core/platforms/leon3mp/leon3mp.platform --help` to see all available command line options.*

