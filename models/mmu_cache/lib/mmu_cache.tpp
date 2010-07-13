/***********************************************************************/
/* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     */
/*                                                                     */
/* File:       mmu_cache.cpp - Class definition of a cache-subsystem.  */
/*             The cache-subsystem envelopes an instruction cache,     */
/*             a data cache and a memory management unit.              */
/*             The mmu_cache class provides two TLM slave interfaces   */
/*             for connecting the cpu to the caches and an AHB master  */
/*             interface for connection to the main memory.            */ 
/*                                                                     */
/* Modified on $Date$   */
/*          at $Revision$                                         */
/*                                                                     */
/* Principal:  European Space Agency                                   */
/* Author:     VLSI working group @ IDA @ TUBS                         */
/* Maintainer: Thomas Schuster                                         */
/***********************************************************************/


// Constructor
SC_HAS_PROCESS(mmu_cache<>);
template <int dsu, int icen, int irepl, int isets, int ilinesize, int isetsize, int isetlock,
	  int dcen, int drepl, int dsets, int dlinesize, int dsetsize, int dsetlock, int dsnoop,
	  int ilram, int ilramsize, int ilramstart, int dlram, int dlramsize, int dlramstart, int cached,
  	  int mmu_en, int itlb_num, int dtlb_num, int tlb_type, int tlb_rep, int mmupgsz>
mmu_cache<dsu, icen, irepl, isets, ilinesize, isetsize, isetlock,
	  dcen, drepl, dsets, dlinesize, dsetsize, dsetlock, dsnoop,
	  ilram, ilramsize, ilramstart, dlram, dlramsize, dlramstart, cached,
	  mmu_en, itlb_num, dtlb_num, tlb_type, tlb_rep, mmupgsz>::mmu_cache(sc_core::sc_module_name name, 
		unsigned int id, 
		sc_core::sc_time icache_hit_read_response_delay, 
		sc_core::sc_time icache_miss_read_response_delay,
		sc_core::sc_time dcache_hit_read_response_delay,
		sc_core::sc_time dcache_miss_read_response_delay,
		sc_core::sc_time dcache_write_response_delay,
		sc_core::sc_time itlb_hit_response_delay,
		sc_core::sc_time itlb_miss_response_delay,
		sc_core::sc_time dtlb_hit_response_delay,
		sc_core::sc_time dtlb_miss_response_delay) : sc_module(name), 
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

			// create mmu (if required)
			srmmu = (mmu_en==1)? new mmu("mmu", 
					*this,
					itlb_hit_response_delay,
					itlb_miss_response_delay,
					dtlb_hit_response_delay,
					dtlb_miss_response_delay,
					itlb_num,
					dtlb_num,
					tlb_type,
					tlb_rep,
					mmupgsz) : NULL;

			// create icache
			icache = new ivectorcache("ivectorcache",
					*this,
					srmmu,
					mmu_en,
					icache_hit_read_response_delay,
					icache_miss_read_response_delay, 
					isets, 
					isetsize, 
					ilinesize,
					irepl);

			// create dcache
			dcache = new dvectorcache("dvectorcache",
					*this,
					srmmu,
				 	mmu_en,
					dcache_hit_read_response_delay,
					dcache_miss_read_response_delay,
					dcache_write_response_delay,
					dsets,
					dsetsize,
					dlinesize,
					drepl);

			// register forward transport functions for icio and dcio sockets (slave)
			icio.register_b_transport(this, &mmu_cache::icio_custom_b_transport);
			dcio.register_b_transport(this, &mmu_cache::dcio_custom_b_transport);
}

// TLM forward transport for icio socket
template <int dsu, int icen, int irepl, int isets, int ilinesize, int isetsize, int isetlock,
	  int dcen, int drepl, int dsets, int dlinesize, int dsetsize, int dsetlock, int dsnoop,
	  int ilram, int ilramsize, int ilramstart, int dlram, int dlramsize, int dlramstart, int cached,
	  int mmu_en, int itlb_num, int dtlb_num, int tlb_type, int tlb_rep, int mmupgsz> 
void mmu_cache<dsu, icen, irepl, isets, ilinesize, isetsize, isetlock,
	  	dcen, drepl, dsets, dlinesize, dsetsize, dsetlock, dsnoop,
		ilram, ilramsize, ilramstart, dlram, dlramsize, dlramstart, cached,
		mmu_en, itlb_num, dtlb_num, tlb_type, tlb_rep, mmupgsz>::icio_custom_b_transport(tlm::tlm_generic_payload& tran, sc_core::sc_time& delay) {

  // extract payload
  tlm::tlm_command   cmd  = tran.get_command();
  sc_dt::uint64      adr  = tran.get_address();
  unsigned char*     ptr  = tran.get_data_ptr();
  // unsigned int     len = tran.get_data_length();
  // unsigned char*   byt = tran.get_byte_enable_ptr();
  // unsigned int     wid = tran.get_streaming_width();

  // extract extension
  icio_payload_extension * iext;
  tran.get_extension(iext);

  if(cmd==tlm::TLM_READ_COMMAND) 
  {
    icache->read((unsigned int)adr, (unsigned int*)ptr, iext, &delay);

    //DUMP(name(),"ICIO Socket data received (tlm_read): " << hex << *(unsigned int*)ptr);    
  }
  else if(cmd==tlm::TLM_WRITE_COMMAND) 
  {
    //DUMP(name(),"Command not valid for instruction cache (tlm_write)");
  }

}

