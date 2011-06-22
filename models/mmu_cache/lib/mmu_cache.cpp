//*********************************************************************
// Copyright 2010, Institute of Computer and Network Engineering,
//                 TU-Braunschweig
// All rights reserved
// Any reproduction, use, distribution or disclosure of this program,
// without the express, prior written consent of the authors is 
// strictly prohibited.
//
// University of Technology Braunschweig
// Institute of Computer and Network Engineering
// Hans-Sommer-Str. 66
// 38118 Braunschweig, Germany
//
// ESA SPECIAL LICENSE
//
// This program may be freely used, copied, modified, and redistributed
// by the European Space Agency for the Agency's own requirements.
//
// The program is provided "as is", ther is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      mmu_cache.cpp
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    Class definition of a cache-subsystem.
//             The cache-subsystem envelopes an instruction cache,
//             a data cache and a memory management unit.
//             The mmu_cache class provides two TLM slave interfaces
//             for connecting the cpu to the caches and an AHB master
//             interface for connection to the main memory.
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author$
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Thomas Schuster
// Reviewed:
//*********************************************************************

#include "mmu_cache.h"

//SC_HAS_PROCESS(mmu_cache<>);
/// Constructor
mmu_cache::mmu_cache(unsigned int icen, unsigned int irepl, unsigned int isets,
                     unsigned int ilinesize, unsigned int isetsize,
                     unsigned int isetlock, unsigned int dcen,
                     unsigned int drepl, unsigned int dsets,
                     unsigned int dlinesize, unsigned int dsetsize,
                     unsigned int dsetlock, unsigned int dsnoop,
                     unsigned int ilram, unsigned int ilramsize,
                     unsigned int ilramstart, unsigned int dlram,
                     unsigned int dlramsize, unsigned int dlramstart,
                     unsigned int cached, unsigned int mmu_en,
                     unsigned int itlb_num, unsigned int dtlb_num,
                     unsigned int tlb_type, unsigned int tlb_rep,
                     unsigned int mmupgsz, sc_core::sc_module_name name,
                     unsigned int id,
		     amba::amba_layer_ids abstractionLevel) :

    sc_module(name),
    AHBDevice(id,
	      0x01,  // vendor: Gaisler Research (Fake the LEON)
	      0x003,  // 
	      0,
	      0,
	      0,
	      0,
	      0,
	      0),
    icio("icio"), 
    dcio("dcio"), 
    ahb_master("ahb_master_socket", amba::amba_AHB, abstractionLevel, false),
    snoop(&mmu_cache::snoopingCallBack,"SNOOP"),
    icio_PEQ("icio_PEQ"), 
    dcio_PEQ("dcio_PEQ"),
    m_icen(icen), 
    m_dcen(dcen),
    m_dsnoop(dsnoop),
    m_ilram(ilram), 
    m_ilramstart(ilramstart),
    m_dlram(dlram),
    m_dlramstart(dlramstart),
    m_cached(cached),
    m_mmu_en(mmu_en),
    m_master_id(id), 
    m_abstractionLevel(abstractionLevel), 
    m_txn_count(0), 
    m_data_count(0),
    m_bus_granted(false), 
    current_trans(NULL),
    m_request_pending(false), 
    m_data_pending(false), 
    m_bus_req_pending(false), 
    m_restart_pending_req(false),
    clockcycle(10, sc_core::SC_NS) {

    // parameter checks
    // ----------------

    // check range of cacheability mask (0x0 - 0xffff)
    assert((m_cached>=0)&&(m_cached<=0xffff));

    // create mmu (if required)
    m_mmu = (mmu_en == 1)? new mmu("mmu", 
				   (mmu_cache_if *)this,
				   itlb_num,
				   dtlb_num,
				   tlb_type,
				   tlb_rep,
				   mmupgsz) : NULL;

    // create icache
    icache = (icen == 1)? (cache_if*)new ivectorcache("ivectorcache",
            (mmu_cache_if *)this, (mmu_en)? (mem_if *)m_mmu->get_itlb_if()
                                           : (mem_if *)this, mmu_en,
            isets, isetsize, isetlock, ilinesize, irepl, ilram, ilramstart,
            ilramsize) : (cache_if*)new nocache("no_icache",
            (mmu_en)? (mem_if *)m_mmu->get_itlb_if() : (mem_if *)this);

    // create dcache
    dcache = (dcen == 1)? (cache_if*)new dvectorcache("dvectorcache",
            (mmu_cache_if *)this, (mmu_en)? (mem_if *)m_mmu->get_dtlb_if()
                                           : (mem_if *)this, mmu_en,
            dsets, dsetsize, dsetlock, dlinesize, drepl, dlram, dlramstart, 
	    dlramsize) : (cache_if*)new nocache("no_dcache", 
	    (mmu_en)? (mem_if *)m_mmu->get_dtlb_if() : (mem_if *)this);

    // create instruction scratchpad
    // (! only allowed with mmu disabled !)
    ilocalram = ((ilram == 1) && (mmu_en == 0))? new localram("ilocalram",
            ilramsize, ilramstart) : NULL;

    // create data scratchpad
    // (! only allowed with mmu disabled !)
    dlocalram = ((dlram == 1) && (mmu_en == 0))? new localram("dlocalram",
            dlramsize, dlramstart) : NULL;

    if (abstractionLevel==amba::amba_LT) {

      // register blocking forward transport functions for icio and dcio sockets (slave)
      icio.register_b_transport(this, &mmu_cache::icio_custom_b_transport);
      dcio.register_b_transport(this, &mmu_cache::dcio_custom_b_transport);

    } else if (abstractionLevel==amba::amba_AT) {

      // register non-blocking forward transport functions for icio and dcio sockets
      icio.register_nb_transport_fw(this, &mmu_cache::icio_custom_nb_transport_fw);
      dcio.register_nb_transport_fw(this, &mmu_cache::dcio_custom_nb_transport_fw);

      // register non-blocking backward transport function for ahb_master socket
      ahb_master.register_nb_transport_bw(this, &mmu_cache::ahb_custom_nb_transport_bw);

      // register icio service thread (for AT)
      SC_THREAD(icio_service_thread);
      sensitive << icio_PEQ.get_event();
      dont_initialize();

      // register dcio service thread (for AT)
      SC_THREAD(dcio_service_thread);
      sensitive << dcio_PEQ.get_event();
      dont_initialize();

    } else {

      v::error << this->name() << "Abstraction Layer not valid!!" << v::endl;
      assert(0);

    }

    // initialize cache control registers
    CACHE_CONTROL_REG = 0;

}

