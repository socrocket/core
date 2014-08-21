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
#include "models/gptimer/gptimer.h"
#include "common/report.h"

// Constructor: create all members, registers and Counter objects.
// Store configuration default value in conf_defaults.
GPTimer::GPTimer(ModuleName name, unsigned int ntimers,
                   int pindex, int paddr, int pmask, int pirq, int sepirq,
                   int sbits, int nbits, int wdog, bool powmon) :
    APBDevice<RegisterBase>(name, pindex, 0x1, 0x11, 1, pirq, APBIO, pmask, false, false, paddr, 4 * (1+ ntimers)),
    bus("bus", r, ((paddr) & (pmask)) << 8, (((~pmask & 0xfff) + 1) << 8), ::amba::amba_APB, ::amba::amba_LT, false),
    irq("IRQ"), wdog("WDOG"),
    conf_defaults((sepirq << 8) | ((pirq & 0xF) << 3) | (ntimers & 0x7)),
    lasttime(0, sc_core::SC_NS), lastvalue(0),
    g_ntimers("ntimers", ntimers, m_generics),
    g_sbits("sbit", sbits, m_generics),
    g_nbits("nbits", nbits, m_generics),
    g_wdog_length("wdog", wdog, m_generics),
    powermon("powermon", powmon, m_generics),
    g_pindex("index", pindex, m_generics),
    g_paddr("addr", paddr, m_generics),
    g_pmask("mask", pmask, m_generics),
    g_sepirq("sepirq", sepirq, m_generics),
    sta_power_norm("sta_power_norm", 2.46e+6, m_power),   // Normalized static power input
    int_power_norm("int_power_norm", 1.093e-8, m_power),  // Normalized internal power input
    sta_power("sta_power", 0.0, m_power),   // Static power output
    int_power("int_power", 0.0, m_power) {  // Internal dynamic power output (activation independent)
  // Parameter checking
  assert("gsbits has to be between 1 and 32" && sbits > 0 && sbits < 33);
  assert("nbits has to be between 1 and 32" && nbits > 0 && nbits < 33);
  assert("ntimers has to be between 1 and 7" && ntimers > 0 && ntimers < 8);
  assert("wdog has to be between 0 and 2^nbits-1" && wdog >= 0 && wdog < (1LL << nbits));

  init_generics();
  init_registers();

  // Display APB slave information
  srInfo("/configuration/gptimer/apbslave")
     ("addr", (uint64_t)bus.get_base_addr())
     ("size", (uint64_t)bus.get_size())
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
  srInfo("/configuration/gptimer/generics")
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
  // v::info << this->name() << " ************************************************************************** " << v::endl;
  // v::info << this->name() << " * Created gptimer with following parameters: " << v::endl;
  // v::info << this->name() << " * ------------------------------------------ " << v::endl;
  // v::info << this->name() << " * ntimers: " << ntimers << v::endl;
  // v::info << this->name() << " * pindex: " << pindex << v::endl;
  // v::info << this->name() << " * paddr/pmask: " << hex << paddr << "/" << pmask << v::endl;
  // v::info << this->name() << " * pirq: " << pirq << v::endl;
  // v::info << this->name() << " * sepirq: " << sepirq << v::endl;
  // v::info << this->name() << " * sbits: " << sbits << v::endl;
  // v::info << this->name() << " * nbits: " << nbits << v::endl;
  // v::info << this->name() << " * wdog: " << wdog << v::endl;
  // v::info << this->name() << " * pow_mon: " << powmon << v::endl;
  // v::info << this->name() << " ************************************************************************** " << v::endl;
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
  g_paddr.add_properties()
    ("name", "APB Base Address")
    ("range", "0..4095")
    ("The 12bit MSB address at the APB bus");

  g_pmask.add_properties()
    ("name", "APB Base Mask")
    ("range", "0..4095")
    ("The 12bit APB address mask");

  g_pindex.add_properties()
    ("name", "Bus Index")
    ("range", "0..15")
    ("The slave index at the APB bus");

  g_sepirq.add_properties()
    ("name", "Separated IRQs")
    ("1 - each timer will drive an individual interrupt line, starting with interrupt irq. \n"
     "0 - all timers will drive the same interrupt line (irq).");

  g_ntimers.add_properties()
    ("name", "Number of Counters")
    ("range", "1..7")
    ("Defines the number of timers in the unit.");

  g_sbits.add_properties()
    ("name", "Scaler Bits")
    ("range", "1..32")
    ("Defines the number of bits in the scaler");

  g_nbits.add_properties()
    ("name", "Counter Bits")
    ("range", "1..32")
    ("Defines the number of bits in the counters");
}

