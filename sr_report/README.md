SoCRocket SystemC Reporting Extensions {#sr_report}
======================================

While developing models for [SoCRocket](https://socrocket.github.io/) we needed a good SystemC reporting system.
We started with something much simpler and closer to the C++ standard (VerbosityKit).
It was sufficient until we introduced many models and worked with multiple people 
on very different aspects of the platform.

The challenge to have as much logging messages as needed for each problem without 
removing them to speed up the simulation and flexible switch on or off more messages
made us rethink the way we would like to log our messages.

Furthermore we wanted to use the same channels as our foundation SystemC. 
So we pluged this solution into the sc_report_handler routiens.

Moreover long analysises of logfile keept us wishing for a more flexible aproach for analysing and storeing the data.
So we introduced a scriptable backend and preserved the types of loged primitives.

This solution sends standard SystemC sc_reported extended by Key/Value of primitive types 
through the SystemC sc_report_handler, it extends the filtering capabilities of sc_report 
by fast black-/white-list filtering of sc_object IDs and has a Python backend handler to 
utilize databases for the analysis work.

Usage
-----

To use the sr_report frontend include the sr_report header in your code and use the following macros:
srInfo, srWarn, srError and srFatal which represent the standard levels of sc_report reporting and additional
srDebug, srAnalyse and srMessage.

All these macros can be used like this:

~~~~ {.cpp}
// Inside a sc_object
srInfo()
  ("key1", value1)
  ("key2", value2)
  ("message");

// everywhere
srInfo("identifier")
  ("key1", value1)
  ("key2", value2)
  ("message");

~~~~

Inside of classes derived of sc_object the identifier will be set automaticaly to the sc_object hierachial name.

To use the default C++ backend handler you have to set it as early as possible in your sc_main:

~~~~{.cpp}
sr_report_handler::handler = sr_report_handler::default_handler;
~~~~

To replace the backend hander by your own set your own handler instead of the default_handler.

License
-------

This code is avaliable under [Apache-2.0](https://spdx.org/licenses/Apache-2.0.html) license.