/// TLM blocking forward transport function for icio socket
void mmu_cache::icio_custom_b_transport(tlm::tlm_generic_payload& tran,
                                        sc_core::sc_time& delay) {

    // extract payload
    tlm::tlm_command cmd = tran.get_command();
    sc_dt::uint64 addr = tran.get_address();
    unsigned char* ptr = tran.get_data_ptr();
    //unsigned int len = tran.get_data_length();

    // extract extension
    icio_payload_extension * iext;
    tran.get_extension(iext);

    unsigned int * debug = iext->debug;

    if (cmd == tlm::TLM_READ_COMMAND) {
        // instruction scratchpad enabled && address points into selecte 16MB region
        if (m_ilram && (((addr >> 24) & 0xff) == m_ilramstart)) {

            ilocalram->read((unsigned int)addr, ptr, 4, &delay, debug);

            // instruction cache access
        } else {

            icache->mem_read((unsigned int)addr, ptr, 4, &delay, debug);

        }

        // Setting gp response status to OK
        tran.set_response_status(tlm::TLM_OK_RESPONSE);
	return;

    } else {
        v::error << name()
                 << "Command not valid for instruction cache (tlm_write)"
                 << v::endl;

	tran.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	return;

    }
}

/// TLM forward blocking transport function for dcio socket
void mmu_cache::dcio_custom_b_transport(tlm::tlm_generic_payload& tran,
                                        sc_core::sc_time& delay) {

    // extract payload
    tlm::tlm_command cmd = tran.get_command();
    sc_dt::uint64 addr = tran.get_address();
    unsigned char* ptr = tran.get_data_ptr();
    unsigned int len = tran.get_data_length();

    // extract extension
    dcio_payload_extension * dext;
    tran.get_extension(dext);

    unsigned int asi = dext->asi;
    unsigned int * debug = dext->debug;

    // access system registers
    if (asi == 2) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << this->name()
                    << "System Registers read with ASI 0x2 - addr:" << std::hex
                    << addr << v::endl;

            if (addr == 0) {

                // cache control register
                *(unsigned int *)ptr = read_ccr();
		// Setting response status
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;

            } else if (addr == 8) {

                // instruction cache configuration register
                *(unsigned int *)ptr = icache->read_config_reg(&delay);
		// Setting response status
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;

            } else if (addr == 0x0c) {

                // data cache configuration register
                *(unsigned int *)ptr = dcache->read_config_reg(&delay);
		// Setting response status
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;

            } else {

                v::error << name()
                         << "Address not valid for read with ASI 0x2" 
			 << v::endl;
		
		// set TLM response
		tran.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
		return;
            }

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

	    v::debug << this->name()
                    << "System Register write with ASI 0x2 - addr:" << std::hex
                    << addr << v::endl;

            if (addr == 0) {

                // cache control register
                write_ccr(ptr, len, &delay);
		// Setting response status
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;


            // TRIGGER DEBUG OUTPUT / NOT A SPARC SYSTEM REGISTER
            } else if (addr == 0xfe) {

                // icache debug output (arg: line)
                icache->dbg_out(*ptr);
		// Setting response status
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;


            } else if (addr == 0xff) {

                // dcache debug output (arg: line)
                dcache->dbg_out(*ptr);
		// Setting response status
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;

            } else {

                v::error << name()
                         << "Address not valid for write with ASI 0x2 (or read only)"
                         << v::endl;

		// set TLM response
		tran.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
		return;
            }

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    tran.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	    return;
	    
        }
    }
    // diagnostic access to instruction PDC
    else if (asi == 0x5) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << this->name()
                    << "Diagnostic read from instruction PDC (ASI 0x5)"
                    << v::endl;

            if (m_mmu_en) {

                m_mmu->diag_read_itlb(addr, (unsigned int *)ptr);
		// set TLM response
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;

            } else {

	        v::error << name()
		       << "MMU not present" 
		       << v::endl;

  	        // set TLM response
	        tran.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
	        return;
            }

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << this->name()
                    << "Diagnostic write to instruction PDC (ASI 0x5)"
                    << v::endl;

            if (m_mmu_en) {

                m_mmu->diag_write_itlb(addr, (unsigned int *)ptr);
		// set TLM response
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;

            } else {

	        v::error << name()
		       << "MMU not present" 
		       << v::endl;

	        // set TLM response
	        tran.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
	        return;
            }
        }
    }
    // diagnostic access to data or shared instruction and data PDC
    else if (asi == 0x6) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << name()
                    << "Diagnostic read from data (or shared) PDC (ASI 0x6)"
                    << v::endl;

            if (m_mmu_en) {

                m_mmu->diag_read_dctlb(addr, (unsigned int *)ptr);
		// set TLM response
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;

            } else {

	        v::error << name()
		         << "MMU not present" 
		         << v::endl;

	        // set TLM response
	        tran.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
	        return;
            }

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name()
                    << "Diagnostic write to data (or shared) PDC (ASI 0x6)"
                    << v::endl;

            if (m_mmu_en) {

                m_mmu->diag_write_dctlb(addr, (unsigned int *)ptr);
		// set TLM response
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;

            } else {

	        v::error << name()
		         << "MMU not present" 
		         << v::endl;

  	        // set TLM response
	        tran.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
		return;

            }
	}
    }
    // access instruction cache tags
    else if (asi == 0xc) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << name() 
		    << "ASI read instruction cache tags"
                    << v::endl;

            icache->read_cache_tag((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    tran.set_response_status(tlm::TLM_OK_RESPONSE);
	    return;

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name() << "ASI write instruction cache tags"
                    << v::endl;

            icache->write_cache_tag((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    tran.set_response_status(tlm::TLM_OK_RESPONSE);
	    return;

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    tran.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	    return;
        }
    }
    // access instruction cache data
    else if (asi == 0xd) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << name() 
		    << "ASI read instruction cache entry"
                    << v::endl;

            icache->read_cache_entry((unsigned int)addr, (unsigned int*)ptr,
                    &delay);
	    
	    // set TLM response
	    tran.set_response_status(tlm::TLM_OK_RESPONSE);
	    return;

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name() 
		    << "ASI write instruction cache entry"
                    << v::endl;

            icache->write_cache_entry((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    tran.set_response_status(tlm::TLM_OK_RESPONSE);
	    return;

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    tran.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	    return;
        }
    }
    // access data cache tags
    else if (asi == 0xe) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << name() 
		    << "ASI read data cache tags" 
		    << v::endl;

            dcache->read_cache_tag((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    tran.set_response_status(tlm::TLM_OK_RESPONSE);
	    return;

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name() 
		    << "ASI write data cache tags" 
		    << v::endl;

            dcache->write_cache_tag((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    tran.set_response_status(tlm::TLM_OK_RESPONSE);
	    return;

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    tran.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	    return;

        }
    }
    // access data cache data
    else if (asi == 0xf) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << name() 
		     << "ASI read data cache entry" 
		     << v::endl;

            dcache->read_cache_entry((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    tran.set_response_status(tlm::TLM_OK_RESPONSE);
	    return;

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name() 
		     << "ASI write data cache entry" 
		     << v::endl;

            dcache->write_cache_entry((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    tran.set_response_status(tlm::TLM_OK_RESPONSE);
	    return;

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    tran.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	    return;
        }
    }
    // flush instruction cache
    else if (asi == 0x10) {

        // icache is flushed on any write with ASI 0x10
        if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name() 
		     << "ASI flush instruction cache" 
		     << v::endl;

            icache->flush(&delay, debug);

	    // set TLM response
	    tran.set_response_status(tlm::TLM_OK_RESPONSE);
	    return;

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    tran.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	    return;
        }
    }
    // flush data cache
    else if (asi == 0x11) {

        // dcache is flushed on any write with ASI 0x11
        if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name() 
		     << "ASI flush data cache" 
		     << v::endl;

            dcache->flush(&delay, debug);

	    // set TLM response
	    tran.set_response_status(tlm::TLM_OK_RESPONSE);
	    return;

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    tran.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	    return;
        }
    }
    // access MMU internal registers
    else if (asi == 0x19) {

        // check MMU present
        if (m_mmu_en == 0x1) {

            if (cmd == tlm::TLM_READ_COMMAND) {

                v::debug << name()
                         << "MMU register read with ASI 0x19 - addr:"
                         << std::hex 
			 << addr 
			 << v::endl;

                if (addr == 0x000) {

                    // MMU Control Register
                    v::debug << name() 
			    << "ASI read MMU Control Register"
                            << v::endl;

                    *(unsigned int *)ptr = m_mmu->read_mcr();
		    // set TLM response
		    tran.set_response_status(tlm::TLM_OK_RESPONSE);
		    return;

                } else if (addr == 0x100) {

                    // Context Pointer Register
                    v::debug << name()
                            << "ASI read MMU Context Pointer Register"
                            << v::endl;

                    *(unsigned int *)ptr = m_mmu->read_mctpr();
		    // set TLM response
		    tran.set_response_status(tlm::TLM_OK_RESPONSE);
		    return;

                } else if (addr == 0x200) {

                    // Context Register
                    v::debug << name() 
			     << "ASI read MMU Context Register"
                             << v::endl;

                    *(unsigned int *)ptr = m_mmu->read_mctxr();
		    // set TLM response
		    tran.set_response_status(tlm::TLM_OK_RESPONSE);
		    return;

                } else if (addr == 0x300) {

                    // Fault Status Register
                    v::debug << name()
                             << "ASI read MMU Fault Status Register" 
			     << v::endl;

                    *(unsigned int *)ptr = m_mmu->read_mfsr();
		    // set TLM response
		    tran.set_response_status(tlm::TLM_OK_RESPONSE);
		    return;

                } else if (addr == 0x400) {

                    // Fault Address Register
                    v::debug << name()
                             << "ASI read MMU Fault Address Register" 
			     << v::endl;

                    *(unsigned int *)ptr = m_mmu->read_mfar();
		    // set TLM response
		    tran.set_response_status(tlm::TLM_OK_RESPONSE);
		    return;

                } else {
                    
		    v::error << name()
                             << "Address not valid for read with ASI 0x19"
                             << v::endl;

		    // Setting TLM response
		    tran.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
		    return;

                }

            } else if (cmd == tlm::TLM_WRITE_COMMAND) {

                v::debug << name()
                         << "MMU register write with ASI 0x19 - addr:"
                         << std::hex 
		 	 << addr 
			 << v::endl;

                if (addr == 0x000) {

                    // MMU Control Register
                    v::debug << name() 
			     << "ASI write MMU Control Register"
                             << v::endl;

                    m_mmu->write_mcr((unsigned int *)ptr);
		    // set TLM response
		    tran.set_response_status(tlm::TLM_OK_RESPONSE);
		    return;

                } else if (addr == 0x100) {

                    // Context Table Pointer Register
                    v::debug << name()
                             << "ASI write MMU Context Table Pointer Register"
                             << v::endl;

                    m_mmu->write_mctpr((unsigned int*)ptr);
		    // set TLM response
		    tran.set_response_status(tlm::TLM_OK_RESPONSE);
		    return;

                } else if (addr == 0x200) {

                    // Context Register
                    v::debug << name() 
			     << "ASI write MMU Context Register"
                             << v::endl;

                    m_mmu->write_mctxr((unsigned int*)ptr);
		    // set TLM response
		    tran.set_response_status(tlm::TLM_OK_RESPONSE);
		    return;

                } else {
                    
		    v::error << name()
                             << "Address not valid for write with ASI 0x19 (or read-only)"
                             << v::endl;

		    // Setting TLM response
		    tran.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
		    return;
                }
            }
 
       } else {

            v::error << name()
                     << "Access to MMU registers, but MMU not present!"
                     << v::endl;

	    // Setting TLM response
	    tran.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
	    return;

        }
    }
    // ordinary access
    else if ((asi == 0x8) || (asi == 0x9) || (asi == 0xa) || (asi == 0xb)) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            // instruction scratchpad enabled && address points into selected 16 MB region
            if (m_ilram && (((addr >> 24) & 0xff) == m_ilramstart)) {

                ilocalram->read((unsigned int)addr, ptr, len, &delay, debug);
		// set TLM response
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;

                // data scratchpad enabled && address points into selected 16MB region
            } else if (m_dlram && (((addr >> 24) & 0xff) == m_dlramstart)) {

                dlocalram->read((unsigned int)addr, ptr, len, &delay, debug);
		// set TLM response
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;

                // cache access || bypass || direct mmu
            } else {

                dcache->mem_read((unsigned int)addr, ptr, len, &delay, debug);
		// set TLM response
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;
            }

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            // instruction scratchpad enabled && address points into selected 16 MB region
            if (m_ilram && (((addr >> 24) & 0xff) == m_ilramstart)) {

                ilocalram->write((unsigned int)addr, ptr, len, &delay, debug);
		// set TLM response
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;

            // data scratchpad enabled && address points into selected 16MB region
            } else if (m_dlram && (((addr >> 24) & 0xff) == m_dlramstart)) {

                dlocalram->write((unsigned int)addr, ptr, len, &delay, debug);
		// set TLM response
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;

            // cache access (write through) || bypass || direct mmu
            } else {

                dcache->mem_write((unsigned int)addr, ptr, len, &delay, debug);
		// set TLM response
		tran.set_response_status(tlm::TLM_OK_RESPONSE);
		return;
            }

        } else {

            v::error << name()
                     << "TLM command not valid"
                     << v::endl;

	    // Setting TLM response
	    tran.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	    return;
	}

    } else {

        v::error << name() 
		 << "ASI not recognized: " 
		 << std::hex 
		 << asi
		 << v::endl;
	
	// Setting TLM response
	tran.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
	return;
    }
}

/// TLM non-blocking forward transport function for icio socket
tlm::tlm_sync_enum mmu_cache::icio_custom_nb_transport_fw(tlm::tlm_generic_payload &gp, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << " ico_custom_nb_transport_fw @phase: " << phase << " delay: " << delay << v::endl;

  tlm::tlm_sync_enum status = tlm::TLM_COMPLETED;

  // AT coding style making use of backward path
  switch (phase) {

    case tlm::BEGIN_REQ:

      // put transaction in PEQ
      icio_PEQ.notify(gp, delay);

      // there is no accept delay
      delay = SC_ZERO_TIME;

      // advance transaction state.
      phase = tlm::END_REQ;

      // set return state
      status = tlm::TLM_UPDATED;
      break;

    case tlm::END_RESP:

      status = tlm::TLM_COMPLETED;
      break;

    default:

      // Catch END_REQ, BEGIN_REQ and not valid !
      v::error << name() << " Illegal phase in icio nb_transport_fw !" << v::endl;
      assert(0);
      break;
  }

  return(status);

}

/// Processes transactions from the icio_PEQ.
/// Contains a state machine to manage the communication path back to instruction initiator
/// Is registered as an SC_THREAD and sensitive to icio_PEQ.get_event() 
void mmu_cache::icio_service_thread() {

  // locals
  tlm::tlm_command cmd;
  sc_dt::uint64 addr;
  unsigned char* ptr;
  icio_payload_extension * iext;

  unsigned int * debug;

  sc_core::sc_time delay;

  tlm::tlm_generic_payload *tran;
  tlm::tlm_sync_enum status;

  while(1) {

    status = tlm::TLM_COMPLETED;

    // Process all icio transactions scheduled for the current time.
    // A return value of NULL indicates that the PEQ is empty at this time.

    while((tran = icio_PEQ.get_next_transaction()) != 0) {

      v::debug << name() << " Start ICIO transaction response processing." << v::endl;

      // -- Call the functional part of the model

      // extract payload
      cmd  = (*tran).get_command();
      addr = (*tran).get_address();
      ptr  = (*tran).get_data_ptr();

      // extract extension
      (*tran).get_extension(iext);

      delay = sc_core::SC_ZERO_TIME;

      debug = iext->debug;

      if (cmd == tlm::TLM_READ_COMMAND) {

	// instruction scratchpad enabled && address points into selecte 16MB region
	if (m_ilram && (((addr >> 24) & 0xff) == m_ilramstart)) {

	  ilocalram->read((unsigned int)addr, ptr, 4, &delay, debug);

	// instruction cache access
	} else {

	  icache->mem_read((unsigned int)addr, ptr, 4, &delay, debug);
    
	}

	// set response status
	(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

      } else {

	v::error << name() << " Command not valid for instruction cache (tlm_write)" << v::endl;
	// set response status
	(*tran).set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    
      }

      v::debug << name() << " Spend component delay: " << delay << v::endl;

      // burn delay
      //wait(delay);
    
      // todo: fix memory has no delay yet
      wait(20, SC_NS);

      delay = SC_ZERO_TIME;

      // -- Call backward path with phase BEGIN_RESP and check returned status

      tlm::tlm_phase phase = tlm::BEGIN_RESP;

      v::debug << name() << " Call ICIO backward transport with tlm::BEGIN_RESP." << v::endl;

      status = icio->nb_transport_bw(*tran, phase, delay);

      v::debug << name() << " Master returned status: " << status << v::endl;

      // check return status
      switch (status) {

	case tlm::TLM_COMPLETED:

	  wait(delay);
	  break;

	case tlm::TLM_ACCEPTED:

	  wait(delay);
	  break;

	default:

	  v::error << name() << " TLM return status undefined or not valid!! " << v::endl;
	  assert(0);
	  break;

      } // switch

    } // while PEQ
  
    v::debug << name() << " Wait for next event" << v::endl;
    wait();

  }  // while thread
}

/// TLM non-blocking forward transport function for dcio socket
tlm::tlm_sync_enum mmu_cache::dcio_custom_nb_transport_fw(tlm::tlm_generic_payload &gp, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  v::debug << name() << " dcio_custom_nb_transport_fw @phase: " << phase << " delay: " << delay << v::endl;

  tlm::tlm_sync_enum status = tlm::TLM_COMPLETED;

  // AT coding style making use of backward path
  switch(phase) {

    case tlm::BEGIN_REQ:

      // put transaction in PEQ
      dcio_PEQ.notify(gp, delay);

      // there is no accept delay
      delay = SC_ZERO_TIME;

      // advance transaction state
      phase = tlm::END_REQ;

      // set return state
      status = tlm::TLM_UPDATED;
      break;

    case tlm::END_RESP:

      status = tlm::TLM_COMPLETED;
      break;

    default:

      // Catch END_REQ, BEGIN_REQ and not valid !
      v::error << name() << " Illegal phase in dcio_custom_nb_transport_fw !" << v::endl;
      assert(0);
      break;

  }

  return(status);

}

/// Processes transactions from the dcio_PEQ.
/// Contains a state machine to manage the communication path back to the data initiator
/// Is registered as an SC_THREAD and sensitive to dcio_PEQ.get_event()
void mmu_cache::dcio_service_thread() {

  // locals
  tlm::tlm_command cmd;
  sc_dt::uint64 addr;
  unsigned char* ptr;
  unsigned int len;
  dcio_payload_extension * dext;

  unsigned int asi;
  unsigned int * debug;

  sc_core::sc_time delay;

  tlm::tlm_generic_payload *tran;
  tlm::tlm_sync_enum status;

  while(1) {

    status = tlm::TLM_COMPLETED;

    // Process all dcio transactions scheduled for the current time.
    // A return value of NULL indicates that the PEQ is empty at this time.

    while((tran = dcio_PEQ.get_next_transaction()) != 0) {

      v::debug << name() << " Start dcio transaction response processing." << v::endl;

      // -- Call the functional part of the model

      // extract payload
      cmd  = (*tran).get_command();
      addr = (*tran).get_address();
      ptr  = (*tran).get_data_ptr();
      len  = (*tran).get_data_length();

      // extract extension
      (*tran).get_extension(dext);

      asi = dext->asi;
      debug = dext->debug;    

      delay = SC_ZERO_TIME;

      // access system registers
      if (asi == 2) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << this->name()
                     << "System Registers read with ASI 0x2 - addr:" << std::hex
                     << addr << v::endl;

            if (addr == 0) {

                // cache control register
                *(unsigned int *)ptr = read_ccr();
		// Setting response status
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

            } else if (addr == 8) {

                // instruction cache configuration register
                *(unsigned int *)ptr = icache->read_config_reg(&delay);
		// Setting response status
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

            } else if (addr == 0x0c) {

                // data cache configuration register
                *(unsigned int *)ptr = dcache->read_config_reg(&delay);
		// Setting response status
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

            } else {

                v::error << name()
                         << "Address not valid for read with ASI 0x2" 
			 << v::endl;
		
		// set TLM response
		(*tran).set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            }

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

	    v::debug << this->name()
                     << "System Register write with ASI 0x2 - addr:" << std::hex
                     << addr << v::endl;

            if (addr == 0) {

                // cache control register
                write_ccr(ptr, len, &delay);
		// Setting response status
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

            // TRIGGER DEBUG OUTPUT / NOT A SPARC SYSTEM REGISTER
            } else if (addr == 0xfe) {

                // icache debug output (arg: line)
                icache->dbg_out(*ptr);
		// Setting response status
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);


            } else if (addr == 0xff) {

                // dcache debug output (arg: line)
                dcache->dbg_out(*ptr);
		// Setting response status
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

            } else {

                v::error << name()
                         << "Address not valid for write with ASI 0x2 (or read only)"
                         << v::endl;

		// set TLM response
		(*tran).set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            }

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	    
        }
      }
      // diagnostic access to instruction PDC
      else if (asi == 0x5) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << this->name()
                     << "Diagnostic read from instruction PDC (ASI 0x5)"
                     << v::endl;

            if (m_mmu_en) {

                m_mmu->diag_read_itlb(addr, (unsigned int *)ptr);
		// set TLM response
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

            } else {

	        v::error << name()
		       << "MMU not present" 
		       << v::endl;

  	        // set TLM response
	        (*tran).set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            }

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << this->name()
                     << "Diagnostic write to instruction PDC (ASI 0x5)"
                     << v::endl;

            if (m_mmu_en) {

                m_mmu->diag_write_itlb(addr, (unsigned int *)ptr);
		// set TLM response
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

            } else {

	        v::error << name()
		       << "MMU not present" 
		       << v::endl;

	        // set TLM response
	        (*tran).set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            }
        }
      }
      // diagnostic access to data or shared instruction and data PDC
      else if (asi == 0x6) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << name()
                     << "Diagnostic read from data (or shared) PDC (ASI 0x6)"
                     << v::endl;

            if (m_mmu_en) {

                m_mmu->diag_read_dctlb(addr, (unsigned int *)ptr);
		// set TLM response
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

            } else {

	        v::error << name()
		         << "MMU not present" 
		         << v::endl;

	        // set TLM response
	        (*tran).set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            }

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name()
                     << "Diagnostic write to data (or shared) PDC (ASI 0x6)"
                     << v::endl;

            if (m_mmu_en) {

                m_mmu->diag_write_dctlb(addr, (unsigned int *)ptr);
		// set TLM response
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

            } else {

	        v::error << name()
		         << "MMU not present" 
		         << v::endl;

  	        // set TLM response
	        (*tran).set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);

            }
	}
      }
      // access instruction cache tags
      else if (asi == 0xc) {
      
        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << name() 
		     << "ASI read instruction cache tags"
                     << v::endl;

            icache->read_cache_tag((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name() << "ASI write instruction cache tags"
                     << v::endl;

            icache->write_cache_tag((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
        }
      }
      // access instruction cache data
      else if (asi == 0xd) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << name() 
		     << "ASI read instruction cache entry"
                     << v::endl;

            icache->read_cache_entry((unsigned int)addr, (unsigned int*)ptr,
                    &delay);
	    
	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name() 
		     << "ASI write instruction cache entry"
                     << v::endl;

            icache->write_cache_entry((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
        }
      }
      // access data cache tags
      else if (asi == 0xe) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << name() 
		     << "ASI read data cache tags" 
		     << v::endl;

            dcache->read_cache_tag((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name() 
		     << "ASI write data cache tags" 
		     << v::endl;

            dcache->write_cache_tag((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);

        }
      }
      // access data cache data
      else if (asi == 0xf) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            v::debug << name() 
		     << "ASI read data cache entry" 
		     << v::endl;

            dcache->read_cache_entry((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name() 
		     << "ASI write data cache entry" 
		     << v::endl;

            dcache->write_cache_entry((unsigned int)addr, (unsigned int*)ptr,
                    &delay);

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
        }
      }
      // flush instruction cache
      else if (asi == 0x10) {

        // icache is flushed on any write with ASI 0x10
        if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name() 
		     << "ASI flush instruction cache" 
		     << v::endl;

            icache->flush(&delay, debug);

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
        }
      }
      // flush data cache
      else if (asi == 0x11) {

        // dcache is flushed on any write with ASI 0x11
        if (cmd == tlm::TLM_WRITE_COMMAND) {

            v::debug << name() 
		     << "ASI flush data cache" 
		     << v::endl;

            dcache->flush(&delay, debug);

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

        } else {

            v::error << name() 
		     << "Unvalid TLM Command" 
		     << v::endl;

	    // set TLM response
	    (*tran).set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
        }
      }
      // access MMU internal registers
      else if (asi == 0x19) {

        // check MMU present
        if (m_mmu_en == 0x1) {

            if (cmd == tlm::TLM_READ_COMMAND) {

                v::debug << name()
                         << "MMU register read with ASI 0x19 - addr:"
                         << std::hex 
			 << addr 
			 << v::endl;

                if (addr == 0x000) {

                    // MMU Control Register
                    v::debug << name() 
			     << "ASI read MMU Control Register"
                             << v::endl;

                    *(unsigned int *)ptr = m_mmu->read_mcr();
		    // set TLM response
		    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

                } else if (addr == 0x100) {

                    // Context Pointer Register
                    v::debug << name()
                             << "ASI read MMU Context Pointer Register"
                             << v::endl;

                    *(unsigned int *)ptr = m_mmu->read_mctpr();
		    // set TLM response
		    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

                } else if (addr == 0x200) {

                    // Context Register
                    v::debug << name() 
			     << "ASI read MMU Context Register"
                             << v::endl;

                    *(unsigned int *)ptr = m_mmu->read_mctxr();
		    // set TLM response
		    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

                } else if (addr == 0x300) {

                    // Fault Status Register
                    v::debug << name()
                             << "ASI read MMU Fault Status Register" 
			     << v::endl;

                    *(unsigned int *)ptr = m_mmu->read_mfsr();
		    // set TLM response
		    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

                } else if (addr == 0x400) {

                    // Fault Address Register
                    v::debug << name()
                             << "ASI read MMU Fault Address Register" 
			     << v::endl;

                    *(unsigned int *)ptr = m_mmu->read_mfar();
		    // set TLM response
		    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

                } else {
                    
		    v::error << name()
                             << "Address not valid for read with ASI 0x19"
                             << v::endl;

		    // Setting TLM response
		    (*tran).set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);

                }

            } else if (cmd == tlm::TLM_WRITE_COMMAND) {

                v::debug << name()
                         << "MMU register write with ASI 0x19 - addr:"
                         << std::hex 
			 << addr 
			 << v::endl;

                if (addr == 0x000) {

                    // MMU Control Register
                    v::debug << name() 
			    << "ASI write MMU Control Register"
                            << v::endl;

                    m_mmu->write_mcr((unsigned int *)ptr);
		    // set TLM response
		    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

                } else if (addr == 0x100) {

                    // Context Table Pointer Register
                    v::debug << name()
                             << "ASI write MMU Context Table Pointer Register"
                             << v::endl;

                    m_mmu->write_mctpr((unsigned int*)ptr);
		    // set TLM response
		    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

                } else if (addr == 0x200) {

                    // Context Register
                    v::debug << name() 
			     << "ASI write MMU Context Register"
                             << v::endl;

                    m_mmu->write_mctxr((unsigned int*)ptr);
		    // set TLM response
		    (*tran).set_response_status(tlm::TLM_OK_RESPONSE);

                } else {
                    
		    v::error << name()
                             << "Address not valid for write with ASI 0x19 (or read-only)"
                             << v::endl;

		    // Setting TLM response
		    (*tran).set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
                }
            }
 
	} else {

            v::error << name()
                     << "Access to MMU registers, but MMU not present!"
                     << v::endl;

	    // Setting TLM response
	    (*tran).set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);

        }
      }
      // ordinary access
      else if ((asi == 0x8) || (asi == 0x9) || (asi == 0xa) || (asi == 0xb)) {

        if (cmd == tlm::TLM_READ_COMMAND) {

            // instruction scratchpad enabled && address points into selected 16 MB region
            if (m_ilram && (((addr >> 24) & 0xff) == m_ilramstart)) {

                ilocalram->read((unsigned int)addr, ptr, len, &delay, debug);
		// set TLM response
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

                // data scratchpad enabled && address points into selected 16MB region
            } else if (m_dlram && (((addr >> 24) & 0xff) == m_dlramstart)) {

                dlocalram->read((unsigned int)addr, ptr, len, &delay, debug);
		// set TLM response
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

                // cache access || bypass || direct mmu
            } else {

                dcache->mem_read((unsigned int)addr, ptr, len, &delay, debug);
		// set TLM response
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);
            }

        } else if (cmd == tlm::TLM_WRITE_COMMAND) {

            // instruction scratchpad enabled && address points into selected 16 MB region
            if (m_ilram && (((addr >> 24) & 0xff) == m_ilramstart)) {

                ilocalram->write((unsigned int)addr, ptr, len, &delay, debug);
		// set TLM response
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

            // data scratchpad enabled && address points into selected 16MB region
            } else if (m_dlram && (((addr >> 24) & 0xff) == m_dlramstart)) {

                dlocalram->write((unsigned int)addr, ptr, len, &delay, debug);
		// set TLM response
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);

            // cache access (write through) || bypass || direct mmu
            } else {

                dcache->mem_write((unsigned int)addr, ptr, len, &delay, debug);
		// set TLM response
		(*tran).set_response_status(tlm::TLM_OK_RESPONSE);
            }

        } else {

            v::error << name()
                     << "TLM command not valid"
                     << v::endl;

	    // Setting TLM response
	    (*tran).set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
	}
	
      } else {

        v::error << name() 
		 << "ASI not recognized: " 
		 << std::hex 
		 << asi
		 << v::endl;
	
	// Setting TLM response
	(*tran).set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
      }

      v::debug << name() << " Spend component delay: " << delay << v::endl;

      // burn delay
      //wait(delay);

      // todo: fix memory delay
      wait(20,SC_NS);
      
      delay = SC_ZERO_TIME;

      // Call backward path with phase BEGIN_RESP and check returned status

      tlm::tlm_phase phase = tlm::BEGIN_RESP;
      delay = sc_core::SC_ZERO_TIME;

      v::debug << name() << " DCIO backward transport with tlm::BEGIN_RESP>" << v::endl;

      status = dcio->nb_transport_bw(*tran, phase, delay);

      v::debug << name() << " Master returned status: " << status << v::endl;

      // check return status
      switch (status) {

        case tlm::TLM_COMPLETED:

	  wait(delay);
	  break;

        case tlm::TLM_ACCEPTED:

	  wait(delay);
	  break;

        default:

	  v::error << name() << " TLM return status undefined or not valid!! " << v::endl;
	  break;
 
      } // switch
    
    } // while PEQ

    wait();

  } // while thread     
}

