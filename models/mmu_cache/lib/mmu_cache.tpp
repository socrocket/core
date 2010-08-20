// ***********************************************************************
// * Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *
// *                                                                     *
// * File:       mmu_cache.cpp - Class definition of a cache-subsystem.  *
// *             The cache-subsystem envelopes an instruction cache,     *
// *             a data cache and a memory management unit.              *
// *             The mmu_cache class provides two TLM slave interfaces   *
// *             for connecting the cpu to the caches and an AHB master  *
// *             interface for connection to the main memory.            * 
// *                                                                     *
// * Modified on $Date$   *
// *          at $Revision$                                         *
// *                                                                     *
// * Principal:  European Space Agency                                   *
// * Author:     VLSI working group @ IDA @ TUBS                         *
// * Maintainer: Thomas Schuster                                         *
// ***********************************************************************


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
			m_restart_pending_req(false)
{
  // parameter checks
  // ----------------
  // todo

  // create mmu (if required)
  m_mmu = (mmu_en==1)? new mmu("mmu", 
		(mmu_cache_if *)this,
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
  icache = (icen==1)? new ivectorcache("ivectorcache",
		(mmu_cache_if *)this,
		(mmu_en) ? (mem_if *)m_mmu->get_itlb_if() : (mem_if *)this,
		mmu_en,
		icache_hit_read_response_delay,
		icache_miss_read_response_delay,
		isets, 
		isetsize, 
		ilinesize,
		irepl,
		ilram,
		ilramstart,
		ilramsize) : NULL;

  // create dcache
  dcache = (dcen==1)? new dvectorcache("dvectorcache",
		(mmu_cache_if *)this,
		(mmu_en) ? (mem_if *)m_mmu->get_dtlb_if() : (mem_if *) this,
		mmu_en,
		dcache_hit_read_response_delay,
		dcache_miss_read_response_delay,
		dcache_write_response_delay,
		dsets,
		dsetsize,
		dlinesize,
		drepl,
		dlram,
		dlramstart,
		dlramsize) : NULL;

  // create instruction scratchpad
  // (! only allowed with mmu disabled !)
  ilocalram = ((ilram == 1)&&(mmu_en==0)) ? new localram("ilocalram",
		ilramsize,
		ilramstart) :NULL;

  // create data scratchpad
  // (! only allowed with mmu disabled !)
  dlocalram = ((dlram == 1)&&(mmu_en==0)) ? new localram("dlocalram",
		dlramsize,
		dlramstart) : NULL;

  // register forward transport functions for icio and dcio sockets (slave)
  icio.register_b_transport(this, &mmu_cache::icio_custom_b_transport);
  dcio.register_b_transport(this, &mmu_cache::dcio_custom_b_transport);

  // initialize cache control registers
  CACHE_CONTROL_REG = 0;
	
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
  unsigned int       len = tran.get_data_length();
  // unsigned char*   byt = tran.get_byte_enable_ptr();
  // unsigned int     wid = tran.get_streaming_width();

  // extract extension
  icio_payload_extension * iext;
  tran.get_extension(iext);

  unsigned int * debug = iext->debug;

  if(cmd==tlm::TLM_READ_COMMAND) 
  {
    // instruction scratchpad enabled && address points into selecte 16MB region 
    if(ilram && (((adr >> 24) & 0xff)==ilramstart)) {

	ilocalram->read((unsigned int)adr, ptr, len, &delay, debug); 

    } else {

    	icache->read((unsigned int)adr, ptr, len, &delay, debug);
    	//DUMP(name(),"ICIO Socket data received (tlm_read): " << hex << *(unsigned int*)ptr);    
    }
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
  unsigned int     len = tran.get_data_length();
  // unsigned char*   byt = tran.get_byte_enable_ptr();
  // unsigned int     wid = tran.get_streaming_width();

  // extract extension
  dcio_payload_extension * dext;
  tran.get_extension(dext);

  unsigned int asi     = dext->asi;
  unsigned int * debug = dext->debug;

  // access system registers
  if (asi == 2) {

	if (cmd==tlm::TLM_READ_COMMAND) {
		
		DUMP(this->name(),"System Registers read with ASI 0x2 - addr:" << std::hex << adr);
		if (adr == 0) {
			// cache control register
			*(unsigned int *)ptr = read_ccr();
		}
		else if (adr == 8) {
			// instruction cache configuration register
			*(unsigned int *)ptr = icache->read_config_reg(&delay);
		}
		else if (adr == 0x0c) {
			// data cache configuration register
			*(unsigned int *)ptr = dcache->read_config_reg(&delay);
		}
		else {
			DUMP(this->name(),"Address not valid for read with ASI 0x2");
			*ptr = 0;
		}
	} 
	else if (cmd==tlm::TLM_WRITE_COMMAND) {

		DUMP(this->name(),"System Register write with ASI 0x2 - addr:" << std::hex << adr);
		if (adr == 0) {
			// cache control register
			write_ccr(ptr, len, &delay);
		}
		// TRIGGER DEBUG OUTPUT / NOT A SPARC SYSTEM REGISTER
		else if (adr == 0xfe) {
			// icache debug output (arg: line)
			icache->dbg_out(*ptr);
		}
		else if (adr == 0xff) {
			// dcache debug output (arg: line)
			dcache->dbg_out(*ptr);
		}
		else {
			DUMP(this->name(),"Address not valid for write with ASI 0x2 (or read only)");
			// ignore (cache configuration regs (0x8, 0xc) are read only
		}
	}
	else {
		DUMP(this->name(),"Unvalid TLM Command");
		assert(false);
	}
  }
  // diagnostic access to instruction PDC
  else if (asi == 0x5) {

	if (cmd==tlm::TLM_READ_COMMAND) {
	
		DUMP(this->name(),"Diagnostic read from instruction PDC (ASI 0x5)");
		m_mmu->diag_read_itlb(adr, (unsigned int *)ptr);
	}
	else if (cmd==tlm::TLM_WRITE_COMMAND) {

		DUMP(this->name(),"Diagnostic write to instruction PDC (ASI 0x5)");
		m_mmu->diag_write_itlb(adr, (unsigned int *)ptr);
	}	
  }
  // diagnostic access to data or shared instruction and data PDC
  else if (asi == 0x6) {

	if (cmd==tlm::TLM_READ_COMMAND) {
	
		DUMP(this->name(),"Diagnostic read from data (or shared) PDC (ASI 0x6)");
		m_mmu->diag_read_dctlb(adr, (unsigned int *)ptr);
	}
	else if (cmd==tlm::TLM_WRITE_COMMAND) {

		DUMP(this->name(),"Diagnostic write to data (or shared) PDC (ASI 0x6)");
		m_mmu->diag_write_dctlb(adr, (unsigned int *)ptr);
	}
  }
  // access instruction cache tags
  else if (asi == 0xc) {

	if (cmd==tlm::TLM_READ_COMMAND) {
		DUMP(this->name(),"ASI read instruction cache tags");
		icache->read_cache_tag((unsigned int)adr, (unsigned int*)ptr, &delay);
	}
	else if (cmd==tlm::TLM_WRITE_COMMAND) {
		DUMP(this->name(),"ASI write instruction cache tags");
		icache->write_cache_tag((unsigned int)adr, (unsigned int*)ptr, &delay);
	}
	else {
		DUMP(this->name(), "Unvalid TLM Command");
	}
  }
  // access instruction cache data
  else if (asi == 0xd) {

	if (cmd==tlm::TLM_READ_COMMAND) {
		DUMP(this->name(),"ASI read instruction cache entry");
		icache->read_cache_entry((unsigned int)adr, (unsigned int*)ptr, &delay);
	}
	else if (cmd==tlm::TLM_WRITE_COMMAND) {
		DUMP(this->name(),"ASI write instruction cache entry");
		icache->write_cache_entry((unsigned int)adr, (unsigned int*)ptr, &delay);
	}
	else {
		DUMP(this->name(), "Unvalid TLM Command");
	}
  }
  // access data cache tags
  else if (asi == 0xe) {

	if (cmd==tlm::TLM_READ_COMMAND) {
		DUMP(this->name(),"ASI read data cache tags");
		dcache->read_cache_tag((unsigned int)adr, (unsigned int*)ptr, &delay);
	}
	else if (cmd==tlm::TLM_WRITE_COMMAND) {
		DUMP(this->name(),"ASI write data cache tags");
		dcache->write_cache_tag((unsigned int)adr, (unsigned int*)ptr, &delay);
	}
	else {
		DUMP(this->name(), "Unvalid TLM Command");
	}
  }
  // access data cache data
  else if (asi == 0xf) {
	
	if (cmd==tlm::TLM_READ_COMMAND) {
		DUMP(this->name(),"ASI read data cache entry");
		dcache->read_cache_entry((unsigned int)adr, (unsigned int*)ptr, &delay);
	}
	else if (cmd==tlm::TLM_WRITE_COMMAND) {
		DUMP(this->name(),"ASI write data cache entry");
		dcache->write_cache_entry((unsigned int)adr, (unsigned int*)ptr, &delay);
	}
	else {
		DUMP(this->name(), "Unvalid TLM Command");
	}
  }
  // flush instruction cache
  else if (asi == 0x10) {

	// icache is flushed on any write with ASI 0x10
	if (cmd==tlm::TLM_WRITE_COMMAND) {
		DUMP(this->name(),"ASI flush instruction cache");
		icache->flush(&delay, debug);
	}
	else {
		DUMP(this->name(), "Unvalid TLM Command");
	}
  }
  // flush data cache
  else if (asi == 0x11) {

	// dcache is flushed on any write with ASI 0x11
	if (cmd==tlm::TLM_WRITE_COMMAND) {
		DUMP(this->name(), "ASI flush data cache");
		dcache->flush(&delay, debug);
	}
	else {
		DUMP(this->name(), "Unvalid TLM Command");
	}
  }
  // access MMU internal registers
  // (only allowed if mmu present)
  else if ((asi == 0x19)&&(mmu_en == 0x1)) {

    	if (cmd==tlm::TLM_READ_COMMAND) {
      
		DUMP(this->name(),"MMU register read with ASI 0x19 - addr:" << std::hex << adr);
      		if (adr==0x000) {
			// MMU Control Register
			*(unsigned int *)ptr = m_mmu->read_mcr();
     		}
      		else if (adr == 0x100) {
			// Context Pointer Register
			*(unsigned int *)ptr = m_mmu->read_mctpr();
      		}
      		else if (adr == 0x200) {
			// Context Register
			*(unsigned int *)ptr = m_mmu->read_mctxr();
      		}
      		else if (adr == 0x300) {
			// Fault Status Register
			*(unsigned int *)ptr = m_mmu->read_mfsr();
      		}
      		else if (adr == 0x400) {
			// Fault Address Register
			*(unsigned int *)ptr = m_mmu->read_mfar();
      		}
      		else {
			DUMP(this->name(),"Address not valid for read with ASI 0x19");
			*(unsigned int *)ptr = 0;
      		}
	
    	}
    	else if (cmd==tlm::TLM_WRITE_COMMAND) {

		DUMP(this->name(),"MMU register write with ASI 0x19 - addr:" << std::hex << adr);
      		if (adr==0x000) {
			// MMU Control Register
			m_mmu->write_mcr((unsigned int *)ptr);
      		}
     		else if (adr==0x100) {
			// Context Table Pointer Register
			m_mmu->write_mctpr((unsigned int*)ptr);
		}
      		else if (adr==0x200) {
			// Context Register
			m_mmu->write_mctxr((unsigned int*)ptr);
      		}
		else {
			DUMP(this->name(),"Address not valid for write with ASI 0x19 (or read-only)");
			// ignore
      		}
	}
  }
  // ordinary access
  else if ((asi == 0x8)||(asi == 0x9)||(asi == 0xa)||(asi == 0xb)) {

    if (cmd==tlm::TLM_READ_COMMAND) {

	// instruction scratchpad enabled && address points into selected 16 MB region 
	if (ilram && (((adr >> 24) & 0xff)==ilramstart)) {
		
	   ilocalram->read((unsigned int)adr, ptr, len, &delay, debug);

	// data scratchpad enabled && address points into selected 16MB region
	} else if (dlram && (((adr >> 24) & 0xff)==dlramstart)) {

	   dlocalram->read((unsigned int)adr, ptr, len, &delay, debug);
	
	// cache access
	} else {

	   dcache->read((unsigned int)adr, ptr, len, &delay, debug);
      	   //DUMP(name(),"DCIO Socket data received (tlm_read): " << std::hex << *(unsigned int*)ptr);    
	}
    }
    else if(cmd==tlm::TLM_WRITE_COMMAND) 
    {
	// instruction scratchpad enabled && address points into selected 16 MB region
	if (ilram && (((adr >> 24) & 0xff)==ilramstart)) {

	   ilocalram->write((unsigned int)adr, ptr, len, &delay, debug);

	// data scratchpad enabled && address points into selected 16MB region
	} else if (dlram && (((adr >> 24) & 0xff)==dlramstart)) {

	   dlocalram->write((unsigned int)adr, ptr, len, &delay, debug);

	// cache access (write through)
	} else {

       	   dcache->write((unsigned int)adr, ptr, len, &delay, debug);
	   //DUMP(name(),"DCIO Socket done tlm_write");
	}
    }
  }
  else {

   DUMP(name(),"ASI not recognized: " << std::hex << asi);
   assert(0);
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
	  mmu_en, itlb_num, dtlb_num, tlb_type, tlb_rep, mmupgsz>::mem_write(unsigned int addr, unsigned char * data, unsigned int len, sc_core::sc_time * t, unsigned int * debug) {

	sc_core::sc_time delay;

	// init transaction
	tlm::tlm_generic_payload *gp = ahb_master.get_transaction();
	gp->set_command(tlm::TLM_WRITE_COMMAND);
	gp->set_address(addr);
	gp->set_data_length(len);
	gp->set_data_ptr(data);

	amba::burst_size* size_ext;
	ahb_master.template validate_extension<amba::burst_size>(*gp);
	ahb_master.template get_extension<amba::burst_size>(size_ext, *gp);
	size_ext->value=len;
	
	// issue transaction
	ahb_master->b_transport(*gp, delay);

	// burn the time
	wait(delay);
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
	  mmu_en, itlb_num, dtlb_num, tlb_type, tlb_rep, mmupgsz>::mem_read(unsigned int addr, unsigned char * data, unsigned int len, sc_core::sc_time * t, unsigned int * debug) {

	sc_core::sc_time delay;

	// init transaction
	tlm::tlm_generic_payload *gp = ahb_master.get_transaction();
	gp->set_command(tlm::TLM_READ_COMMAND);
	gp->set_address(addr);
	gp->set_data_length(len);
	gp->set_data_ptr(data);
	gp->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	amba::burst_size* size_ext;
	ahb_master.template validate_extension<amba::burst_size>(*gp);
	ahb_master.template get_extension<amba::burst_size>(size_ext, *gp);
	size_ext->value=len;

	// issue transaction
	ahb_master->b_transport(*gp, delay);
	
	// burn the time
	wait(delay);
	ahb_master.release_transaction(gp);
}

// writes the cache control register and handles the commands
template <int dsu, int icen, int irepl, int isets, int ilinesize, int isetsize, int isetlock,
	  int dcen, int drepl, int dsets, int dlinesize, int dsetsize, int dsetlock, int dsnoop,
	  int ilram, int ilramsize, int ilramstart, int dlram, int dlramsize, int dlramstart, int cached,
	  int mmu_en, int itlb_num, int dtlb_num, int tlb_type, int tlb_rep, int mmupgsz>
void mmu_cache<dsu, icen, irepl, isets, ilinesize, isetsize, isetlock,
	  dcen, drepl, dsets, dlinesize, dsetsize, dsetlock, dsnoop,
	  ilram, ilramsize, ilramstart, dlram, dlramsize, dlramstart, cached,
	  mmu_en, itlb_num, dtlb_num, tlb_type, tlb_rep, mmupgsz>::write_ccr(unsigned char * data, unsigned int len, sc_core::sc_time * delay) {

	unsigned int tmp = 0;
	unsigned int dummy;

	memcpy(&tmp, data, len);

	// [DS] data cache snoop enable (todo) 
	if (tmp & (1<<23)) {}
	// [FD] dcache flush (do not set; always reads as zero)
	if (tmp & (1<<22)) { dcache->flush(delay, &dummy); }
	// [FI] icache flush (do not set; always reads as zero)
	if (tmp & (1<<21)) { icache->flush(delay, &dummy); }
	// [IB] instruction burst fetch (todo)
	if (tmp & (1<<16)) {}

	// [IP] Instruction cache flush pending (bit 15 - read only)
	// [DP] Data cache flush pending (bit 14 - read only)

	// [DF] data cache freeze on interrupt (todo)
	if (tmp & (1<<5)) {}

	// [IF] instruction cache freeze on interrupt (todo)
	if (tmp & (1<<4)) {}

	// [DCS] data cache state (bits 3:2 - read only)
	// [ICS] instruction cache state (bits 1:0 - read only)

	// read only masking: 1111 1111 1001 1111 0011 1111 1111 1111
	CACHE_CONTROL_REG = (tmp & 0xff9f3fff);
}

// read the cache control register
template <int dsu, int icen, int irepl, int isets, int ilinesize, int isetsize, int isetlock,
	  int dcen, int drepl, int dsets, int dlinesize, int dsetsize, int dsetlock, int dsnoop,
	  int ilram, int ilramsize, int ilramstart, int dlram, int dlramsize, int dlramstart, int cached,
	  int mmu_en, int itlb_num, int dtlb_num, int tlb_type, int tlb_rep, int mmupgsz>
unsigned int mmu_cache<dsu, icen, irepl, isets, ilinesize, isetsize, isetlock,
	  dcen, drepl, dsets, dlinesize, dsetsize, dsetlock, dsnoop,
	  ilram, ilramsize, ilramstart, dlram, dlramsize, dlramstart, cached,
	  mmu_en, itlb_num, dtlb_num, tlb_type, tlb_rep, mmupgsz>::read_ccr() {

	return(CACHE_CONTROL_REG);
}