void GPTimer::init_registers() {
  // create register
  r.create_register("scaler", "Scaler Value",
    0x00,                                                 // offset
    gs::reg::STANDARD_REG | gs::reg::SINGLE_IO |          // config
    gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
    static_cast<unsigned int>((1ULL << g_sbits) - 1),     // init value
    static_cast<unsigned int>((1ULL << g_sbits) - 1),     // write mask
    // sbits defines the width of the value. Any unused most significant bits are reserved always read as 0s.
    32,                  // Register width. Maximum register with is 32bit sbit must be less than 32.
    0x00);               // Lock Mask: Not implementet has to be zero.

  r.create_register("screload", "Scaler Reload Value",
    0x04,                                                 // offset
    gs::reg::STANDARD_REG | gs::reg::SINGLE_IO |          // config
    gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
    static_cast<unsigned int> ((1ULL << g_sbits) - 1),    // init value
    static_cast<unsigned int> ((1ULL << g_sbits) - 1),    // write mask
    // sbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s.
    32,                                                   // register width
    0x00);                                                // lock mask

  r.create_register("conf", "Configuration Register",
    0x08,                                                 // offset
    gs::reg::STANDARD_REG | gs::reg::SINGLE_IO |          // config
    gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
    conf_defaults,                                        // init value
    0x00000200,                                           // write mask
    32,                                                   // register width
    0x00);                                                // lock mask

  for (unsigned int i = 0; i < g_ntimers; ++i) {
    GPCounter *c = new GPCounter(this, i, gen_unique_name("GPCounter", true));
    counter.push_back(c);
    r.create_register(gen_unique_name("value", false), "GPCounter Value Register",
      VALUE(i),                                           // offset
      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO |        // config
      gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      0x00000000,                                         // init value
      static_cast<unsigned int> ((1ULL << g_nbits) - 1),  // write mask
      // nbits defines the width of the value. Any unused most significant bits are reserved Always read as 0s.
      32,                                                 // register width
      0x00);                                              // lock mask

    r.create_register(gen_unique_name("reload", false), "Reload Value Register",
      RELOAD(i),                                          // offset
      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO |        // config
      gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      0x00000001,                                         // init value
      static_cast<unsigned int> ((1ULL << g_nbits) - 1),  // write mask
      // nbits defines the width of the reload. Any unused most significant bits are reserved Always read as 0s.
      32,                                                 // register width
      0x00);                                              // lock mask

    r.create_register(gen_unique_name("ctrl", false), "Controle Register",
      CTRL(i),                                            // offset
      gs::reg::STANDARD_REG | gs::reg::SINGLE_IO |        // config
      gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
      0x00000000,                                         // init value
      0x0000007F,                                         // write mask
      32,                                                 // register width
      0x00);                                              // lock mask
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
  GR_FUNCTION(GPTimer, scaler_read);
  GR_SENSITIVE(r[SCALER].add_rule(gs::reg::PRE_READ, "scaler_read", gs::reg::NOTIFY));

  GR_FUNCTION(GPTimer, scaler_write);
  GR_SENSITIVE(r[SCALER].add_rule(gs::reg::POST_WRITE, "scaler_write", gs::reg::NOTIFY));

  GR_FUNCTION(GPTimer, screload_write);
  GR_SENSITIVE(r[SCRELOAD].add_rule(gs::reg::POST_WRITE, "scaler_reload_write", gs::reg::NOTIFY));

  GR_FUNCTION(GPTimer, conf_read);
  GR_SENSITIVE(r[CONF].add_rule(gs::reg::PRE_READ, "conf_read", gs::reg::NOTIFY));
}

// Calback for scaler register. Updates register value before reads.
void GPTimer::scaler_read() {
    sc_core::sc_time now = sc_core::sc_time_stamp();
    int reload = r[SCRELOAD] + 1;
    int value = valueof(now, 0, clock_cycle) - (reload);

    if (reload) {
        value = value % (reload);
    }
    r[SCALER] = reload + value - 1;
    v::debug << name() << "Scaler: " << v::uint32 << value << " Reload: " << v::uint32 << reload << v::endl;
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
    v::debug << name() << "Scaler: " << lastvalue << v::endl;
    for (std::vector<GPCounter *>::iterator iter = counter.begin(); iter != counter.end(); iter++) {
        (*iter)->calculate();  // Recalculate
    }
}

// Callback for configuration register. Updates the content before reads.
void GPTimer::conf_read() {
  r[CONF] = (r[CONF] & 0x0000200) | (conf_defaults & 0x000000FF);
}

// Calback for the rst signal. Resets the module on true.
void GPTimer::dorst() {
    v::debug << name() << "Reseting" << v::endl;
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
int GPTimer::valueof(sc_core::sc_time t, int offset, sc_core::sc_time cycletime) const {
    return static_cast<int>(lastvalue - ((t - lasttime - (1 + offset) * cycletime) / cycletime) + 1);
}

// Number of zero crosses between two prescaler values.
int GPTimer::numberofticksbetween(sc_core::sc_time a, sc_core::sc_time b, int counter, sc_core::sc_time cycletime) {
    int reload = r[SCRELOAD] + 1;
    int val_a = valueof(a, 0, cycletime);
    int val_b = valueof(b, counter, cycletime);
    int num_a = val_a / reload + (val_a > 0 && val_b < 0);
    int num_b = val_b / reload;
    return std::abs(num_a - num_b);
}

/// @}
