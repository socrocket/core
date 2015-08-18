// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup gptimer GPTimer
/// @{
/// @file gptimer.h
/// Class definition of the General-Purpose Timer (GP_TIMER)
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef MODELS_GPTIMER_GPTIMER_H_
#define MODELS_GPTIMER_GPTIMER_H_

#include "core/common/base.h"
#include "core/common/systemc.h"
#include <boost/config.hpp>

#include <string>
#include <vector>

#include "gaisler/gptimer/gpcounter.h"
#include "core/common/apbslave.h"
#include "core/common/clkdevice.h"
#include "core/common/sr_signal.h"

#include "core/common/verbose.h"
#include "core/common/sr_param.h"

#undef CTRL

/// @brief This class is a TLM 2.0 Model of the Aeroflex Gaisler GRLIB GPTimer.
/// Further informations to the original VHDL Modle are available in the GRLIB IP Core User's Manual Section 37
class GPTimer : public APBSlave, public CLKDevice {
 public:
  SC_HAS_PROCESS(GPTimer);
  SR_HAS_SIGNALS(GPTimer);
  GC_HAS_CALLBACKS();

  signal<std::pair<uint32_t, bool> >::out irq;
  signal<bool>::out wdog;

  /// Stores the default config register value
  unsigned int conf_defaults;

  /// Stores the time when the value of the prescaler was known.
  sc_core::sc_time lasttime;

  /// Stores the last known prescaler value.
  ///
  /// A prescaler value is known when it gets set by the prescaler value register
  /// or when its get calculatet befor a complete stop.
  ///
  /// It gets always set with the lasttime. Both define the prescaler function
  /// implemented in valueof.
  uint64_t lastvalue;

  /// The tick event.
  ///
  /// This event gets set to calculate and produce the tick value for the prescaler underflow.
  /// The value gets calculated either when the prescaler reset or value register change
  /// or when the undeflow just happend.
  /// See tick_calc for calculation and ticking for the event wait statement.
  ///
  sc_core::sc_event e_tick;

  /// A vector of Counter classes, each representate an internal counter.
  // TODO(rmeyer) Replace by Array instanciated at construction time.
  std::vector<GPCounter *> counter;


  /// Creates an instance of an GPTimer.
  ///
  /// @param name      The name of the instance. It's needed for debunging.
  /// @param ncounters Defines the number of counters in the unit. Default is 1. Max is 7.
  /// @param pindex   APB bus slave index
  /// @param paddr    APB bus slave address
  /// @param pmask    APB bus slave mask
  /// @param pirq     Defines which APB interupt the timers will generate. Default is 0.
  /// @param sepirq   If set to 1, each timer will drive an individual interrupt line,
  ///                  starting with interrupt irq. If set to 0, all timers will drive
  ///                  the same interrupt line (irq).
  /// @param nbits    Defines the number of bits in the timers. Default is 32.
  /// @param sbits    Defines the number of bits in the scaler. Default is 16.
  /// @param wdog     Watchdog reset value. When set to a non-zero value, the
  ///                  last timer will be enabled and pre-loaded with this value
  ///                  at reset. When the timer value reaches 0, the WDOG output
  ///                  is driven active.
  GPTimer(ModuleName name, unsigned int ncounters = 1,
          int pindex = 0, int paddr = 0, int pmask = 4095, int pirq = 0,
          int sepirq = 0, int sbits = 16, int nbits = 32, int wdog = 0,
          bool powmon = false);

  /// Free all counter and unregister all callbacks.
  ~GPTimer();

  /// Initialize the generics with meta data.
  ///
  /// Will ne called from the constructor.
  void init_generics();

  /// Initialize the register file.
  ///
  /// Will be called from the constructor.
  /// Also creates the GPCounter objects to minimize the number of loops in the constructor.
  void init_registers();

  /// SystemC start of simulation callback
  void start_of_simulation();

  /// Calculate power/energy values from normalized input data
  void power_model();

  /// Static power callback
  gs::cnf::callback_return_type sta_power_cb(
      gs::gs_param_base& changed_param,  // NOLINT(runtime/references)
      gs::cnf::callback_type reason);

  /// Dynamic/Internal power callback
  gs::cnf::callback_return_type int_power_cb(
      gs::gs_param_base& changed_param,  // NOLINT(runtime/references)
      gs::cnf::callback_type reason);

  /// Execute the callback registering when systemc reaches the end of elaboration.
  void end_of_elaboration();

  // Register Callbacks
  /// Register callback executed before the prescaler value register gets read, to calculate the current value.
  void scaler_read();

  /// Register callback executed after the presacler value is written, to recalculate the counter and ticking functions.
  void scaler_write();

  /// Register callback executed after the presacler reset is written, to recalculate the counter and ticking functions.
  void screload_write();

  /// Register callback executed before the prescaler config register gets read, to calculate the current value.
  void conf_read();

  // Signal Callbacks
  /// Signal callback executed a reset on the timer when the rst signal arrives.
  ///
  /// @param value The new Value of the Signal.
  /// @param time A possible delay. Which means the reset might be performed in the future (Not used for resets!).
  virtual void dorst();

