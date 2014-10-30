SoCRocket SignalKit {#signalkit_doc}
-------------------------------

The SignalKit constitutes in a set of functions that allow signal communication in TLM-Style, without the overhead of maintaining payload objects. Within the library it is mainly used to model interupts and reset distribution.

Below is pasted from IP User Manual
@todo check if still accurate

Signal communication in TLM platforms is usually modeled using SystemC signals (sc_signals). SystemC signals are applied very similar to RTL signals and, more-or-less, represent hardware wires. To achieve the required level of accuracy, all reads and writes of sc_signals need to be scheduled by the SystemC kernel. For modeling at a higher level of abstraction this involves an unwanted overhead. One would prefer a fast function-call based (TLM-style) communication with a preference of retaining the natural, close to hardware, modeling style of sc_signals.
For this purpose this library provides an extra set of functions. The SoCRocket SignalKit can be found in the root directory of the project (./signalkit). Within the library it is mainly used to model the interrupt and reset distribution, but also for special purposes like dbus snooping. Syntax and application of SignalKit ports are very close to sc_signals. Although, signal transmission is performed by directed function calls, similar to TLM blocking transport. In contrast to TLM no payload handling is required. The general handling is very simple.

A module that is supposed to utilize SignalKit signals must include the signalkit.h header file and must call the SK_HAS_SIGNALS macro in its class definition. The following code example shows a SignalKit module with an outgoing port of type int:

~~~
#include "signalkit.h"

class source : public sc_module {

        SK_HAS_SIGNALS(source);
        SC_HAS_PROCESS(source);

        signal<int>::out out;

        // Constructor
        source(sc_module_name nm) : sc_module(nm), out("out") {

                SC_THREAD(run);

        }

        void run() {

                // ...
                out = i;
                // ...
        }
}
~~~

The actual signal output is defined in line 8. The output is written in line 20. Alternatively to  to the shown direct data assignment, the write method of the port may be used (out.write(i)).

The next code block shows a signal receiver:

~~~
#include "signalkit.h"

class dest : public sc_module {

        SK_HAS_SIGNALS(dest);

        signal<int>::in in;

        // Constructor
        dest(sc_module_name nm) : sc_module(nm), in(&dest::onsignal, "in") {

        }

        // Signal handler for input in
        void onsignal(const int &value, const sc_time &time) {

                // do something

        }

}
~~~

In line 10 the handler function onsignal is registered at SignalKit input in. If any call is received, this function will be triggered. Int value represents the data transmitted.

Sender and receiver can be connected using the SignalKit connect method. An example is given below:

~~~
#include "source.h"
#include "dest.h"

int sc_main(int argc, char *argv[]) {

        source src;
        dest dst;

        connect(src.out, dst.in);

        ...

        return 0;

}
~~~

Next to that trivial direct connection, the connect method is capable of handling broadcasting and muxing, and for converting between Signalkit and SystemC signals. For a broadcast the out signal may be directly connected to multiple ins. In the mux case, multiple transmitters are combined into one receiver. If required the transmitter may be identified by a channel number. 
