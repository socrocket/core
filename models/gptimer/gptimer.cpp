//*********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      gptimer.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    source file defining the implementation of the gptimer
//             model. Due to the fact that the gptimer class is a
//             template class it gets included by its definition in
//             gptimer.h
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Rolf Meyer
// Reviewed:
//*********************************************************************

#include <string>
#include "gptimer.h"

/// @addtogroup gptimer
/// @{

// Constructor: create all members, registers and Counter objects.
// Store configuration default value in conf_defaults.
CGPTimer::CGPTimer(sc_core::sc_module_name name, unsigned int ntimers,
                   int gpindex, int gpaddr, int gpmask, int gpirq, int gsepirq,
                   int gsbits, int gnbits, int gwdog, unsigned int pindex) :
    gr_device(name, gs::reg::ALIGNED_ADDRESS, 4 * (1 + ntimers), NULL),
    APBDevice(pindex, 0x1, 0x11, 0, gpirq, APBIO, gpmask, false, false, gpaddr), 
    bus("bus", r, (gpaddr & gpmask) << 8, (((~gpmask & 0xfff) + 1) << 8), ::amba::amba_APB, 
            ::amba::amba_LT, false), 
    rst(&CGPTimer::do_reset, "RESET"), 
    dhalt(&CGPTimer::do_dhalt, "DHALT"), 
    tick("TICK"), irq("IRQ"), wdog("WDOG"), 
    conf_defaults((gsepirq << 8) | ((gpirq & 0xF) << 3) | (ntimers & 0x7)), 
    lasttime(0, sc_core::SC_NS), lastvalue(0), 
    clockcycle(10.0, sc_core::SC_NS),
    sbits(gsbits),
    nbits(gnbits),
    wdog_length(gwdog) {


    assert("gsbits has to be between 1 and 32" && gsbits > 0 && gsbits < 33);
    assert("gnbits has to be between 1 and 32" && gnbits > 0 && gnbits < 33);
    assert("ntimers has to be between 1 and 7" && ntimers > 0 && ntimers < 8);
    assert("gwdog has to be between 0 and 2^nbits-1" && gwdog >= 0 && gwdog < (1LL << gnbits));
    // Display APB slave information
    v::info << this->name() << "APB slave @0x" << hex << v::setw(8)
            << v::setfill('0') << bus.get_base_addr() << " size: 0x" << hex
            << v::setw(8) << v::setfill('0') << bus.get_size() << " byte"
            << endl;


    /* create register */
    r.create_register("scaler", "Scaler Value",
                      0x00,                // offset
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | // config
                      gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      static_cast<unsigned int>((1ULL << gsbits) - 1), // init value
                      static_cast<unsigned int>((1ULL << gsbits) - 1), // write mask
                      // sbits defines the width of the value. Any unused most significant bits are reserved Always read as 0s.
                      32,                  // Register width. Maximum register with is 32bit sbit must be less than 32.
                      0x00                 // Lock Mask: Not implementet has to be zero.
    );
    
    r.create_register("screload", "Scaler Reload Value",
                      0x04, // offset
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO |      // config
                      gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      static_cast<unsigned int> ((1ULL << gsbits) - 1), // init value
                      static_cast<unsigned int> ((1ULL << gsbits) - 1), // write mask
                      // sbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s.
                      32,                                               // register width
                      0x00                                              // lock mask
    );
    
    r.create_register("conf", "Configuration Register",
                      0x08,                                        // offset
                      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | // config
                      gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                      conf_defaults,                               // init value
                      0x00000200,                                  // write mask
                      32,                                          // register width
                      0x00                                         // lock mask
    );

    for (unsigned int i = 0; i < ntimers; ++i) {
        CGPCounter *c = new CGPCounter(*this, i, gen_unique_name("CGPCounter",
                true));
        counter.push_back(c);
        r.create_register(gen_unique_name("value", false), "CGPCounter Value Register",
                          VALUE(i), // offset
                          gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | // config
                          gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                          0x00000000, // init value
                          static_cast<unsigned int> ((1ULL << gnbits) - 1), // write mask
                          // nbits defines the width of the value. Any unused most significant bits are reserved Always read as 0s.
                          32, // register width
                          0x00 // lock mask
        );
        
        r.create_register(gen_unique_name("reload", false), "Reload Value Register",
                          RELOAD(i), // offset
                          gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | // config
                          gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                          0x00000001, // init value
                          static_cast<unsigned int> ((1ULL << gnbits) - 1), // write mask
                          // nbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s.
                          32, // register width
                          0x00 // lock mask
        );
        
        r.create_register(gen_unique_name("ctrl", false), "Controle Register",
                          CTRL(i), // offset
                          gs::reg::STANDARD_REG | gs::reg::SINGLE_IO | // config
                          gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                          0x00000000, // init value
                          0x0000007F, //write mask
                          32, //register width
                          0x00 // lock mask
        );
    }

#ifdef DEBUG
    SC_THREAD(diag);
#endif

#ifdef DEBUGOUT
    SC_THREAD(ticking);
#endif
}

