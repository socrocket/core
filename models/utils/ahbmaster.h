// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file ahbmaster.h
/// 
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Thomas Schuster
///
#ifndef MODELS_UTILS_AHBMASTER_H_
#define MODELS_UTILS_AHBMASTER_H_

#include <tlm.h>
#include <systemc.h>

#include <greencontrol/config.h>

#include <amba.h>

#include "ahbdevice.h"
#include "verbose.h"
#include "msclogger.h"

#include <stdint.h>

template<class BASE=sc_module>
class AHBMaster : public BASE, public AHBDevice {

 public:
  SC_HAS_PROCESS(AHBMaster);

  using BASE::name;

  /// Constructor
  AHBMaster(sc_core::sc_module_name nm,
            uint8_t hindex,
            uint8_t vendor,
            uint8_t device,
            uint8_t version,
            uint8_t irq,
            amba::amba_layer_ids ambaLayer,
            uint32_t bar0 = 0,
            uint32_t bar1 = 0,
            uint32_t bar2 = 0,
            uint32_t bar3 = 0,
	    uint32_t register_count = 16
            );

  /// Destructor
  ~AHBMaster();

  /// The AHB master socketo
  amba::amba_master_socket<32> ahb;

  /// Read data from AHB (SC_ZERO_TIME version)
  virtual void ahbread(uint32_t addr, unsigned char * data, uint32_t length);

  /// Write data to AHB (SC_ZERO_TIME version)
  virtual void ahbwrite(uint32_t addr, unsigned char * data, uint32_t length);

  /// Read data from AHB
  virtual void ahbread(uint32_t addr, unsigned char * data, uint32_t length, sc_core::sc_time &delay, bool &cacheable, tlm::tlm_response_status &response );

  /// Read data from AHB
  virtual void ahbread(uint32_t addr, unsigned char * data, uint32_t length, sc_core::sc_time &delay, bool &cacheable, bool is_lock, tlm::tlm_response_status &response );

  /// Write data to AHB
  virtual void ahbwrite(uint32_t addr, unsigned char * data, uint32_t length, sc_core::sc_time &delay, tlm::tlm_response_status &response);

  /// Write data to AHB
  virtual void ahbwrite(uint32_t addr, unsigned char * data, uint32_t length, sc_core::sc_time &delay, bool is_lock, tlm::tlm_response_status &response);

  /// Debug read from AHB
  virtual uint32_t ahbread_dbg(uint32_t addr, unsigned char * data, uint32_t length);

  /// Debug write from AHB
  virtual uint32_t ahbwrite_dbg(uint32_t addr, unsigned char * data, uint32_t length);

  /// Function may be implemented by the child for payload checking (e.g. testbench master)
  virtual void response_callback(tlm::tlm_generic_payload * trans) {};

  /// TLM non-blocking backward transport function
  virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay);

  /// Returns clock cycle time from master
  virtual sc_core::sc_time get_clock() = 0;

  /// Thread for response processing (read)
  void ResponseThread();

  /// Thread for processing data-phase of write transactions
  void DataThread();

        /// Collect common transport statistics.
        virtual void transport_statistics(tlm::tlm_generic_payload &gp) throw();

        /// Print common transport statistics.
        virtual void print_transport_statistics(const char *name) const throw();

 protected:

  /// PEQs for response synchronization
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_ResponsePEQ;

  /// Events for phase notifications
  sc_event m_EndRequestEvent;

  /// The abstraction layer of the instance (LT or AT)
  amba::amba_layer_ids m_ambaLayer;

  /// GreenControl API container
  gs::cnf::cnf_api *m_api;

  /// Open a namespace for performance counting in the greencontrol realm
  gs::gs_param_array m_performance_counters;

  /// Stores the number of Bytes read from the device
  gs::gs_param<unsigned long long> m_reads;

  /// Stores the number of Bytes written from the device
  gs::gs_param<unsigned long long> m_writes;

  // Indicates a TLM response error
  bool response_error;

};

#include "ahbmaster.tpp"

#endif // MODELS_UTILS_AHBMASTER_H_
/// @}