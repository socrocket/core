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
// The program is provided "as is", there is no warranty that
// the program is correct or suitable for any purpose,
// neither implicit nor explicit. The program and the information in it
// contained do not necessarily reflect the policy of the 
// European Space Agency or of TU-Braunschweig.
//*********************************************************************
// Title:      gaisler_record_types.h
//
// ScssId:
//
// Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
//
// Purpose:    defines struct data types for connection with gaisler IP
//
// Method:
//
// Modified on $Date$
//          at $Revision$
//          by $Author: HWSWSIM $
//
// Principal:  European Space Agency
// Author:     VLSI working group @ IDA @ TUBS
// Maintainer: Dennis Bode
// Reviewed:
//*********************************************************************

#ifndef _GAISLER_RECORD_TYPES_
#define _GAISLER_RECORD_TYPES_

#include "systemc.h"

struct memory_in_type {
    sc_uint<32> data;
    bool brdyn;
    bool bexcn;
    bool writen;
    sc_uint<4> wrn;
    sc_uint<2> bwidth;
    sc_uint<64> sd;
    sc_uint<16> cb;
    sc_uint<16> scb;
    bool edac;
};

inline ostream& operator<<(ostream& os, const memory_in_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const memory_in_type&, const std::string &) {}

inline int operator== (const memory_in_type& left, const memory_in_type& right) {
    return 0;
}
/************************************/

struct memory_out_type {
    sc_uint<32> address;
    sc_uint<32> data;
    sc_uint<64> sddata;
    sc_uint<8> ramsn;
    sc_uint<8> ramoen;
    bool ramn;
    bool romn;
    sc_uint<4> mben;
    bool iosn;
    sc_uint<8> romsn;
    bool oen;
    bool writen;
    sc_uint<4> wrn;
    sc_uint<4> bdrive;
    sc_uint<32> vbdrive;
    sc_uint<64> svbdrive;
    bool mread;
    sc_uint<15> sa;
    sc_uint<16> cb;
    sc_uint<16> scb;
    sc_uint<16> vcdrive;
    sc_uint<16> svcdrive;
    bool ce;
    bool sdram_en;
    bool rs_edac_en;
};

inline bool read_oen(const memory_out_type& a) {
    return a.oen;
}

inline ostream& operator<<(ostream& os, const memory_out_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const memory_out_type&, const std::string &) {}

inline int operator== (const memory_out_type& left, const memory_out_type& right) {
    return 0;
}
/************************************/

struct ahb_addr_type {
};

inline ostream& operator<<(ostream& os, const ahb_addr_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const ahb_addr_type&, const std::string &) {}

inline int operator== (const ahb_addr_type& left, const ahb_addr_type& right) {
    return 0;
}

/************************************/

struct ahb_mst_in_type {
    sc_uint<16> hgrant;
    bool hready;
    sc_uint<2> hresp;
    sc_uint<32> hrdata;
    bool hcache;
    sc_uint<32> hirq;
    bool testen;
    bool testrst;
    bool scanen;
    bool testoen;
};

inline ostream& operator<<(ostream& os, const ahb_mst_in_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const ahb_mst_in_type&, const std::string &) {}

inline int operator== (const ahb_mst_in_type& left, const ahb_mst_in_type& right) {
    return 0;
}

/************************************/

struct ahb_mst_out_type {
    bool hbusreq;
    bool hlock;
    sc_uint<2> htrans;
    sc_uint<32> haddr;
    bool hwrite;
    sc_uint<3> hsize;
    sc_uint<3> hburst;
    sc_uint<4> hprot;
    sc_uint<32> hwdata;
    sc_uint<32> hirq;
    sc_uint<32> hconfig[8];
    int hindex;
};

inline ostream& operator<<(ostream& os, const ahb_mst_out_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const ahb_mst_out_type&, const std::string &) {}

inline int operator== (const ahb_mst_out_type& left, const ahb_mst_out_type& right) {
    return 0;
}

/************************************/

struct ahb_slv_in_type {
    sc_uint<16> hsel;
    sc_uint<32> haddr;
    bool hwrite;
    sc_uint<2> htrans;
    sc_uint<3> hsize;
    sc_uint<3> hburst;
    sc_uint<32> hwdata;
    sc_uint<4> hprot;
    bool hready;
    sc_uint<4> hmaster;
    bool hmastlock;
    sc_uint<4> hmbsel;
    bool hcache;
    sc_uint<32> hirq;
    bool testen;
    bool testrst;
    bool scanen;
    bool testoen;
};

inline ostream& operator<<(ostream& os, const ahb_slv_in_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const ahb_slv_in_type&, const std::string &) {}

inline int operator== (const ahb_slv_in_type& left, const ahb_slv_in_type& right) {
    return 0;
}

/************************************/

struct ahb_slv_out_type {
    bool hready;
    sc_uint<2> hresp;
    sc_uint<32> hrdata;
    sc_uint<16> hsplit;
    bool hcache;
    sc_uint<32> hirq;
    sc_uint<32> hconfig[8];
    int hindex;
};

inline ostream& operator<<(ostream& os, const ahb_slv_out_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const ahb_slv_out_type&, const std::string &) {}

inline int operator== (const ahb_slv_out_type& left, const ahb_slv_out_type& right) {
    return 0;
}
/************************************/

struct apb_slv_in_type {
    sc_uint<16> psel;
    bool penable;
    sc_uint<32> paddr;
    bool pwrite;
    sc_uint<32> pwdata;
    sc_uint<32> pirq;
    bool testen;
    bool testrst;
    bool scanen;
    bool testoen;
};

inline ostream& operator<<(ostream& os, const apb_slv_in_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const apb_slv_in_type&, const std::string &) {}

inline int operator== (const apb_slv_in_type& left, const apb_slv_in_type& right) {
    return 0;
}
/************************************/

struct apb_slv_out_type {
    sc_uint<32> prdata;
    sc_uint<32> pirq;
    sc_uint<32> pconfig[2];
    int pindex;
};

inline ostream& operator<<(ostream& os, const apb_slv_out_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const apb_slv_out_type&, const std::string &) {}

inline int operator== (const apb_slv_out_type& left, const apb_slv_out_type& right) {
    return 0;
}
/************************************/

struct wprot_out_type {
    bool wprothit;
};

inline ostream& operator<<(ostream& os, const wprot_out_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const wprot_out_type&, const std::string &) {}

inline int operator== (const wprot_out_type& left, const wprot_out_type& right) {
    return 0;
}
/************************************/

struct sdram_out_type {
    sc_uint<2> sdcke;
    sc_uint<2> sdcsn;
    bool sdwen;
    bool rasn;
    bool casn;
    sc_uint<8> dqm;
};

inline ostream& operator<<(ostream& os, const sdram_out_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const sdram_out_type&, const std::string &) {}

inline int operator== (const sdram_out_type& left, const sdram_out_type& right) {
    return 0;
}
#endif