// Destructor: Unregister Register Callbacks.
// Destroy all Counter objects.
CGPTimer::~CGPTimer() {
    for (std::vector<CGPCounter *>::iterator iter = counter.begin(); iter
            != counter.end(); iter++) {
        delete *iter;
    }
    GC_UNREGISTER_CALLBACKS();
}

// Set all register callbacks
void CGPTimer::end_of_elaboration() {
    GR_FUNCTION(CGPTimer, scaler_read);
    GR_SENSITIVE(r[SCALER].add_rule(gs::reg::PRE_READ, "scaler_read",
            gs::reg::NOTIFY));

    GR_FUNCTION(CGPTimer, scaler_write);
    GR_SENSITIVE(r[SCALER].add_rule(gs::reg::POST_WRITE, "scaler_write",
            gs::reg::NOTIFY));
    GR_FUNCTION(CGPTimer, screload_write);
    GR_SENSITIVE(r[SCRELOAD].add_rule(gs::reg::POST_WRITE,
            "scaler_reload_write", gs::reg::NOTIFY));

    GR_FUNCTION(CGPTimer, conf_read);
    GR_SENSITIVE(r[CONF].add_rule(gs::reg::PRE_READ, "conf_read",
            gs::reg::NOTIFY));

}

#ifdef DEBUGOUT
// Calculate tick output for the prescaler tick TICK(0)
void CGPTimer::tick_calc() {
    e_tick.cancel();
    scaler_read();
    int scaler = r[SCALER] + 1;
    int reload = r[SCRELOAD] + 1;
    //sc_core::sc_time now = sc_core::sc_time_stamp();
    //int scaler = valueof(now) - reload;
    if(reload) {
        scaler = (scaler) % reload;
    }
    //v::debug << name() << "Scaler: " << scaler << v::endl;
    //v::debug << name() << "Reload: " << reload << v::endl;
    //if(((unsigned )scaler != 0xFFFFFFFF) || (scaler == 0 && reload == 0)) {
    //  e_tick.notify(clockcycle * (((scaler)? scaler: (reload - 1)) - 1));
    //}
    //if(reload != 0) {
    e_tick.notify(clockcycle * (scaler));
    //}
}

// Wait for prescaler tick output ans set TICK(0)
void CGPTimer::ticking() {
    while(1) {
        tick_calc();
        wait(e_tick);
        if(rst.read() == 1) {
            tick.write(tick.read() | 1);
        }
        wait(clockcycle);
        if(rst.read() == 1) {
            tick.write(tick.read() & ~1);
        }
    }
}
#endif

// Disable or enable CGPCounter when DHALT arrives
// The value will be directly fetched in conf_read to show the right value in the conf registers.
void CGPTimer::do_dhalt(const bool &value, const sc_core::sc_time &time) {
    if (r[CONF].b[CONF_DF]) {
        if (value) {
            for (std::vector<CGPCounter *>::iterator iter = counter.begin(); iter
                    != counter.end(); iter++) {
                (*iter)->stop();
            }
        } else {
            for (std::vector<CGPCounter *>::iterator iter = counter.begin(); iter
                    != counter.end(); iter++) {
                (*iter)->start();
            }
        }
    }
}

// Calback for scaler register. Updates register value before reads.
void CGPTimer::scaler_read() {
    sc_core::sc_time now = sc_core::sc_time_stamp();
    int reload = r[SCRELOAD] + 1;
    int value = valueof(now, 0, clockcycle) - (reload);
    //  v::debug << name() << " pure: " << std::dec << valueof(now) << "->" << value << v::endl;

    //  if(value<0) {
    if (reload) {
        value = value % (reload);
    }
    r[SCALER] = reload + value - 1;
    //  v::debug << name() << " reg: " << std::dec << value << " +reload: " << reload + value << v::endl;
    //  } else {
    //  if(reload + 1) {
    //    value = value % (reload + 1);
    //  }
    //    r[SCALER] = value;
    //    v::debug << name() << "unten" << std::dec << value << v::endl;
    //  }
    // lastvalue = r[SCALER];
    // lasttime  = now;
}