/// TLM non-blocking backward transport function for ahb_master socket
tlm::tlm_sync_enum mmu_cache::ahb_custom_nb_transport_bw(tlm::tlm_generic_payload &payload, tlm::tlm_phase &phase, sc_core::sc_time &delay) {

  tlm::tlm_sync_enum status = tlm::TLM_COMPLETED;

  v::debug << name() << " ahb_custom_nb_transport_bw @phase: " << phase << v::endl;

  switch (phase) {

    case tlm::BEGIN_RESP:

      status = tlm::TLM_ACCEPTED;
      
      // response notification
      ahb_transaction_response.notify();
      break;

    default:

      v::error << name() << " TLM phase received on backward path not valid" << v::endl;
      assert(0);
      break;
  }

  return(status);

}

/// Function for write access to AHB master socket
void mmu_cache::mem_write(unsigned int addr, unsigned char * data,
                          unsigned int len, sc_core::sc_time * t,
                          unsigned int * debug) {


    // init transaction
    tlm::tlm_generic_payload *gp = ahb_master.get_transaction();
    gp->set_command(tlm::TLM_WRITE_COMMAND);
    gp->set_address(addr);
    gp->set_data_length(len);
    gp->set_data_ptr(data);
    gp->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    // set the burst size
    amba::amba_burst_size* size_ext;
    ahb_master.validate_extension<amba::amba_burst_size> (*gp);
    ahb_master.get_extension<amba::amba_burst_size> (size_ext, *gp);
    size_ext->value = (len < 4)? len : 4;

    // set the id of the master
    amba::amba_id* m_id;
    ahb_master.get_extension<amba::amba_id> (m_id, *gp);
    m_id->value = m_master_id;
    ahb_master.validate_extension<amba::amba_id> (*gp);

    sc_core::sc_time delay = SC_ZERO_TIME;

    v::debug << name() << "AHB write to addr: " << hex << addr << v::endl;

    if (m_abstractionLevel == amba::amba_LT) {

      // blocking transport
      ahb_master->b_transport(*gp, delay);

    } else {

      // initial phase for AT
      tlm::tlm_phase phase = tlm::BEGIN_REQ;

      // non-blocking transport
      tlm::tlm_sync_enum status = ahb_master->nb_transport_fw(*gp,phase,delay);
      assert(status==tlm::TLM_COMPLETED);

      // wait for service thread
      if (status==tlm::TLM_COMPLETED) {

	// backward path not used for write

	//wait(ahb_transaction_response);

      }

    } 

    v::debug << name() << "AHB transaction done (mem_write) - delay: " << delay << v::endl;

    // increment time
    //*t += delay;

    // temp hack: memory has no delay yet
    *t += sc_core::sc_time(10, SC_NS);

    // release transaction
    ahb_master.release_transaction(gp);

}

