#ifndef __MMU_CACHE_TEST_H__
#define __MMU_CACHE_TEST_H__

#include <tlm.h>
#include "amba.h"
#include "socrocket.h"

#if defined(MTI_SYSTEMC) || defined(NO_INCLUDE_PATHS)
#include "simple_initiator_socket.h"
#else
#include "tlm_utils/simple_initiator_socket.h"
#endif

#include "verbose.h"
#include "defines.h"
#include "icio_payload_extension.h"
#include "dcio_payload_extension.h"

// All mmu_cache tests inherit from this class
class mmu_cache_test : public sc_module {

 public:
  
  /// TLM2.0 initiator sockets for instruction and data
  tlm_utils::simple_initiator_socket<mmu_cache_test> icio;
  tlm_utils::simple_initiator_socket<mmu_cache_test> dcio;  

  SC_HAS_PROCESS(mmu_cache_test);
  
  /// TLM non-blocking backward transport functions
  tlm::tlm_sync_enum icio_nb_transport_bw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay);
  tlm::tlm_sync_enum dcio_nb_transport_bw(tlm::tlm_generic_payload &trans, tlm::tlm_phase &phase, sc_core::sc_time &delay);

  // Instruction read
  void iread(unsigned int addr, unsigned char * data, unsigned int flush, unsigned int flushl, unsigned int fline, unsigned int *debug);

  // Data read
  void dread(unsigned int addr, unsigned char * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock, unsigned int *debug);

  // Data write
  void dwrite(unsigned int addr, unsigned char * data, unsigned int length, unsigned int asi, unsigned int flush, unsigned int flushl, unsigned int lock, unsigned int *debug);

  // Displays and returns number of errors during test
  unsigned int error_stat();

  /// Function for result checking
  void check(unsigned char * result, unsigned char * refer, unsigned int lenght);

  /// Thread for delayed result checking (AT pipeline)
  void check_delayed();

  /// Helper functions for maintaining data, reference and debug pointers
  unsigned char * get_datap();
  unsigned char * get_datap_word(unsigned int value);
  unsigned char * get_datap_short(unsigned int value);
  unsigned char * get_datap_byte(unsigned int value);
  unsigned char * get_refp();
  unsigned char * get_refp_word(unsigned int value);
  unsigned char * get_refp_short(unsigned int value);
  unsigned char * get_refp_byte(unsigned int value);

  unsigned int  * get_debugp();
  void inc_tptr();

  // Delayed release of transactions
  void cleanUP();

  /// Thread for instruction response processing
  void InstrResponseThread();

  /// Thread for data response processing
  void DataResponseThread();

  /// Thread for processing write data-phase
  void DataThread();

  /// Constructor
  mmu_cache_test(sc_core::sc_module_name name, amba::amba_layer_ids abstractionLayer);

  
  // Data members
  // ------------

  /// PEQs for response synchronization
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_InstrResponsePEQ;
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_DataResponsePEQ;
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_DataPEQ;
  tlm_utils::peq_with_get<tlm::tlm_generic_payload> m_EndTransactionPEQ;

  /// Events for phase notifications
  sc_event m_EndInstrRequestEvent;
  sc_event m_EndDataRequestEvent;
  sc_event m_BeginResponseEvent;
  sc_event m_EndDataEvent;

  /// For result checking
  typedef struct {
    unsigned char * result;
    unsigned char * refer;
    unsigned int len;
    sc_time check_time;
  } checkpair_type;

  tlm_utils::peq_with_get<checkpair_type> m_CheckPEQ;

  // Space for keeping track of results, references and debug info
  unsigned char data[1024];
  unsigned char ref[1024];
  unsigned int debug[256];

  unsigned int tc;
  unsigned int ec;

 protected:

  amba::amba_layer_ids m_abstractionLayer;

};

#endif // __MMU_CACHE_TEST_H
		 
