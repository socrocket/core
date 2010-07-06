/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mmu_cache.h - Class definition of a cache-subsystem.    */
/*             The cache-subsystem envelopes an instruction cache,     */
/*             a data cache and a memory management unit.              */
/*             The mmu_cache class provides two TLM slave interfaces   */
/*             for connecting the cpu to the caches and an AHB master  */
/*             interface for connection to the main memory.            */ 
/*                                                                     */
/* Modified on $Date$                                                  */
/*          at $Revision$                                              */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/

#ifndef __MMU_CACHE_H__
#define __MMU_CACHE_H__

#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#include "amba.h"
#include "ivectorcache.h"

#include "mmu_cache_if.h"

#include <math.h>
#include <ostream>


template <int dsu=0, int cen=0, int repl=0, int sets=4, int linesize=4, int setsize=1, int setlock=1, int snoop=0, 
	  int lram=0, int lramsize=1, int lramstart=0x8f, int memtech=0, int cached=0> 
class mmu_cache : public mmu_cache_if, public sc_core::sc_module {

  public:

  // TLM sockets
  // -----------

  // iu3 instruction cache in/out
  typedef tlm::tlm_generic_payload *gp_ptr;
  tlm_utils::simple_target_socket<mmu_cache> icio;

  // amba master socket
  amba::amba_master_socket<32> ahb_master;

  // constructor
  // args: name of sysc module, id of the AHB master, icache delay for read hit, icache delay for read miss
  mmu_cache(sc_core::sc_module_name name, unsigned int id, sc_core::sc_time icache_hit_read_response_delay, sc_core::sc_time icache_miss_read_response_delay);

  // member functions
  // ----------------
  // forward transport function for icio socket
  void custom_b_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay_time);

  // interface to AMBA master socket
  void amba_write(unsigned int addr, unsigned int data, unsigned int length);
  void amba_read(unsigned int addr, unsigned int * data, unsigned int length);

  // data members
  // ------------
  // instruction cache
  ivectorcache * icache;

  private:

  // amba related
  unsigned int master_id;
  unsigned int m_txn_count;
  unsigned int m_data_count;
  bool m_bus_granted;
  tlm::tlm_generic_payload *current_trans;
  bool m_request_pending;
  bool m_data_pending;
  bool m_bus_req_pending;
  bool m_restart_pending_req;

};

#include "mmu_cache.tpp"

#endif //__MMU_CACHE_H__