/// Function for read access to AHB master socket
bool mmu_cache::mem_read(unsigned int addr, unsigned char * data,
                         unsigned int len, sc_core::sc_time * t,
                         unsigned int * debug) {

    // tmp
    bool cacheable;

     // init transaction
    tlm::tlm_generic_payload *gp = ahb_master.get_transaction();
    gp->set_command(tlm::TLM_READ_COMMAND);
    gp->set_address(addr);
    gp->set_data_length(len);
    gp->set_data_ptr(data);
    gp->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    // set the burst size
    amba::amba_burst_size* size_ext;
    ahb_master.validate_extension<amba::amba_burst_size> (*gp);
    ahb_master.get_extension<amba::amba_burst_size> (size_ext, *gp);
    size_ext->value = (len < 4)? len : 4;

    // set the id of the master
    amba::amba_id* m_id;
    ahb_master.get_extension<amba::amba_id> (m_id, *gp);
    m_id->value = m_master_id;
    ahb_master.validate_extension<amba::amba_id> (*gp);

    sc_core::sc_time delay = SC_ZERO_TIME;

    v::debug << name() << " AHB read from addr: " << hex << addr << v::endl;

    if (m_abstractionLevel == amba::amba_LT) {

      // blocking transport
      ahb_master->b_transport(*gp, delay);

    } else {

      // initial phase for AT
      tlm::tlm_phase phase = tlm::BEGIN_REQ;
      
      // non-blocking transport
      tlm::tlm_sync_enum status = ahb_master->nb_transport_fw(*gp,phase,delay);
      assert((status==tlm::TLM_UPDATED)||(status==tlm::TLM_ACCEPTED)||(status==tlm::TLM_COMPLETED));

      // @AT level mem_read will be indirectly called from icio/dcio_service_thread
      if (status==tlm::TLM_UPDATED) {
	
	wait(ahb_transaction_response);
      
      } else if (status==tlm::TLM_ACCEPTED) {

	// not really correct - but ok for this testbench
	wait(ahb_transaction_response);

      } else {

	// completed - nothing to do

      }

    }

    v::debug << name() << " AHB transaction done (mem_read) - delay: " << delay << v::endl;

    // increment time
    //*t += delay;

    // temp hack: memory has no delay yet
    *t += sc_core::sc_time(10, SC_NS);

    // Check cacheability:
    // -------------------
    // Cacheable areas are defined by the 'cached' parameter.
    // In case 'cached' is zero, plug & play information will be used.
    if (m_cached != 0) {

	// use fixed mask
	cacheable = (m_cached & (1 << (addr >> 28))) ? true : false;

    } else {

	// use PNP - information is carried by protection extension
	cacheable = (ahb_master.get_extension<amba::amba_cacheable>(*gp)) ? true : false;
	
    }    

    // release transaction
    ahb_master.release_transaction(gp);


    // return cacheability state
    return cacheable;
}

