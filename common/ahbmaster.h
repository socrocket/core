// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file ahbmaster.h
///
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef MODELS_UTILS_AHBMASTER_H_
#define MODELS_UTILS_AHBMASTER_H_

#include <stdint.h>
#include "core/common/sr_param.h"
#include "core/common/amba.h"
#include "core/common/systemc.h"
#include <tlm.h>

#include "core/common/ahbdevice.h"
#include "core/common/msclogger.h"
#include "core/common/verbose.h"

/// @details Almost all models implementing an AHB master interface (except busses) 
/// are derived from class AHBMaster. AHBMaster is a convenience class providing an 
/// AHB master socket and implementations of various access functions for reading/writing 
/// data over the bus. AHBMaster inherits AHBDevice and can be configured for loosely 
/// timed (LT) or approximately timed (AT) level of abstraction.
///
/// An overview about how to build own components based on AHBMaster is given in @ref modeling7 "Extending the Library".
/// 
template<class BASE = BaseModule<DefaultBase> >
class AHBMaster : public AHBDevice<BASE> {
  public:
    SC_HAS_PROCESS(AHBMaster);

    using BASE::name;

    /// Constructor
    AHBMaster(
      ModuleName nm,
      uint8_t hindex,
      uint8_t vendor,
      uint8_t device,
      uint8_t version,
      uint8_t irq,
      AbstractionLayer ambaLayer,
      BAR bar0 = BAR(),
      BAR bar1 = BAR(),
      BAR bar2 = BAR(),
      BAR bar3 = BAR());

    /// Destructor
    ~AHBMaster();

    /// The AHB master socketo
    amba::amba_master_socket<32> ahb;

    /// Read data from AHB (SC_ZERO_TIME version)
    virtual void ahbread(uint32_t addr, unsigned char *data, uint32_t length);

    /// Write data to AHB (SC_ZERO_TIME version)
    virtual void ahbwrite(uint32_t addr, unsigned char *data, uint32_t length);

    /// Read data from AHB
    virtual void ahbread(
        uint32_t addr,
        unsigned char *data,
        uint32_t length,
        sc_core::sc_time &delay,              // NOLINT(runtime/references)
        bool &cacheable,                      // NOLINT(runtime/references)
        tlm::tlm_response_status &response);  // NOLINT(runtime/references)

    /// Read data from AHB
    virtual void ahbread(
        uint32_t addr,
        unsigned char *data,
        uint32_t length,
        sc_core::sc_time &delay,              // NOLINT(runtime/references)
        bool &cacheable,                      // NOLINT(runtime/references)
        bool is_lock,
        tlm::tlm_response_status &response);  // NOLINT(runtime/references)

    /// Write data to AHB
    virtual void ahbwrite(
        uint32_t addr,
        unsigned char *data,
        uint32_t length,
        sc_core::sc_time &delay,              // NOLINT(runtime/references)
        tlm::tlm_response_status &response);  // NOLINT(runtime/references)

    /// Write data to AHB
    virtual void ahbwrite(
        uint32_t addr,
        unsigned char *data,
        uint32_t length,
        sc_core::sc_time &delay,              // NOLINT(runtime/references)
        bool is_lock,
        tlm::tlm_response_status &response);  // NOLINT(runtime/references)

    /// Generic AHB access function (blocking and non-blocking)
    virtual void ahbaccess(tlm::tlm_generic_payload * trans);

    /// Generic AHB debug access function 
    virtual uint32_t ahbaccess_dbg(tlm::tlm_generic_payload * trans);

    /// Debug read from AHB
    virtual uint32_t ahbread_dbg(uint32_t addr, unsigned char *data, uint32_t length);

    /// Debug write from AHB
    virtual uint32_t ahbwrite_dbg(uint32_t addr, unsigned char *data, uint32_t length);

    /// Function may be implemented by the child for payload checking (e.g. testbench master)
    virtual void response_callback(tlm::tlm_generic_payload *trans) {}

    /// TLM non-blocking backward transport function
    virtual tlm::tlm_sync_enum nb_transport_bw(
        tlm::tlm_generic_payload &payload,    // NOLINT(runtime/references)
        tlm::tlm_phase &phase,                // NOLINT(runtime/references)
        sc_core::sc_time &delay);             // NOLINT(runtime/references)

    /// Returns clock cycle time from master
    virtual sc_core::sc_time get_clock() = 0;

    /// Thread for response processing (read)
    void ResponseThread();

    /// Thread for processing data-phase of write transactions
    void DataThread();

    /// Collect common transport statistics.
    virtual void transport_statistics(tlm::tlm_generic_payload &gp) throw();  // NOLINT(runtime/references)

    /// Print common transport statistics.
    virtual void print_transport_statistics(const char *name) const throw();

  protected:
    /// PEQs for response synchronization
    tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_ResponsePEQ;

    /// Events for phase notifications
    sc_event m_EndRequestEvent;

    /// The abstraction layer of the instance (LT or AT)
    AbstractionLayer m_ambaLayer;

    /// Stores the number of Bytes read from the device
    sr_param<uint64_t> m_reads;  // NOLINT(runtime/int)

    /// Stores the number of Bytes written from the device
    sr_param<uint64_t> m_writes;  // NOLINT(runtime/int)

    // Indicates a TLM response error
    bool response_error;
};

#include "core/common/ahbmaster.tpp"

#endif  // MODELS_UTILS_AHBMASTER_H_
/// @}