// Callback for scaler relaod register. Updates Prescaler Ticks and all Counters on write.
void CGPTimer::screload_write() {
    uint32_t reload = r[SCRELOAD];
    r[SCALER] = reload;
    //  v::debug << name() << "!Reload: " << reload << v::endl;
    scaler_write();
}

// Callback for scaler value register. Updates Prescaler Ticks and all Counters on write.
void CGPTimer::scaler_write() {
    lasttime = sc_core::sc_time_stamp();
    lastvalue = r[SCALER];
    v::debug << name() << "Scaler: " << lastvalue << v::endl;
    for(std::vector<CGPCounter *>::iterator iter = counter.begin(); iter != counter.end(); iter++) {
        (*iter)->calculate(); // Recalculate
    }
#ifdef DEBUGOUT
    tick_calc();
#endif
}

// Callback for configuration register. Updates the content before reads.
void CGPTimer::conf_read() {
    r[CONF] = (r[CONF] & 0x0000200) | (conf_defaults & 0x000000FF);
}

// Calback for the rst signal. Resets the module on true.
void CGPTimer::do_reset(const bool &value, const sc_core::sc_time &time) {
    if (!value) {
        r[SCALER] = static_cast<unsigned int> ((1ULL << sbits) - 1);
        r[SCRELOAD] = static_cast<unsigned int> ((1ULL << sbits) - 1);
        r[CONF] = conf_defaults;
        lastvalue = 0;
        tick = 0;
        wdog = 0;
        // TODO Reset Irqs: irq = 0;
        lasttime = sc_core::sc_time_stamp();
        scaler_write();
        scaler_read();

        for (std::vector<CGPCounter *>::iterator iter = counter.begin(); iter
                != counter.end(); iter++) {
            (*iter)->do_reset();
        }
        if(wdog_length) {
            size_t count = counter.size() -1;
            r[CGPTimer::RELOAD(count)] = wdog_length;
            r[CGPTimer::CTRL(count)] = 0xD; // Enabled, Load, Irq
            counter.back()->ctrl_write();
        }
    }
}

// Prescaler value relative to the current value.
int CGPTimer::valueof(sc_core::sc_time t, int offset,
                      sc_core::sc_time cycletime) const {
    return (int)(lastvalue - ((t - lasttime - (1 + offset) * cycletime)
            / cycletime) + 1);
}

// Number of zero crosses between two prescaler values.
int CGPTimer::numberofticksbetween(sc_core::sc_time a, sc_core::sc_time b,
                                   int CGPCounter, sc_core::sc_time cycletime) {
    int reload = r[SCRELOAD] + 1;
    int val_a = valueof(a, 0, cycletime);
    int val_b = valueof(b, CGPCounter, cycletime);
    int num_a = val_a / reload + (val_a > 0 && val_b < 0);
    int num_b = val_b / reload;
    return std::abs(num_a - num_b);
}

// Extract basic cycle rate from a sc_clock
void CGPTimer::clk(sc_core::sc_clock &clk) {
    clockcycle = clk.period();
}

// Extract basic cycle rate from a clock period
void CGPTimer::clk(sc_core::sc_time &period) {
    clockcycle = period;
}

// Extract basic cycle rate from a clock period in double
void CGPTimer::clk(double period, sc_core::sc_time_unit base) {
    clockcycle = sc_core::sc_time(period, base);
}

#ifdef DEBUG
#define SHOWCGPCounter(n) \
    CGPCounter[n]->value_read(); \
    v::debug << name() << " CGPTimer"#n << ":{ v:" << r[TIM_VALUE(n)] << ", r:" << r[TIM_RELOAD(n)] << "}";

// Diagnostic thread.
void CGPTimer::diag() {
    while(1) {
        std::printf("\n@%-7s /%-4d: ", sc_core::sc_time_stamp().to_string().c_str(), (unsigned)sc_core::sc_delta_count());
        scaler_read();
        std::cout << "Scaler:{ v:" << r[SCALER] << ", r:" << r[SCRELOAD] << "}";
        SHOWCGPCounter(0);
        SHOWCGPCounter(1);
        SHOWCGPCounter(2);
        wait(clockcycle);
    }
}
#endif

/// @}

