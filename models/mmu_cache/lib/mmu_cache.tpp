/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mmu_cache.cpp - Class definition of a cache-subsystem.    */
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


// Constructor
SC_HAS_PROCESS(mmu_cache<>);
template <int dsu, int cen, int repl, int sets, int linesize, int setsize, int setlock, int snoop, int lram, int lramsize, int lramstart, int memtech, int cached> 
mmu_cache<dsu, cen, repl, sets, linesize, setsize, setlock, snoop, lram, lramsize, lramstart, memtech, cached>::mmu_cache(sc_core::sc_module_name name, unsigned int id, sc_core::sc_time icache_hit_read_response_delay, sc_core::sc_time icache_miss_read_response_delay) : 	    sc_module(name), 
	icio("icio"), 
	ahb_master("ahb_master_socket", amba::amba_AHB, amba::amba_LT, false), 
	master_id(id), 
	m_txn_count(0), 
	m_data_count(0),
	m_bus_granted(false),
	current_trans(NULL),
	m_request_pending(false),
	m_data_pending(false),
	m_bus_req_pending(false),
	m_restart_pending_req(false){

  // create icache
  icache = new ivectorcache("ivectorcache",*this, icache_hit_read_response_delay, icache_miss_read_response_delay, sets, setsize, linesize, repl);

  // register forward transport function for icio socket (slave)
  icio.register_b_transport(this, &mmu_cache::custom_b_transport);

}

// TLM forward transport for icio socket
template <int dsu, int cen, int repl, int sets, int linesize, int setsize, int setlock, int snoop, int lram, int lramsize, int lramstart, int memtech, int cached> 
void mmu_cache<dsu, cen, repl, sets, linesize, setsize, setlock, snoop, lram, lramsize, lramstart, memtech, cached>::custom_b_transport(tlm::tlm_generic_payload& tran, sc_core::sc_time& delay) {

  // extract payload
  tlm::tlm_command cmd = tran.get_command();
  sc_dt::uint64    adr = tran.get_address();
  unsigned char*   ptr = tran.get_data_ptr();
  // unsigned int     len = tran.get_data_length();
  // unsigned char*   byt = tran.get_byte_enable_ptr();
  // unsigned int     wid = tran.get_streaming_width();

  if(cmd==tlm::TLM_READ_COMMAND) 
  {
    icache->read((unsigned int)adr, (unsigned int*)ptr, &delay);

    //cout << "ICIO Socket data received: " << hex << *(unsigned int*)ptr << endl;    
  }
  else if(cmd==tlm::TLM_WRITE_COMMAND) 
  {
    //std::cout << "command not valid on instruction cache (tlm_write)" << std::endl;
  }

}


template <int dsu, int cen, int repl, int sets, int linesize, int setsize, int setlock, int snoop, int lram, int lramsize, int lramstart, int memtech, int cached>
void mmu_cache<dsu, cen, repl, sets, linesize, setsize, setlock, snoop, lram, lramsize, lramstart, memtech, cached>::amba_write(unsigned int addr, unsigned int data, unsigned int length) {

	sc_core::sc_time t;

	// init transaction
	tlm::tlm_generic_payload *gp = ahb_master.get_transaction();
	gp->set_command(tlm::TLM_WRITE_COMMAND);
	gp->set_address(addr);
	gp->set_data_length(length);
	gp->set_streaming_width(4);
	gp->set_byte_enable_ptr(NULL);
	gp->set_data_ptr((unsigned char*)&data);

	// issue transaction
	ahb_master->b_transport(*gp,t);

	//cout << "WRITE " << gp->get_response_string() << ": 0x" << hex << gp->get_address();

	// burn the time
	wait(t);
	ahb_master.release_transaction(gp);
}

template <int dsu, int cen, int repl, int sets, int linesize, int setsize, int setlock, int snoop, int lram, int lramsize, int lramstart, int memtech, int cached>
void mmu_cache<dsu, cen, repl, sets, linesize, setsize, setlock, snoop, lram, lramsize, lramstart, memtech, cached>::amba_read(unsigned int addr, unsigned int * data, unsigned int length) {

	sc_core::sc_time t;

	// init transaction
	tlm::tlm_generic_payload *gp = ahb_master.get_transaction();
	gp->set_command(tlm::TLM_READ_COMMAND);
	gp->set_address(addr);
	gp->set_data_length(length);
	gp->set_streaming_width(4);
	gp->set_byte_enable_ptr(NULL);
	gp->set_data_ptr((unsigned char*)data);
	gp->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	// issue transaction
	ahb_master->b_transport(*gp,t);
	
	//cout << " READ " << gp->get_response_string() << " address: " << hex << gp->get_address() << " data: " << hex << *data << endl;

	// burn the time
	wait(t);
	ahb_master.release_transaction(gp);
}