/// writes the cache control register and handles the commands
void mmu_cache::write_ccr(unsigned char * data, unsigned int len,
                          sc_core::sc_time * delay) {

    unsigned int tmp = 0;
    unsigned int dummy;

    memcpy(&tmp, data, len);

    // [DS] data cache snoop enable (todo)
    if (tmp & (1 << 23)) {
    }
    // [FD] dcache flush (do not set; always reads as zero)
    if (tmp & (1 << 22)) {
        dcache->flush(delay, &dummy);
    }
    // [FI] icache flush (do not set; always reads as zero)
    if (tmp & (1 << 21)) {
        icache->flush(delay, &dummy);
    }
    // [IB] instruction burst fetch (todo)
    if (tmp & (1 << 16)) {
    }

    // [IP] Instruction cache flush pending (bit 15 - read only)
    // [DP] Data cache flush pending (bit 14 - read only)

    // [DF] data cache freeze on interrupt (todo)
    if (tmp & (1 << 5)) {
    }

    // [IF] instruction cache freeze on interrupt (todo)
    if (tmp & (1 << 4)) {
    }

    // [DCS] data cache state (bits 3:2 - read only)
    // [ICS] instruction cache state (bits 1:0 - read only)

    // read only masking: 1111 1111 1001 1111 0011 1111 1111 1111
    CACHE_CONTROL_REG = (tmp & 0xff9f3fff);
}

