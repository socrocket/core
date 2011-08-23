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
// Title:      gpCGPCounter.cpp
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
#include "gpcounter.h"
#include "gptimer.h"
#include "verbose.h"

/// @addtogroup gptimer
/// @{

CGPCounter::CGPCounter(CGPTimer &_parent, unsigned int _nr,
                       sc_core::sc_module_name name) :
    gr_subdevice(name, _parent), p(_parent), nr(_nr), stopped(true), chain_run(false) {
    SC_THREAD(ticking);

    PM::registerIP(this, "gpcounter",1);
    PM::send_idle(this,"idle",sc_time_stamp(),1);
}

CGPCounter::~CGPCounter() {
    GC_UNREGISTER_CALLBACKS();
}

void CGPCounter::end_of_elaboration() {
    GR_FUNCTION(CGPCounter, ctrl_read);
    GR_SENSITIVE(p.r[CGPTimer::CTRL(nr)].add_rule(gs::reg::PRE_READ,
            gen_unique_name("ctrl_read", false), gs::reg::NOTIFY));

    GR_FUNCTION(CGPCounter, ctrl_write);
    GR_SENSITIVE(p.r[CGPTimer::CTRL(nr)].add_rule(gs::reg::POST_WRITE,
            gen_unique_name("ctrl_write", false), gs::reg::NOTIFY));

    GR_FUNCTION(CGPCounter, value_read);
    GR_SENSITIVE(p.r[CGPTimer::VALUE(nr)].add_rule(gs::reg::PRE_READ,
            gen_unique_name("value_read", false), gs::reg::NOTIFY));

    GR_FUNCTION(CGPCounter, value_write);
    GR_SENSITIVE(p.r[CGPTimer::VALUE(nr)].add_rule(gs::reg::POST_WRITE,
            gen_unique_name("value_write", false), gs::reg::NOTIFY));

    p.r[CGPTimer::VALUE(nr)].enable_events();
}

void CGPCounter::ctrl_read() {
    p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_DH] = (p.dhalt.read() != 0);
    p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_IP] = m_pirq;
}

void CGPCounter::ctrl_write() {
    p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_DH] = (p.dhalt.read() != 0);

    /* Clean irq if desired */
    bool old_pirq = m_pirq;

    m_pirq = p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_IP];
    /* // Unset IRQ
    if (old_pirq && !m_pirq) {
        unsigned int irqnr = (p.r[CGPTimer::CONF] >> 3) & 0xF;
        if (p.r[CGPTimer::CONF].b[CGPTimer::CONF_SI]) {
            irqnr += nr;
        }
        p.irq.write(p.irq.read() & ~(1 << irqnr));
    }
    */
    /* Prepare for chainging */
    if (p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH]) {
        chain_run = false;
        stop();
    }

    /* Load */
    if (p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_LD]) {
        unsigned int reload = p.r[CGPTimer::RELOAD(nr)];
        p.r[CGPTimer::VALUE(nr)].set(reload);
        value_write();
        p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_LD] = false;
    }

    /* Enable */
    //v::debug << name() << "StartStop_" << nr << v::endl;
    if (p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_EN] && stopped) {
        v::debug << name() << "Start_" << nr << v::endl;
        start();
    } else if ((!p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_EN]) && !stopped) {
        v::debug << name() << "Stop_" << nr << v::endl;
        stop();
    }
}

void CGPCounter::value_read() {
    if (!stopped) {
        sc_core::sc_time now = sc_core::sc_time_stamp();
        int reload = p.r[CGPTimer::RELOAD(nr)] + 1;
        int dticks;
        if (p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH]) {
            dticks = p.numberofticksbetween(lasttime, now, 0,
                    p.counter[nr - 1]->cycletime());
        } else {
            dticks
                    = p.numberofticksbetween(lasttime, now, nr + 2,
                            p.clockcycle);
        }
        int value = ((int)lastvalue - dticks) % reload;

        if (value < 0) {
            p.r[CGPTimer::VALUE(nr)] = reload + value;
        } else {
            p.r[CGPTimer::VALUE(nr)] = value;
        }
    }
}

void CGPCounter::value_write() {
    lastvalue = p.r[CGPTimer::VALUE(nr)];
    lasttime = sc_core::sc_time_stamp();
    if (!stopped) {
        calculate();
    }
}

void CGPCounter::chaining() {
    chain_run = true;
    //v::debug << name() << "Chaining_" << nr << v::endl;
    start();
}

