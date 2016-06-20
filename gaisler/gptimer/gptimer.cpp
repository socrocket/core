// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup gptimer
/// @{
/// @file gptimer.cpp
/// source file defining the implementation of the gptimer model. Due to the
/// fact that the gptimer class is a template class it gets included by its
/// definition in gptimer.h
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#include <string>
#include <vector>
#include "gaisler/gptimer/gptimer.h"
#include "core/common/sr_report.h"
#include "core/common/sr_registry.h"

SR_HAS_MODULE(GPTimer);

// Constructor: create all members, registers and Counter objects.
// Store configuration default value in conf_defaults.
GPTimer::GPTimer(ModuleName name, unsigned int ntimers,
                   int pindex, int paddr, int pmask, int pirq, int sepirq,
                   int sbits, int nbits, int wdog, bool powmon) :
    APBSlave(name, pindex, 0x1, 0x11, 0, /* VER: SoCRocket default: 1, try to Mimic TSIM therefore 0 -- psiegl */
              pirq, APBIO, pmask, false, false, paddr),
    irq("IRQ"), wdog("WDOG"),
    conf_defaults((sepirq << 8) | ((pirq & 0xF) << 3) | (ntimers & 0x7)),
    lasttime(0, sc_core::SC_NS), lastvalue(0),
    g_ntimers("ntimers", ntimers, m_generics),
    g_sbits("sbits", sbits, m_generics),
    g_nbits("nbits", nbits, m_generics),
    g_wdog_length("wdog", wdog, m_generics),
    powermon("powermon", powmon, m_generics),
    g_sepirq("sepirq", sepirq, m_generics),
    sta_power_norm("sta_power_norm", 2.46e+6, m_power),   // Normalized static power input
    int_power_norm("int_power_norm", 1.093e-8, m_power),  // Normalized internal power input
    sta_power("sta_power", 0.0, m_power),   // Static power output
    int_power("int_power", 0.0, m_power) {  // Internal dynamic power output (activation independent)
  // Parameter checking
  GPTimer::init_generics();
  GPTimer::init_registers();
  assert("gsbits has to be between 1 and 32" && sbits > 0 && sbits < 33);
  assert("nbits has to be between 1 and 32" && nbits > 0 && nbits < 33);
  assert("ntimers has to be between 1 and 7" && ntimers > 0 && ntimers < 8);
  assert("wdog has to be between 0 and 2^nbits-1" && wdog >= 0 && wdog < (1LL << nbits));
  

  
  // Display APB slave information
  srInfo("/configuration/gptimer/apbslave")
     ("addr", (uint64_t)apb.get_base_addr())
     ("size", (uint64_t)apb.get_size())
     ("APB Slave Configuration");
  
  //v::info << this->name() << "APB slave @0x" << hex << v::setw(8)
  //        << v::setfill('0') << bus.get_base_addr() << " size: 0x" << hex
  //        << v::setw(8) << v::setfill('0') << bus.get_size() << " byte"
  //        << endl;


  // Register power callback functions
  if (powermon) {
    GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, GPTimer, sta_power_cb);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, GPTimer, int_power_cb);
  }
  
  // Configuration report
  srInfo()
    ("ntimers", ntimers)
    ("pindex", pindex)
    ("paddr", paddr)
    ("pmask", pmask)
    ("pirq", pirq)
    ("sepirq", sepirq)
    ("sbits", sbits)
    ("nbits", nbits)
    ("wdog", wdog)
    ("pow_mon", powmon)
    ("A GPTimer is created with these generics");
}

// Destructor: Unregister Register Callbacks.
// Destroy all Counter objects.
GPTimer::~GPTimer() {
  for (std::vector<GPCounter *>::iterator iter = counter.begin(); iter != counter.end(); iter++) {
    delete *iter;
  }
  GC_UNREGISTER_CALLBACKS();
}

