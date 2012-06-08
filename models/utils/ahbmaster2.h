#ifndef AHBMASTER_H
#define AHBMASTER_H

#include <stdint.h>

#include <greencontrol/config.h>
#include <greensocket/initiator/multi_socket.h>
#include <greenreg_ambasockets.h>


#include <tlm.h>
#include <amba.h>
#include <systemc.h>

#include "ahbdevice.h"
#include "verbose.h"
#include "msclogger.h"

template<class BASE=sc_module>
class AHBMaster : public BASE, public AHBDevice {

 public:
  SC_HAS_PROCESS(AHBMaster);

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
            uint32_t bar3 = 0
            );

  /// Destructor
  ~AHBMaster();

  /// The AHB master socket
  amba::amba_master_socket<32> ahb;

  /// Read data from AHB (SC_ZERO_TIME version)
  virtual void ahbread(uint32_t addr, unsigned char * data, uint32_t length);

  /// Write data to AHB (SC_ZERO_TIME version)
  virtual void ahbwrite(uint32_t addr, unsigned char * data, uint32_t length);

  /// Read data from AHB
  virtual void ahbread(uint32_t addr, unsigned char * data, uint32_t length, sc_core::sc_time &delay, bool &cacheable, tlm::tlm_response_status &response );

  /// Write data to AHB
  virtual void ahbwrite(uint32_t addr, unsigned char * data, uint32_t length, sc_core::sc_time &delay, tlm::tlm_response_status &response);

  /// Debug read from AHB
  virtual uint32_t ahbread_dbg(uint32_t addr, unsigned char * data, uint32_t length);

  /// Debug write from AHB
  virtual uint32_t ahbwrite_dbg(uint32_t addr, unsigned char * data, uint32_t length);

  /// Function may be implemented by the child for payload checking (e.g. testbench master)
  virtual void response_callback(tlm::tlm_generic_payload * trans) {};

  /// TLM non-blocking backward transport function
  virtual tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay);

  /// Thread for response processing (read)
  void ResponseThread();

  /// Thread for processing data-phase of write transactions
  void DataThread();

 protected:

  /// PEQs for response synchronization
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_ResponsePEQ;

  /// Events for phase notifications
  sc_event m_EndRequestEvent;

  /// The abstraction layer of the instance (LT or AT)
  amba::amba_layer_ids m_ambaLayer;

  sc_core::sc_time clock_cycle;

};

#include "ahbmaster2.tpp"

#endif // AHBMASTER_H
