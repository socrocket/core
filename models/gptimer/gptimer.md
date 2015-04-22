GPTimer - General Purpose Timer SystemC Model {#gptimer_p}
==========================================================

[TOC]

@section gptimer_p1 Functionality and Features

The GPTimer unit acts as a slave at the APB bus. Its basic functionality is a countdown mechanism that asserts an interrupt on underflow. The GPTimer unit consists of a prescaler unit that is generating ticks and up to seven counter units that are decrementing on prescaler ticks. In the VHDL model, the counter units are named ‘timers’ just like the entire IP model. As this is a potential source of confusion, the name has been changed to ‘counters’ in the TLM implementation.
The GPTimer unit can be configured and operated through its registers addressed through the APB interface. All registers have a width of 32 bits and are summarized in Table 18. 

@table Table 18 - GPTimer Registers
| APB Address Offset | Register                         |
|--------------------|----------------------------------|
| 0x00               | Scaler Value                     |
| 0x04               | Scaler Reload Value              |
| 0x08               | Configuration Register           |
| 0x0C               | Unused                           |
| 0xn0               | Counter n Value Register         |
| 0cn4               | Counter n Reload Register        |
| 0xn8               | Counter n Configuration Register |
| 0xnC               | Unused                           |
@endtable

@register gpcounter_reload GPCounter Reload Value Register
  [31:0](COUNTER_RELOAD_VALUE) Timer Reload value. This value is loaded into the timer counter value register when '1' is written to load bit in the timers control register or when the RS bit is set in the control register and the timer underflows. Any unused most significant bits are reserved. Always reads as '000...0'.
@endregister

@register gpcounter_conf GPCounter Configuration Register
  [6](DH) Debug Halt: Value of GPTI.DHALT signal which is used to freeze counters
                      (e.g. when a system is in debug mode). Read-only.
  [5](CH) Chain: Chain with preceding timer. If set for timer n, 
                 decrementing timer n begins when timer (n-1) underflows.
  [4](IP) Interrupt Pending: The core sets this bit to '1' when an interrupt is signalled. 
          This bit remains '1' until cleared by writing '1' to this bit, writes of '0' have no effect.
  [3](IE) Interrupt Enable: If set the timer signals interrupt when it underflows.
  [2](LD) Load: Load value from the timer reload register to the timer counter value register.
  [1](RS) Restart: If set, the timer counter value register is reloaded with the value of the reload register when the timer underflows
  [0](EN) Enable: Enable the timer.
@endregister


The prescaler and all the timers are equipped with a value register and a reload value register. The value register is decremented on each trigger and can be reset to the reload value on underflow or on reset command. In the VHDL model, the trigger for decrementing the prescaler is the bus clock input of the GPTimer unit. In the SystemC model the prescaler ticks are calculated by multiplying the clock period with the prescaler reload register. The clock period is stored to the ‘clock_cycle’ variable, which can be set using one of the overloaded ‘clk’ functions. The triggers for decrementing the counters are ticks issued by prescaler underflow. The prescaler is automatically reset on underflow and cannot be halted. Due to a specific characteristic of the VHDL implementation of the GPTimer unit, the prescaler reload value must be greater than or equal to the number of counters implemented in the GPTimer instance.

The configuration register located at address 0x08 can be used to configure the GPTimer unit. The counter n configuration registers located at addresses 0xn8 can be used to configure the individual counters.

The configuration register consists of four fields, DF, SI, IRQ, and TIMERS. The DF field is the only field that can by modified dynamically, all other fields are read only, i.e. their values are determined by VHDL generics and written to the registers at system startup.

The <b>DF (disable freeze) field</b> disables the sensitivity to the dhalt input signal. This signal can be used to freeze the timer value registers, if DF is disabled.

The <b>SI (separate interrupt) field</b> specifies whether each counter asserts an individual interrupt line or all counters assert the same interrupt line. If all counters assert the same interrupt line, this line is specified in the IRQ field. Else, counter 1 asserts the interrupt specified in the IRQ field and all other counters are distributed to the subsequent lines. The highest line must not exceed the maximum number of interrupts in the system. For more information on the interrupt scheme, please refer to chapter 8.

The <b>TIMERS field</b> specifies the number of counters in the system.
The counter configuration registers are used to configure and control the counters. The counters are controlled by the enable, load, and debug halt fields. Debug halt freezes the counter value register, load immediately reloads the value register with the contents of the reload value register, and enable can be used to enable or disable the counter.

To increase the counting delay, chaining can be activated for individual counters. If counter n is chaining mode, it does not decrement on prescaler ticks, but on ticks generated by an underflow of the previous counter (n-1). For this operating mode, counter (n-1) must be in restart mode, i.e. its value register is automatically reloaded from the reload value register on underflow. In addition, the interrupt assertion of a counter can be disabled, which would be reasonable for counter (n-1) in the described example. It is possible to enable chaining for multiple counters to wait for very long periods.

In addition, it is possible to configure the last counter as a watchdog using the wdog generic. This generic can be set to an alternative reload value, which will be used to set and reload the counter. The watchdog counter will be started on timer reset and cause an assertion of the wdog output on underflow.

@section gptimer_p2 Internal Structure
The TLM implementation of the GPTimer comprises two classes, GPTimer and GPCounter. Implementing the counter unit in a class of its own enables the GPTimer unit to be instantiated with a variable number of counters, which are dynamically instantiated in the constructor of the GPTimer class. For both classes, the definition is put into header files (gptimer.h, gpcounter.h) and the implementation is put into C++ source files (gptimer.cpp, gpcounter.cpp). The contents of these files are described in the subsequent sections.

@subsection gptimer_p2_1 The gptimer.h file
The ‘gptimer.h’ file contains the module class definition. Any communication with the environment is performed through the GPTimer class defined in this file. The Counters are fully encapsulated in the Timer module.

@subsubsection gptimer_p2_1_1 Parameterization of the module
The parameterization options, implemented as generics in the VHDL model, are realized as constructor parameters of the GPTimer class. This makes the module parametrizable during instantiation. Details on the parameters are given in section 7.3.

@subsubsection gptimer_p2_1_2 Configuration of the module
The GPTimer unit is configurable through its Timer configuration register and its Counter configuration registers. The configuration registers, which are accessible through the APB bus, are modeled and accessed through the comfortable mechanisms provided by sr registers. To ensure compatibility, the GPTimer class needs to be a child module of APBSlave. APBSlave is an encapsulation for a complete functional unit and provides containment structures for other elements, e.g. registers. Thus, the GPTimer class inherits the gr_device class. 

The ‘gptimer.h’ file contains const variables defining register addresses and bit masks. These definitions are made for programming convenience.
The write masks of the registers can be used to ensure that only permitted bits are set when writing to a register. They can also be applied for reading specific fields of a register masking all other bits.

@subsubsection gptimer_p2_1_3 Communication with the module
Apart from the APB communication directed to the registers of the GPTimer, the module is equipped with five signals for direct communication with the master devices.
  - The rst input signal triggers the reset function do_reset of the module….
  - The IRQ output signal is used to launch an interrupt on counter underflow. The according interrupt line will be provided as the value of this uint32_t type signal.
  - The wdog output signal is required if the timer is used as a watchdog. The signal will then be asserted on underflow of Counter 1.

@subsubsection gptimer_p2_1_4 Operation of the module
The GPTimer class definition contains the module interface and the function prototypes of constructor, destructor, SystemC proesses, callback functions, and pure C++ software routines. The GPTimer unit needs to assert interrupt signals at the correct points of time and therefore needs an SC_THREAD process to keep track of time. A second SC_THREAD is used for debug only and is disabled by default.

The SystemC processes have to be registered with the SystemC simulation kernel using the SystemC macro, SC_HAS_PROCESS().
In addition, some class attributes are defined to keep track of the overall state of operation of the module:
  - A lasttime variable stores the last timestamp at which the value of the prescaler has been know. This time is required as a reference for any calculation of ticks.
  - A lastvalue variable stores the contents of the prescaler value register at the time stored in lasttime. The prescaler value is known when it is calculated With the information given in lasttime and lastvalue it is possible to calculate the next tick.

@section gptimer_p3 Parametrization Options
The model can be parametrized through the constructor arguments of class timer. All available options are listed in Table 19.

@table GPTimer Parameters
| Parameter | Description                        |
|-----------|------------------------------------|
|name      | The name of the SystemC instance   |
|ntimers   | Number of counters (1-7)           |
|pirq      | Defines which APB interrupt the timers will generate.    |
|sepirq    | If set to 1, each timer drives an individual interrupt line, starting with interrupt pirq. If set to 0, all timers will drive the same interrupt line. |
|nbits     | Bitwidth of the counters           |
|sbits     | Bitwidth of prescaler              |
|wdog      | Watchdog reset value.             |
@endtable

@copydoc GPTimer::GPTimer

@section gptimer_p4 Interface
The control registers of the module can be accessed through a GreenSocs APB slave socket. In addition, the module provides a set of SignalKit sockets. All socket are implemented in the Timer top-level class.

@table Timer SignalKit sockets
|Name | Type     | In/Out | Description                      |
|-----|----------|--------|----------------------------------|
|rst  | bool     | in     | reset prescaler and all counters |
|irq  | uint32_t | out    | interrupt lines                  |
@endtable

@section gptimer_p5 Compilation Instructions
For the compilation of the Timer IP, a WAF wscript is provided and integrated in the superordinate build mechanism of the TLM model library of the Hardware-Software SystemC Co-Simulation SoC Validation Platform project.
@section gptimer_p6 Example Instantiation

Instantiation of Timer with 4 Counters:

~~~{.cpp}
  GPTimer dut("gptimer", 4);
~~~

Bind APB socket:

~~~{.cpp}
  tb.master_sock(dut.bus);
~~~

Bind SignalKit ports:

~~~{.cpp}
  dut.rst(tb.rst);
  tb.irq(dut.irq);
~~~