  // Functions
  /// The time to value function of the prescaler or the counters.
  ///
  ///  This is the fundamental function which defines the connection between a given time and the value.
  ///  All calculations in the Timer and Counter class are based on this function.
  ///  The function itself is defined through the two attributes lastvalue and lasttime.
  ///
  /// @param t         The Time for which is needed the prescaler value.
  /// @param offset    An offset, defined to calculate a value which is influenced by the counter number.
  ///                  This is needed due to the fact that the rtl model decrements the counters one after one,
  ///                  Each in a clockcycle if the prescaler undeflows.
  /// @param cycletime The cycle length which decrements the value.
  /// @return          The value to time t.
  ///
  /// @see lastvalue
  /// @see lasttime
  /// @see numberofticksbetween()
  ///
  int64_t valueof(sc_core::sc_time t, int64_t offset, sc_core::sc_time cycletime) const;

  /// Calculates the number of ticks (prescaler underflows) in between two certain time points.
  ///
  /// The cycletime can be set to enable the simple calculation of chained counters.
  /// Usualy it is clock_cycle but in case of a chained counter it is the cycletime of counter[n-1].
  /// @param start Start time as sc_core::sc_time
  /// @param end End time as sc_core::sc_time
  /// @param counter Number of a gpcounter to respect the counter delay (round robin decrement).
  /// @param cycletime The length of an cycle in which the decremention of the prescalet takes place.
  int64_t numberofticksbetween(sc_core::sc_time start, sc_core::sc_time end, int counter, sc_core::sc_time cycletime);

  /// Number of Counter in the Timer.
  /// For compatibility to the GPTimer VHDL model this is still called "timers".
  /// But keep in mind that the ticking units of the SystemC GPTimer are called
  /// GPCounters/counters to decrease confusion.
  sr_param<uint32_t> g_ntimers;

  /// Number of scaler bits to use for the pescaler.
  /// This value can be up to 32bits and limits the maximum value of the prescaler.
  sr_param<uint32_t> g_sbits;

  /// Number of counter register bits.
  /// This value can be up to 32bits and limits the maximum value of all counters.
  sr_param<uint32_t> g_nbits;

  /// Length of the initial watchdog period.
  /// It will be set after reset and if the watchdog counter reaches zero fire an watchdog event to reset the hardware.
  /// If zero the watchdog ist disabled.
  sr_param<uint32_t> g_wdog_length;

  /// Enables the powermonitor capapbilities
  sr_param<uint32_t> powermon;

  /// Seperated IRQ lines.
  /// If you whant to have seperated IRQ lines for each counter set this generic to true.
  sr_param<bool> g_sepirq;
  

  // *****************************************************
  // Power Modeling Parameters

  /// Normalized static power input
  sr_param<double> sta_power_norm;

  /// Normalized internal power input (activation independent)
  sr_param<double> int_power_norm;

  /// Static power of module
  sr_param<double> sta_power;

  /// Internal power of module (activation independent)
  sr_param<double> int_power;


  // *****************************************************
  // Constant and mask definitions

  // Register Value Offsets, Masks and Bits
  /// Scaler Value Register Address
  static const uint32_t SCALER = 0x00;

  /// Scaler Relaod Register Address
  static const uint32_t SCRELOAD = 0x04;

  /// Scaler Configuration Register Address
  static const uint32_t CONF = 0x08;

  /// Returns Counter Value Register Address for Counter nr
  ///
  /// @param nr Counter Number
  static const uint32_t VALUE(uint8_t nr) {
    return 0x10 * (nr + 1) + 0x0;
  }

  /// Returns Counter Reload Register Address for Counter nr
  ///
  /// @param nr Counter Number
  static const uint32_t RELOAD(uint8_t nr) {
    return 0x10 * (nr + 1) + 0x4;
  }

  /// Returns Counte Control Register Address for Counter nr
  ///
  /// @param nr Counter Number
  static const uint32_t CTRL(uint8_t nr) {
    return 0x10 * (nr + 1) + 0x8;
  }

  /// Position of the Bit in the Scaler Configuration Register
  static const uint32_t CONF_DF = 9;

  /// Position of the SetInterrupt Bit in the Scaler Configuration Register
  static const uint32_t CONF_SI = 8;

  /// Mask of the IRQ Bits in the Scaler Configuration Register
  static const uint32_t CONF_IQ_MA = 0x000000F8;

  /// Offset of the IRQ Bits in the Scaler Configuration Register
  static const uint32_t CONF_IQ_OS = 3;

  /// Mask of the Counter Number Bits in the Scaler Configuration Register
  static const uint32_t CONF_NR_MA = 0x00000007;

  /// Offset of the Counter Number Bits in the Scaler Configuration Register
  static const uint32_t CONF_NR_OS = 0;

  /// Position of the Debug Halt Bit in the Counter Control Registers
  static const uint32_t CTRL_DH = 6;

  /// Position of the Chaining Bit in the Counter Control Registers
  static const uint32_t CTRL_CH = 5;

  /// Position of the Interrupt Pending Bit in the Counter Control Registers
  static const uint32_t CTRL_IP = 4;

  /// Position of the Interrupt Enable Bit in the Counter Control Registers
  static const uint32_t CTRL_IE = 3;

  /// Position of the Load Bit in the Counter Control Registers
  static const uint32_t CTRL_LD = 2;

  /// Position of the Reset Bit in the Counter Control Registers
  static const uint32_t CTRL_RS = 1;

  /// Position of the Enable Bit in the Counter Control Registers
  static const uint32_t CTRL_EN = 0;

  friend class GPCounter;
};

#endif // MODELS_GPTIMER_GPTIMER_H_
/// @}
