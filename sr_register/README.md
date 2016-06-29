SoCRocket Register Implementation {#sr_register}
=================================

This repository provides the foundation used by [SoCRocket](https://socrocket.github.io/) 
to model memory mapped registers. It is based on work
from Cadence. They proposed an interface for register introspection
[here](http://forums.accellera.org/files/file/102-proposed-interfaces-for-interrupt-modeling-register-introspection-and-modeling-of-memory-maps-from-stmicroelectronics-arm-and-cadence/) which we used as startingpoint.

Due to the fact that in previous versions of SoCRocket we used [GreenReg](http://git.greensocs.com/greenlib/greenlib/tree/master/greenreg) from [GreenSoCs](http://www.greensocs.com/)
to model memory mapped registers. We wanted to keep a simmilar interface.
Therefore we extended the sc_register interfaces to sr_register.
sr_register allow instantiation and handeling of registers in a simmilar 
manner as GreenReg but have much smaller code.
Furthermore they support the scireg interfacing and are therefor comaptible 
with the proposed register interface by Cadence.

Furthermore it is easiely possible to use register fields or attach callbacks
to registers.

Usage
-----

Instanciate in your sc_model a sr_register_bank with needed address and data word size.
the register bank exposes functions to create registers:
Call sr_register_bank::create_register for each register you want to introduce in your
model constructor.
The function will return a reference on the new register so you can directly declare 
bit fields or connect callbacks:

~~~~{.cpp}
r.create_register("register_name", "A Human readable description of the register",
      ADDRESS_OFFSET, DEFAULT_VALUE, BUS_WRITE_MASK)
    .callback(SR_POST_WRITE, this, &Klass::post_write_callback)
    .create_field("six_bit_field", 6, 0)
    .create_field("single_bit", 18, 18);
~~~~

Each sr_register function (callback, create_field) is returning a reference 
to the coresponding register again. This allows the chaining of the function calls.

To connect the register base to a bus it exposes the functions bus_read and bus_write.
The bus interface implementation is responsible for handeling error, delay and enianess.

License
-------

Due to the fact that the original implementation of Cadence was published under [Apache-2.0](https://spdx.org/licenses/Apache-2.0.html)
license. We follow the example and publish this code under Apache-2.0 license as well.