// read the cache control register
unsigned int mmu_cache::read_ccr() {

    return (CACHE_CONTROL_REG);
}

// Snooping function
void mmu_cache::snoopingCallBack(const t_snoop& snoop, const sc_core::sc_time& delay) {

  v::debug << name() << "Snooping write operation on AHB interface (MASTER: " << snoop.master_id << " ADDR: " \
	   << hex << snoop.address << " LENGTH: " << snoop.length << ")" << v::endl;

  // Make sure we are not snooping ourself ;)
  if (snoop.master_id != m_master_id) {

    // If dcache and snooping enabled
    if (m_dcen && m_dsnoop) {

      dcache->snoop_invalidate(snoop, delay);

    }
  }
}

// Helper for setting clock cycle latency using sc_clock argument
void mmu_cache::clk(sc_core::sc_clock &clk) {

  // Set local clock
  clockcycle = clk.period();

  // Set icache clock
  if (m_icen) { 
    
    icache->clk(clk);

  }

  // Set dcache clock
  if (m_dcen) {

    dcache->clk(clk);

  }

  // Set mmu clock
  if (m_mmu_en) {

    m_mmu->clk(clk);

  }

}

// Helper for setting clock cycle latency using sc_time argument
void mmu_cache::clk(sc_core::sc_time &period) {

  // Set local clock
  clockcycle = period;

  // Set icache clock
  if (m_icen) { 
    
    icache->clk(period);

  }

  // Set dcache clock
  if (m_dcen) {

    dcache->clk(period);

  }

  // Set mmu clock
  if (m_mmu_en) {

    m_mmu->clk(period);

  }

}

// Helper for setting clock cycle latency using a value-time_unit pair
void mmu_cache::clk(double period, sc_core::sc_time_unit base) {

  // Set local clock
  clockcycle = sc_core::sc_time(period, base);

  // Set icache clock
  if (m_icen) { 
    
    icache->clk(period, base);

  }

  // Set dcache clock
  if (m_dcen) {

    dcache->clk(period, base);

  }

  // Set mmu clock
  if (m_mmu_en) {

    m_mmu->clk(period, base);

  }

}