void GPTimer::init_generics() {
  // set name, type, default, range, hint and description for gs_configs
  g_wdog_length.add_properties()
    ("name","Watchdog")
    ("vhdl_name", "wdog")
    ("Length of the initial watchdog period. If zero the watchdog is disabled.");
  g_sepirq.add_properties()
    ("name", "Separated IRQs")
    ("vhdl_name", "sepirq")
    ("1 - each timer will drive an individual interrupt line, starting with interrupt irq. \n"
     "0 - all timers will drive the same interrupt line (irq).");

  g_ntimers.add_properties()
    ("name", "Number of Counters")
    ("range", "1..7")
    ("vhdl_name", "ntimers")
    ("Defines the number of timers in the unit.");

  g_sbits.add_properties()
    ("name", "Scaler Bits")
    ("range", "1..32")
    ("vhdl_name", "sbits")
    ("Defines the number of bits in the scaler");

  g_nbits.add_properties()
    ("name", "Counter Bits")
    ("range", "1..32")
    ("vhdl_name", "nbits")
    ("Defines the number of bits in the counters");
  
}

void GPTimer::init_registers() {
  // create register
  r.create_register("scaler", "Scaler Value",
    0x00,                                                 // offset
    static_cast<unsigned int>((1ULL << g_sbits) - 1),     // init value
    static_cast<unsigned int>((1ULL << g_sbits) - 1))     // write mask
    // sbits defines the width of the value. Any unused most significant bits are reserved always read as 0s.
  .callback(SR_PRE_READ, this, &GPTimer::scaler_read)
  .callback(SR_POST_WRITE, this, &GPTimer::scaler_write);

  r.create_register("screload", "Scaler Reload Value",
    0x04,                                                 // offset
    static_cast<unsigned int> ((1ULL << g_sbits) - 1),    // init value
    static_cast<unsigned int> ((1ULL << g_sbits) - 1))    // write mask
    // sbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s.
  .callback(SR_POST_WRITE, this, &GPTimer::screload_write);

  r.create_register("conf", "Configuration Register",
    0x08,                                                 // offset
    conf_defaults,                                        // init value
    0x00000200)                                           // write mask
  .callback(SR_PRE_READ, this, &GPTimer::conf_read);

  for (unsigned int i = 0; i < g_ntimers; ++i) {
    GPCounter *c = new GPCounter(this, i, gen_unique_name("counter", true));
    counter.push_back(c);
    r.create_register(gen_unique_name("value", false), "GPCounter Value Register",
      VALUE(i),                                            // offset
      0x00000000,                                          // init value
      static_cast<unsigned int> ((1ULL << g_nbits) - 1))   // write mask
      // nbits defines the width of the value. Any unused most significant bits are reserved Always read as 0s.
      .callback(SR_PRE_READ, c, &GPCounter::value_read)
      .callback(SR_POST_WRITE, c, &GPCounter::value_write);

    r.create_register(gen_unique_name("reload", false), "Reload Value Register",
      RELOAD(i),                                           // offset
      0x00000001,                                          // init value
      static_cast<unsigned int> ((1ULL << g_nbits) - 1));  // write mask
      // nbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s.

    r.create_register(gen_unique_name("ctrl", false), "Controle Register",
      CTRL(i),                                            // offset
      0x00000000,                                         // init value
      0x0000007F)                                         // write mask
      .callback(SR_PRE_READ, c, &GPCounter::ctrl_read)
      .callback(SR_POST_WRITE, c, &GPCounter::ctrl_write);
  }
}
// Automatically called at start of simulation
void GPTimer::start_of_simulation() {
  // Initialize power model
  if (powermon) {
    power_model();
  }
}

// Calculate power/energy values from normalized input data
void GPTimer::power_model() {
  // Static power calculation (pW)
  sta_power = sta_power_norm * g_ntimers;

  // Cell internal power (uW)
  int_power = int_power_norm * g_ntimers * 1/(clock_cycle.to_seconds());
}

