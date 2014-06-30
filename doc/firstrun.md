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
./build/software/grlib_tests/hello.sparc
~~~

This binary is independent of the system configuration. 
Obtaining an executable file requires additional settings. 
Just as for GRLIB hardware simulations this is achieved using the `mkprom` tool (part of the BCC distribution). 
The tool inserts boot code and generates a compressed image of the application. 
For a leon3 single core system in default configuration call:
~~~
mkprom2 –v –freq 50 –nocomp –nosram –rmw –sdram 16 –msoftfloat –o hello.sparc.prom hello.sparc
~~~

To execute the program the generated image must be loaded into ROM at the beginning of the simulation. 
Therefore, the name and the location of the application must be entered in the system configuration file. 
This can be done by manually editing the configuration (JSON file) or by using the set_json_attr utility:
~~~
./tools/set_json_attr myconf.json conf.mctrl.prom.elf="hello.sparc.prom" > myhelloconf.json
~~~

Now the simulation can be started as follows:
~~~
./build/platforms/leon3mp/leon3mp.platform –s {LUASCRIPT} –j myhelloconf.json
~~~

Depending on the configuration you are eventually asked to connect a terminal a startup. 
In this case open another terminal and enter (Linux):
~~~
telnet localhost <PORT NUMBER>.
~~~
*Hint: Call leon3mp --help to see all available command line options.*

