// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup irqmp
/// @{
/// @file irqmp.h
/// Class definition of multi-processor interrupt controller. (IRQMP).
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef MODELS_IRQMP_IRQMP_H_
#define MODELS_IRQMP_IRQMP_H_

#include <stdint.h>
#include "core/common/systemc.h"
#include "core/common/sr_param.h"
#include <boost/config.hpp>
#include <utility>

#include "core/common/apbslave.h"
#include "core/common/clkdevice.h"
#include "core/common/sr_signal.h"

class Irqmp : public APBSlave, public CLKDevice {
  public:
    SC_HAS_PROCESS(Irqmp);
    SR_HAS_SIGNALS(Irqmp);
    GC_HAS_CALLBACKS();

    /// CPU reset out signals
    signal<bool>::selector cpu_rst;

    /// CPU reset out signals
    signal<bool>::infield cpu_stat;

    /// IRQ Request out signals
    signal<std::pair<uint32_t, bool> >::selector irq_req;

    /// IRQ Acknowledge input signals
    signal<uint32_t>::infield irq_ack;

    /// IRQ input signals from other devices
    signal<std::pair<uint32_t,bool> >::in irq_in;

    /// Internal signal to decouple inputs and outputs. Furthermore it addes the needed delays
    sc_event e_signal;

    /// Constructor
    ///
    ///  The constructor is taking the VHDL generics as parameters.
    ///
    /// @param name SystemC instance name.
    /// @param paddr Upper 12bit of the APB address.
    /// @param pmask Upper 12bit of the APB mask.
    /// @param ncpu  Number of CPU which receive interupts.
    /// @param eirq  Interrupt channel which hides all the extended interrupt channels.
    Irqmp(sc_module_name name,
    int32_t paddr = 0,
    int32_t pmask = 0xFFF,
    int32_t ncpu = 2,
    int32_t eirq = 1,
    uint32_t pindex = 0,
    bool powmon = false);

    /// Default destructor
    ///
    /// Frees all dynamic object members.
    ~Irqmp();

    // function prototypes
    /// Initialize the generics with meta data.
    ///
    /// Will ne called from the constructor.
    void init_generics();
    void init_registers();

    /// Automatically called at start of simulation
    void start_of_simulation();

    /// SystemC end of elaboration implementation
    void end_of_elaboration();

    /// SystemC end of simulation
    void end_of_simulation();

    /// Calculate power/energy values from normalized input data
    void power_model();

    /// Static power callback
    gs::cnf::callback_return_type sta_power_cb(
        gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
        gs::cnf::callback_type reason);

    /// Dynamic/Internal power callback
    gs::cnf::callback_return_type int_power_cb(
        gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
        gs::cnf::callback_type reason);

    /// Recalculates the output for the CPUs.
    ///
    ///  This function is called whenever an interrupt is triggered.
    ///  It will change the output state.
    void launch_irq();

    // Bus Register Callbacks

    /// Write to IR clear register
    ///
    ///  Triggers the cleaning of an interupt bit in the IR clear register.
    ///  Will force a recalculation of a new interrupt.
    void clear_write();

    /// Write to the IR force register
    ///
    ///  If a write is done to the interrupt force register this function will recalculate
    ///  the interrupt force settings and trigger a recalculation of the outputs.
    void force_write();

    /// Prepare reading of the MP status register
    void mpstat_read();

    /// Write to MP status register
    void mpstat_write();

    /// Write to pending register
    ///
    ///  Addes new interrupts to the current pending register.
    ///  It will trigger the recalculation of the outputs.
    void pending_write();

    // Signal Callbacks
    /// Reset Callback
    ///
    ///  This function is called when the reset signal is triggert.
    ///  The reset whill reset all registers and bring the IRQ controler in a valid state.
    ///
    /// @param value Value of the reset signal the reset is active as long the signal is false.
    ///              Therefore the reset is done on the transition from false to true.
    /// @param time  Delay to the current simulation time. Is not used in this callback.
    void dorst();

    /// Incomming interrupts
    ///
    ///  This Callback is registert to the interrupt input signal.
    ///  It will set the corresponding register bits and trigger a recalculation of the outputs.
    ///
    /// @param value The value of the Interrupt.
    ///              A value of false will be ignored due to the fact that interrupts can only
    ///              be cleared in the interrupt controler.
    /// @param irq   The interrupt line which is triggered.
    /// @param time  Delay to the simulation time. Not used in this signal.
    void incomming_irq(const std::pair<uint32_t, bool> &value, const sc_time &time);

    /// Acknowledged Irq Callback
    ///
    ///  This Callback is called if the direct acknowledge way is used.
    ///  It will clean the coressponding bit out of the pending and force registers.
    ///
    /// @param irq  The Interrupt to clean.
    /// @param cpu  The CPU which acknowleged the Interrupt
    /// @param time Delay to the simulation time. Not used with this signal.
    void acknowledged_irq(const uint32_t &irq, const uint32_t &cpu, const sc_time &time);
    /// Number of CPUs in the System
    /// Needet to determ the number of receiver lines.
    sr_param<int> g_ncpu;