void CGPCounter::ticking() {
    unsigned int irqnr = 0;
    while (1) {
        // calculate sleep time, CGPCounter timeout
        calculate();

        v::debug << name() << "CGPCounter_" << nr << " is wait" << v::endl;
        wait(e_wait);
        v::debug << name() << "CGPCounter_" << nr << " is rockin'" << v::endl;
        PM::send(this, "underflow", 1, sc_time_stamp(),0,1);
        // Send interupt and set outputs
        if (p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_IE]) {
            // p.r[CGPTimer::CTRL].b[CGPTimer::TIM_CTRL_SI] // seperatet interupts
            // APBIRQ addresse beachten -.-
            irqnr = (p.r[CGPTimer::CONF] >> 3) & 0xF;
            v::debug << name() << "IRQ: " << irqnr << " CONF "<< hex << (uint32_t)p.r[CGPTimer::CONF] << v::endl;
            if (p.r[CGPTimer::CONF].b[CGPTimer::CONF_SI]) {
                irqnr += nr;
            }
            p.irq.write((1 << irqnr), true);
            m_pirq = true;
        }
        if(p.counter.size()-1==nr && p.wdog_length!=0) {
           p.wdog.write(true);
        }
#ifdef DEBUGOUT
        if(p.rst) {
            p.tick.write(p.tick.read() | (1<<(nr + 1)));
        }
#endif
        wait(p.clockcycle);
        PM::send(this, "underflow", 0, sc_time_stamp(),0,1);
        if(m_pirq&&irqnr) {
            p.irq.write(1 << irqnr, false);
        }
        if(p.counter.size()-1==nr && p.wdog_length!=0) {
           p.wdog.write(false);
        }
#ifdef DEBUGOUT
        if(p.rst) {
            p.tick.write(p.tick.read() & ~(1<<(nr + 1)));
        }
#endif

        unsigned int nrn = (nr + 1 < p.counter.size())? nr + 1 : 0;
        if (p.r[CGPTimer::CTRL(nrn)].b[CGPTimer::CTRL_CH]) {
            p.counter[(nrn) % p.counter.size()]->chaining();
        }

        // Enable value becomes restart value
        p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_EN].set(
                p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_RS].get());
    }
}

void CGPCounter::do_reset() {
    lastvalue = 0;
    lasttime = sc_core::sc_time_stamp();
    p.r[CGPTimer::VALUE(nr)] = 0;
    p.r[CGPTimer::RELOAD(nr)] = 0;
    p.r[CGPTimer::CTRL(nr)] = 0;
    stopped = true;
    chain_run = false;
}

// Gets the time to the end of the next zero hit.
sc_core::sc_time CGPCounter::nextzero() {
    sc_core::sc_time t; // cycle time of foundation (other CGPCounter or the cloccycle)
    int x; // Per cycle
    if (p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH]) { // We depend on the cycle time of the last CGPCounter.
        if (nr) {
            //p.counter[nr-1]->value_read();
            //t = p.counter[nr-1]->cycletime();
            x = p.r[CGPTimer::VALUE(nr - 1)];
        } else {
            p.counter[p.counter.size() - 1]->value_read();
            t = p.counter[p.counter.size() - 1]->cycletime();
            x = p.r[CGPTimer::VALUE(p.counter.size() - 1)];
        }
    } else { // We only depend on the prescaler
        p.scaler_read();
        t = p.clockcycle;
        x = p.r[CGPTimer::SCALER];
    }
    return t * (x + 2);
}


// Gets the Cycletime of the CCGPCounter.
sc_core::sc_time CGPCounter::cycletime() {
    sc_core::sc_time t; // cycle time of foundation (other CGPCounter or the cloccycle)
    int m; // Per cycle
    if (p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH]) { // We depend on the cycle time of the last CGPCounter.
        if (nr) {
            t = p.counter[nr - 1]->cycletime();
            m = p.r[CGPTimer::RELOAD(nr - 1)];
        } else {
            t = p.counter[p.counter.size()]->cycletime();
            m = p.r[CGPTimer::RELOAD(p.counter.size())];
        }
    } else { // We only depend on the prescaler
        t = p.clockcycle;
        m = p.r[CGPTimer::SCRELOAD];
        //m = 1;
    }
    return t * (m + 1);
    //return t * (m + 1);
}

// Recalculate sleeptime and send notification
void CGPCounter::calculate() {
    e_wait.cancel();
    value_read();
    int value = p.r[CGPTimer::VALUE(nr)];
    sc_core::sc_time time;
    // Calculate with currentime, and lastvalue updates
    if (p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_EN]) {
        sc_core::sc_time zero = this->nextzero();
        sc_core::sc_time cycle = this->cycletime();
        v::debug << name() << " calc_" << nr << " Zero: " << zero << " Cycle: " << cycle << " Value: " << value << v::endl;
        time = zero;
        time += (cycle * value) + (nr + 1) * p.clockcycle;
        v::debug << name() << " calc_" << nr << ": " << time << v::endl;
        e_wait.notify(time);
    }
}

// Start counting imideately.
// For example for enable, !dhalt, e_chain
void CGPCounter::start() {
    v::debug << name() << "start_" << nr << " stopped: " << stopped << "-"
            << (bool)(p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_EN]) << "-"
            << (p.dhalt.read() != 0) << "-"
            << (!p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH]
                    || (p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH]
                            && chain_run)) << v::endl;
    if (stopped && p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_EN]
            /*&& (p.dhalt.read()!=0)*/&& (!p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH]
                    || (p.r[CGPTimer::CTRL(nr)].b[CGPTimer::CTRL_CH]
                            && chain_run))) {
        v::debug << name() << "startnow_" << nr << v::endl;

        lasttime = sc_core::sc_time_stamp();
        calculate();
        stopped = false;
        PM::send(this, "active", 1, sc_time_stamp(),0,1);
    }
}

// Stoping counting imideately.
// For example for disableing, dhalt, e_chain
//
void CGPCounter::stop() {
    if (!stopped) {
        //v::debug << name() << "stop_" << nr << v::endl;
        e_wait.cancel();
        value_read();
        lastvalue = p.r[CGPTimer::VALUE(nr)];
        lasttime = sc_core::sc_time_stamp();
        stopped = true;
        PM::send(this, "active", 0, sc_time_stamp(),0,1);
    }
}

/// @}
