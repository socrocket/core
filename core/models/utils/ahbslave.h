// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file ahbslave.h
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///
#ifndef MODELS_UTILS_AHBSLAVE_H_
#define MODELS_UTILS_AHBSLAVE_H_

#include "common/systemc.h"
#include <tlm.h>
#include "common/gs_config.h"
#include <amba.h>
#include <stdint.h>

#include "models/utils/ahbdevice.h"
#include "common/msclogger.h"
#include "common/verbose.h"

template<class BASE = DefaultBase>
class AHBSlave : public AHBDevice<BASE> {
  public:
    SC_HAS_PROCESS(AHBSlave);

    using BASE::name;

    AHBSlave(
      ModuleName mn,
      uint8_t hindex,
      uint8_t vendor,
      uint8_t device,
      uint8_t version,
      uint8_t irq,
      AbstractionLayer ambaLayer,
      BAR bar0 = BAR(),
      BAR bar1 = BAR(),
      BAR bar2 = BAR(),
      BAR bar3 = BAR(),
      uint32_t register_count = 0);

    ~AHBSlave();

    /// AHB Slave Socket
    ///
    /// Receives instructions (mem access) from CPU
    ::amba::amba_slave_socket<32> ahb;

    /// Interface to functional part of the model
    virtual uint32_t exec_func(
        tlm::tlm_generic_payload &gp,  // NOLINT(runtime/references)
        sc_time &delay,                // NOLINT(runtime/references)
        bool debug = false) = 0;

    /// Returns clock cycle time from child
    virtual sc_core::sc_time get_clock() = 0;

    /// TLM blocking transport functions
    virtual void b_transport(tlm::tlm_generic_payload &gp, sc_time &delay);  // NOLINT(runtime/references)

    /// TLM debug transport function
    virtual uint32_t transport_dbg(tlm::tlm_generic_payload &gp);  // NOLINT(runtime/references)

    /// TLM non-blocking transport function
    virtual tlm::tlm_sync_enum nb_transport_fw(
        tlm::tlm_generic_payload &trans,  // NOLINT(runtime/references)
        tlm::tlm_phase &phase,            // NOLINT(runtime/references)
        sc_core::sc_time &delay);         // NOLINT(runtime/references)

    /// Accept new transaction (busy or not)
    void requestThread();

    /// Thread for interfacing functional part of the model in AT mode
    void responseThread();

    /// Collect common transport statistics.
    virtual void transport_statistics(tlm::tlm_generic_payload &gp) throw();  // NOLINT(runtime/references)

    /// Print common transport statistics.
    virtual void print_transport_statistics(const char *name) const throw();

    /// Event queues for AT mode
    tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_RequestPEQ;
    tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_ResponsePEQ;

  private:
    /// Ready to accept new transaction (send END_REQ)
    sc_event unlock_event;

    bool busy;
    // sc_core::sc_time clockcycle;

  protected:
    /// Stores the number of Bytes read from the device
    gs::gs_config<uint64_t> m_reads;  // NOLINT(runtime/int)

    /// Stores the number of Bytes written from the device
    gs::gs_config<uint64_t> m_writes;  // NOLINT(runtime/int)
};

#include "ahbslave.tpp"

#endif  // MODELS_UTILS_AHBSLAVE_H_
/// @}