// TLM forward transport for dcio socket
template <int dsu, int icen, int irepl, int isets, int ilinesize, int isetsize, int isetlock,
	  int dcen, int drepl, int dsets, int dlinesize, int dsetsize, int dsetlock, int dsnoop,
	  int ilram, int ilramsize, int ilramstart, int dlram, int dlramsize, int dlramstart, int cached,
	  int mmu_en, int itlb_num, int dtlb_num, int tlb_type, int tlb_rep, int mmupgsz>
void mmu_cache<dsu, icen, irepl, isets, ilinesize, isetsize, isetlock,
	  	dcen, drepl, dsets, dlinesize, dsetsize, dsetlock, dsnoop,
		ilram, ilramsize, ilramstart, dlram, dlramsize, dlramstart, cached,
		mmu_en, itlb_num, dtlb_num, tlb_type, tlb_rep, mmupgsz>::dcio_custom_b_transport(tlm::tlm_generic_payload& tran, sc_core::sc_time& delay) {
 
  // extract payload
  tlm::tlm_command cmd = tran.get_command();
  sc_dt::uint64    adr = tran.get_address();
  unsigned char*   ptr = tran.get_data_ptr();
  // unsigned int     len = tran.get_data_length();
  unsigned char*   byt = tran.get_byte_enable_ptr();
  // unsigned int     wid = tran.get_streaming_width();

  // extract extension
  dcio_payload_extension * dext;
  tran.get_extension(dext);

  if(cmd==tlm::TLM_READ_COMMAND) 
  {
    dcache->read((unsigned int)adr, (unsigned int*)ptr, dext, &delay);

    //DUMP(name(),"DCIO Socket data received (tlm_read): " << hex << *(unsigned int*)ptr);    
  }
  else if(cmd==tlm::TLM_WRITE_COMMAND) 
  {
    dcache->write((unsigned int)adr, (unsigned int*)ptr, (unsigned int*)byt, dext, &delay);
    //DUMP(name(),"DCIO Socket done tlm_write");
  }

}

// Function for write access to AHB master socket
template <int dsu, int icen, int irepl, int isets, int ilinesize, int isetsize, int isetlock,
	  int dcen, int drepl, int dsets, int dlinesize, int dsetsize, int dsetlock, int dsnoop,
	  int ilram, int ilramsize, int ilramstart, int dlram, int dlramsize, int dlramstart, int cached,
	  int mmu_en, int itlb_num, int dtlb_num, int tlb_type, int tlb_rep, int mmupgsz>
void mmu_cache<dsu, icen, irepl, isets, ilinesize, isetsize, isetlock,
	  dcen, drepl, dsets, dlinesize, dsetsize, dsetlock, dsnoop,
	  ilram, ilramsize, ilramstart, dlram, dlramsize, dlramstart, cached,
	  mmu_en, itlb_num, dtlb_num, tlb_type, tlb_rep, mmupgsz>::amba_write(unsigned int addr, unsigned int * data, unsigned int length) {

	sc_core::sc_time t;

	// init transaction
	tlm::tlm_generic_payload *gp = ahb_master.get_transaction();
	gp->set_command(tlm::TLM_WRITE_COMMAND);
	gp->set_address(addr);
	gp->set_data_length(length);
	gp->set_streaming_width(4);
	gp->set_byte_enable_ptr(NULL);
	gp->set_data_ptr((unsigned char*)data);

	// issue transaction
	ahb_master->b_transport(*gp,t);

	//cout << "WRITE " << gp->get_response_string() << ": 0x" << hex << gp->get_address();

	// burn the time
	wait(t);
	ahb_master.release_transaction(gp);
}

// Function for read access to AHB master socket
template <int dsu, int icen, int irepl, int isets, int ilinesize, int isetsize, int isetlock,
	  int dcen, int drepl, int dsets, int dlinesize, int dsetsize, int dsetlock, int dsnoop,
	  int ilram, int ilramsize, int ilramstart, int dlram, int dlramsize, int dlramstart, int cached,
	  int mmu_en, int itlb_num, int dtlb_num, int tlb_type, int tlb_rep, int mmupgsz>
void mmu_cache<dsu, icen, irepl, isets, ilinesize, isetsize, isetlock,
	  dcen, drepl, dsets, dlinesize, dsetsize, dsetlock, dsnoop,
	  ilram, ilramsize, ilramstart, dlram, dlramsize, dlramstart, cached,
	  mmu_en, itlb_num, dtlb_num, tlb_type, tlb_rep, mmupgsz>::amba_read(unsigned int addr, unsigned int * data, unsigned int length) {

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
	
	// burn the time (do here or better leaf to master ?)
	wait(t);
	ahb_master.release_transaction(gp);
}