    /// Extended Interrupt Number
    /// Behind this interrupt are all extended interrupt cascaded.
    sr_param<uint32_t> g_eirq;

  private:

    /// Status of the force registers
    /// To determ the change in the status force fields.
    uint32_t *forcereg;

    /// Performance Counter per IRQ Line
    /// The number of executed interrupts is stored in the variable
    gs::gs_param<unsigned long long *> m_irq_counter;  // NOLINT(runtime/int)

    /// Performance Counter per CPU Line
    /// The number of executed interrupts on each cpu line is stored in the variable
    gs::gs_param<unsigned long long *> m_cpu_counter;  // NOLINT(runtime/int)

    /// power monitoring enabled or not
    const uint32_t m_pow_mon;

  public:
    // *****************************************************
    // Power Modeling Parameters

    /// Normalized static power of controller
    sr_param<double> sta_power_norm;

    /// Normalized dynamic power of controller
    sr_param<double> int_power_norm;

    /// Controller static power
    sr_param<double> sta_power;

    /// Controller dynamic power
    sr_param<double> int_power;

    // ******************************************************
    // Constant and mask definitions

    // ---register address offset
    static const uint32_t IR_LEVEL           = 0x00;
    static const uint32_t IR_PENDING         = 0x04;
    static const uint32_t IR_FORCE           = 0x08;
    static const uint32_t IR_CLEAR           = 0x0C;
    static const uint32_t MP_STAT            = 0x10;
    static const uint32_t BROADCAST          = 0x14;

    inline static const uint32_t PROC_IR_MASK(int CPU_INDEX) {
      return 0x40 + 0x4 * CPU_INDEX;
    }

    inline static const uint32_t PROC_IR_FORCE(int CPU_INDEX) {
      return 0x80 + 0x4 * CPU_INDEX;
    }

    inline static const uint32_t PROC_EXTIR_ID(int CPU_INDEX) {
      return 0xC0 + 0x4 * CPU_INDEX;
    }

    /// register contents (config bit masks)
    /// interrupt level register
    /// interrupt priority level (0 or 1)
    static const uint32_t IR_LEVEL_IL        = 0x0000FFFE;

    /// interrupt pending register
    /// extended interrupt pending (true or false)
    static const uint32_t IR_PENDING_EIP     = 0xFFFF0000;

    /// interrupt pending (true or false)
    static const uint32_t IR_PENDING_IP      = 0x0000FFFE;

    /// interrupt force register
    /// force interrupt (true or false)
    static const uint32_t IR_FORCE_IF        = 0x0000FFFE;

    /// interrupt clear register
    /// n=1 to clear interrupt n
    static const uint32_t IR_CLEAR_IC        = 0xFFFFFFFE;

    /// multiprocessor status register
    /// multiprocessor status register write mask
    static const uint32_t MP_STAT_WMASK      = 0x0000FFFF;
    /// number of CPUs in the system
    static const uint32_t MP_STAT_NCPU       = 0xF0000000;

    /// interrupt number used for extended interrupts
    static const uint32_t MP_STAT_EIRQ       = 0x000F0000;

    /// broadcast register (applicable if NCPU>1)
    /// broadcast mask: if n=1, interrupt n is broadcasted
    static const uint32_t BROADCAST_BM       = 0x0000FFFE;

    /// processor mask register
    /// interrupt mask for extended interrupts
    static const uint32_t PROC_MASK_EIM      = 0xFFFF0000;

    /// interrupt mask (0 = masked)
    static const uint32_t PROC_MASK_IM       = 0x0000FFFE;

    /// processor interrupt force register
    /// interrupt force clear
    static const uint32_t PROC_IR_FORCE_IFC  = 0xFFFE0000;

    /// interrupt force
    static const uint32_t PROC_IR_FORCE_IF   = 0x0000FFFE;

    /// extended interrupt identification register
    /// ID of the acknowledged extended interrupt (16..31)
    static const uint32_t PROC_EXTIR_ID_EID  = 0x0000001F;

    /// register default values
    /// interrupt level register
    static const uint32_t LEVEL_DEFAULT      = 0x00000000;

    /// interrupt pending register
    static const uint32_t PENDING_DEFAULT    = 0x00000000;

    /// interrupt force register
    static const uint32_t FORCE_DEFAULT      = 0x00000000;

    /// interrupt clear register
    static const uint32_t CLEAR_DEFAULT      = 0x00000000;

    /// multiprocessor status register
    static const uint32_t MP_STAT_DEFAULT    = 0x00000001;

    /// broadcast register
    static const uint32_t BROADCAST_DEFAULT  = 0x00000000;

    /// interrupt mask register
    static const uint32_t MASK_DEFAULT       = 0xFFFFFFFE;

    /// processor interrupt force register
    static const uint32_t PROC_FORCE_DEFAULT = 0x00000000;

    /// extended interrupt identification register
    static const uint32_t EXTIR_ID_DEFAULT   = 0x00000000;
};

#endif  // MODELS_IRQMP_IRQMP_H_
/// @}
