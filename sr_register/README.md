SoCRocket Register Implementation {#sr_register}
=================================

This repository provides the foundation used by [SoCRocket](https://socrocket.github.io/) 
to model memory mapped registers. It is based on work
from Cadence. They proposed an interface for register introspection
[here](http://forums.accellera.org/files/file/102-proposed-interfaces-for-interrupt-modeling-register-introspection-and-modeling-of-memory-maps-from-stmicroelectronics-arm-and-cadence/) which we used as a starting point.

Due to the fact that in previous versions of SoCRocket we used [GreenReg](http://git.greensocs.com/greenlib/greenlib/tree/master/greenreg) from [GreenSoCs](http://www.greensocs.com/)
to model memory mapped registers. We wanted to keep a similar interface.
Therefore we extended the sc_register interfaces to sr_register.
sr_register allows instantiation and handling of registers in a similar 
manner as GreenReg but has much a smaller code footprint.
Furthermore sr_register supports the scireg interface and is therefore comaptible 
with the proposed register interface by Cadence.

Furthermore it is easily possible to use register fields or attach callbacks
to registers.

Usage
-----

Instantiate in your sc_model a sr_register_bank with needed address and data word size.
~~~~{.h}
#include "sr_register.h"

class Device : public sc_core::sc_module {
    public:
        sr_register_bank<ADDR_TYPE,DATA_TYPE> reg_bank;
        void init_registers();
        void post_write_callback();
}
~~~~

Furthermore you will need implementations of the sr_hierarchy_pop() and sr_hierarchy_push() functions. Ours can be found in the [core repository](https://github.com/socrocket/core) in base.h.

The register bank exposes functions to create registers:
Call sr_register_bank::create_register for each register you want to introduce in your
model constructor. Usually the function call is located within a init_registers() function.
The function will return a reference on the new register so you can directly declare 
bit fields or connect callbacks:

~~~~{.cpp}
Device::Device() : 
    reg_bank("register bank name") {
    Device::init_registers();    
}

Device::init_registers() {
    reg_bank.create_register("register_name", "A Human readable description of the register",
          ADDRESS_OFFSET, DEFAULT_VALUE, BUS_WRITE_MASK)
        .callback(SR_POST_WRITE, this, &Device::post_write_callback)
        .create_field("six_bit_field", 6, 0)
        .create_field("single_bit", 18, 18);
}
~~~~

Each sr_register function (callback, create_field) returns a reference 
to the corresponding register again. This allows the chaining of the function calls.

To connect the register base to a bus it exposes the functions bus_read and bus_write.
The bus interface implementation is responsible for handling error, delay and endianess.

The registers behave like an array. You can access them via reg_bank[OFFSET].

License
-------

Due to the fact that the original implementation of Cadence was published under [Apache-2.0](https://spdx.org/licenses/Apache-2.0.html)
license. We follow the example and publish this code under Apache-2.0 license as well.


