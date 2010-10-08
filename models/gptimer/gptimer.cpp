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

CGPTimer::CGPTimer(sc_core::sc_module_name name, unsigned int ntimers,
                   int gpindex, int gpaddr, int gpmask, int gpirq, int gsepirq,
                   int gsbits, int gnbits, int gwdog) :
    gr_device(name, gs::reg::ALIGNED_ADDRESS, 4 * (1 + ntimers), NULL),
            CGrlibDevice(0x1, 0x11, 0, 0, gpirq, GrlibBAR(APBIO, gpmask, false,
                    false, gpaddr)), bus("bus", r, gpaddr << 20,
                    (4095 - gpmask) << 20, ::amba::amba_APB, ::amba::amba_LT,
                    false), rst(&CGPTimer::do_reset, "RESET"), dhalt(
                    &CGPTimer::do_dhalt, "DHALT"), tick("TICK"), irq("IRQ"),
            wdog("WDOG"), conf_defaults((gsepirq << 8) | ((gpirq & 0xF) << 3)
                    | (ntimers & 0x7)), lasttime(0, sc_core::SC_NS), lastvalue(
                    0), clockcycle(10.0, sc_core::SC_NS) {

    /* create register */
    r.create_register("scaler", "Scaler Value",
    /* offset */0x00,
    /* config */gs::reg::STANDARD_REG | gs::reg::SINGLE_IO
            | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
    /* init value */0xFFFFFFFF,
    /* write mask */(2 << (gsbits)) - 1, /* sbits defines the width of the value. Any unused most significant bits are reserved Always read as 0s. */
    /* reg width */32, /* Maximum register with is 32bit sbit must be less than 32. */
    /* lock mask */0x00 /* Not implementet has to be zero. */
    );
    r.create_register("screload", "Scaler Reload Value",
    /* offset */0x04,
    /* config */gs::reg::STANDARD_REG | gs::reg::SINGLE_IO
            | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
    /* init value */0xFFFFFFFF,
    /* write mask */static_cast<unsigned int> ((1ULL << gsbits) - 1), /* sbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s. */
    /* reg width */32,
    /* lock mask */0x00);
    r.create_register("conf", "Configuration Register",
    /* offset */0x08,
    /* config */gs::reg::STANDARD_REG | gs::reg::SINGLE_IO
            | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
    /* init value */conf_defaults,
    /* write mask */0x00003FFF,
    /* reg width */32,
    /* lock mask */0x00);

    for (unsigned int i = 0; i < ntimers; ++i) {
        CGPCounter *c = new CGPCounter(*this, i, gen_unique_name("CGPCounter",
                true));
        counter.push_back(c);
        r.create_register(
                gen_unique_name("value", false),
                "CGPCounter Value Register",
                /* offset */VALUE(i),
                /* config */gs::reg::STANDARD_REG | gs::reg::SINGLE_IO
                        | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                /* init value */0x00000000,
                /* write mask */static_cast<unsigned int> ((1ULL << gnbits) - 1), /* nbits defines the width of the value. Any unused most significant bits are reserved Always read as 0s. */
                /* reg width */32,
                /* lock mask */0x00);
        r.create_register(
                gen_unique_name("reload", false),
                "Reload Value Register",
                /* offset */RELOAD(i),
                /* config */gs::reg::STANDARD_REG | gs::reg::SINGLE_IO
                        | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
                /* init value */0x00000001,
                /* write mask */static_cast<unsigned int> ((1ULL << gnbits) - 1), /* nbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s. */
                /* reg width */32,
                /* lock mask */0x00);
        r.create_register(gen_unique_name("ctrl", false), "Controle Register",
        /* offset */CTRL(i),
        /* config */gs::reg::STANDARD_REG | gs::reg::SINGLE_IO
                | gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
        /* init value */0x00000000,
        /* write mask */0x0000007F,
        /* reg width */32,
        /* lock mask */0x00);
    }

#ifdef DEBUG
    SC_THREAD(diag);
#endif

#ifdef DEBUGOUT
    SC_THREAD(ticking);
#endif
}