// Static power callback
gs::cnf::callback_return_type GPTimer::sta_power_cb(
    gs::gs_param_base& changed_param,  // NOLINT(runtime/references)
    gs::cnf::callback_type reason) {
  // Nothing to do !!
  // Static power of GPTimer is constant !!
  return GC_RETURN_OK;
}

// Internal power callback
gs::cnf::callback_return_type GPTimer::int_power_cb(
    gs::gs_param_base& changed_param,  // NOLINT(runtime/references)
    gs::cnf::callback_type reason) {
  // Nothing to do !!
  // Internal power of GPTimer is constant !!
  return GC_RETURN_OK;
}

// Set all register callbacks
void GPTimer::end_of_elaboration() {
}

// Calback for scaler register. Updates register value before reads.
void GPTimer::scaler_read() {
    sc_core::sc_time now = sc_core::sc_time_stamp();
    int64_t reload = r[SCRELOAD] + 1;
    int64_t value = valueof(now, 0, clock_cycle) - (reload);

    //if (reload) {
    value = value % (reload);
    //}
    //
    if (value < 0) {
      r[SCALER] = reload + value - 1;
    } else {
      r[SCALER] = reload - value;
    }
}

// Callback for scaler relaod register. Updates Prescaler Ticks and all Counters on write.
void GPTimer::screload_write() {
    uint32_t reload = r[SCRELOAD];
    r[SCALER] = reload;
    scaler_write();
}

// Callback for scaler value register. Updates Prescaler Ticks and all Counters on write.
void GPTimer::scaler_write() {
    lasttime = sc_core::sc_time_stamp();
    lastvalue = r[SCALER];
    for (std::vector<GPCounter *>::iterator iter = counter.begin(); iter != counter.end(); iter++) {
        (*iter)->calculate();  // Recalculate
    }
}

// Callback for configuration register. Updates the content before reads.
void GPTimer::conf_read() {
  r[CONF] = (r[CONF] & 0x0000200) | (conf_defaults & 0x000001FF);
}

// Calback for the rst signal. Resets the module on true.
void GPTimer::dorst() {
    r[SCALER] = static_cast<unsigned int> ((1ULL << g_sbits) - 1);
    r[SCRELOAD] = static_cast<unsigned int> ((1ULL << g_sbits) - 1);
    r[CONF] = conf_defaults;
    lastvalue = 0;
    wdog = 0;

    // TODO(rmeyer) Reset Irqs: irq = 0;
    lasttime = sc_core::sc_time_stamp();
    scaler_write();
    scaler_read();

    for (std::vector<GPCounter *>::iterator iter = counter.begin(); iter != counter.end(); iter++) {
        (*iter)->do_reset();
    }
    if (g_wdog_length) {
      size_t count = counter.size() -1;
      r[GPTimer::RELOAD(count)] = g_wdog_length;
      r[GPTimer::CTRL(count)] = 0xD;  // Enabled, Load, Irq
      counter[count]->ctrl_write();
    }
}

// Prescaler value relative to the current value.
int64_t GPTimer::valueof(sc_core::sc_time t, int64_t offset, sc_core::sc_time cycletime) const {
    return static_cast<int64_t>(lastvalue - int64_t(sc_core::sc_time(t - lasttime - (1 + offset) * cycletime) / cycletime) + 1);
}

// Number of zero crosses between two prescaler values.
int64_t GPTimer::numberofticksbetween(sc_core::sc_time a, sc_core::sc_time b, int counter, sc_core::sc_time cycletime) {
    int64_t reload = r[SCRELOAD] + 1;
    int64_t val_a = valueof(a, 0, cycletime);
    //int val_b = valueof(b, 0, cycletime);
    int64_t val_b = valueof(b, counter, cycletime);
    int64_t num_a = val_a / reload + (val_a > 0 && val_b < 0);
    int64_t num_b = val_b / reload;
    return std::abs(num_a - num_b);
}

/// @}