CGPTimer::~CGPTimer() {
    for (std::vector<CGPCounter *>::iterator iter = counter.begin(); iter
            != counter.end(); iter++) {
        delete *iter;
    }
    GC_UNREGISTER_CALLBACKS();
}

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
    //std::cout << "Scaler: " << scaler << std::endl;
    //std::cout << "Reload: " << reload << std::endl;
    //if(((unsigned )scaler != 0xFFFFFFFF) || (scaler == 0 && reload == 0)) {
    //  e_tick.notify(clockcycle * (((scaler)? scaler: (reload - 1)) - 1));
    //}
    //if(reload != 0) {
    e_tick.notify(clockcycle * (scaler));
    //}
}

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

/* Disable or enable CGPCounter when DHALT arrives
 * The value will be directly fetched in conf_read to show the right value in the conf registers.
 */
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

void CGPTimer::scaler_read() {
    sc_core::sc_time now = sc_core::sc_time_stamp();
    int reload = r[SCRELOAD] + 1;
    int value = valueof(now, 0, clockcycle) - (reload);
    //  std::cout << " pure: " << std::dec << valueof(now) << "->" << value;

    //  if(value<0) {
    if (reload) {
        value = value % (reload);
    }
    r[SCALER] = reload + value - 1;
    //  std::cout << " reg: " << std::dec << value << " +reload: " << reload + value << std::endl;
    //  } else {
    //  if(reload + 1) {
    //    value = value % (reload + 1);
    //  }
    //    r[SCALER] = value;
    //    std::cout << "unten" << std::dec << value << std::endl;
    //  }
    // lastvalue = r[SCALER];
    // lasttime  = now;
}
void CGPTimer::screload_write() {
    uint32_t reload = r[SCRELOAD];
    r[SCALER] = reload;
    //  std::cout << "!Reload: " << reload << std::endl;
    scaler_write();
}

void CGPTimer::scaler_write() {
    lasttime = sc_core::sc_time_stamp();
    lastvalue = r[SCALER];
    //  std::cout << "!Scaler: " << lastvalue << std::endl;
#ifdef DEBUGOUT
    tick_calc();
#endif
}

void CGPTimer::conf_read() {
    r[CONF] = (r[CONF] & 0x0000030) | (conf_defaults & 0x0000000F);
}

void CGPTimer::do_reset(const bool &value, const sc_core::sc_time &time) {
    if (!value) {
        r[SCALER] = 0xFFFFFFFF;
        r[SCRELOAD] = 0xFFFFFFFF;
        r[CONF] = conf_defaults;
        lastvalue = 0;
        tick = 0;
        wdog = 0;
        lasttime = sc_core::sc_time_stamp();
        scaler_write();
        scaler_read();

        for (std::vector<CGPCounter *>::iterator iter = counter.begin(); iter
                != counter.end(); iter++) {
            (*iter)->do_reset();
        }
    }
}

int CGPTimer::valueof(sc_core::sc_time t, int offset,
                      sc_core::sc_time cycletime) const {
    return (int)(lastvalue - ((t - lasttime - (1 + offset) * cycletime)
            / cycletime) + 1);
}

int CGPTimer::numberofticksbetween(sc_core::sc_time a, sc_core::sc_time b,
                                   int CGPCounter, sc_core::sc_time cycletime) {
    int reload = r[SCRELOAD] + 1;
    int val_a = valueof(a, 0, cycletime);
    int val_b = valueof(b, CGPCounter, cycletime);
    int num_a = val_a / reload + (val_a > 0 && val_b < 0);
    int num_b = val_b / reload;
    return std::abs(num_a - num_b);
}

void CGPTimer::clk(sc_core::sc_clock &clk) {
    clockcycle = clk.period();
}

void CGPTimer::clk(sc_core::sc_time &period) {
    clockcycle = period;
}

void CGPTimer::clk(double period, sc_core::sc_time_unit base) {
    clockcycle = sc_core::sc_time(period, base);
}

#ifdef DEBUG
#define SHOWCGPCounter(n) \
    CGPCounter[n]->value_read(); \
    std::cout << " CGPTimer"#n << ":{ v:" << r[TIM_VALUE(n)] << ", r:" << r[TIM_RELOAD(n)] << "}";

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

/// @}

#endif
