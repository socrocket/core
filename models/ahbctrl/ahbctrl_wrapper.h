#ifndef _SCGENMOD_ahbctrl_wrapper_
#define _SCGENMOD_ahbctrl_wrapper_

#include "systemc.h"

#ifndef _AHB_MST_IN_TYPE_
#define _AHB_MST_IN_TYPE_

struct ahb_mst_in_type {
    sc_lv<16> hgrant;
    sc_logic hready;
    sc_lv<2> hresp;
    sc_lv<32> hrdata;
    sc_logic hcache;
    sc_lv<32> hirq;
    sc_logic testen;
    sc_logic testrst;
    sc_logic scanen;
    sc_logic testoen;
};

inline ostream& operator<<(ostream& os, const ahb_mst_in_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const ahb_mst_in_type&, const std::string &) {}

inline int operator== (const ahb_mst_in_type& left, const ahb_mst_in_type& right) {
    return 0;
}

#endif // _AHB_MST_IN_TYPE

/************************************/

#ifndef _AHB_MST_OUT_TYPE_ADAPTER_
#define _AHB_MST_OUT_TYPE_ADAPTER_

struct ahb_mst_out_type_adapter {
    sc_logic hbusreq;
    sc_logic hlock;
    sc_lv<2> htrans;
    sc_lv<32> haddr;
    sc_logic hwrite;
    sc_lv<3> hsize;
    sc_lv<3> hburst;
    sc_lv<4> hprot;
    sc_lv<32> hwdata;
    sc_lv<32> hirq;
    sc_lv<32> hconfig[8];
    int hindex;
};

inline ostream& operator<<(ostream& os, const ahb_mst_out_type_adapter& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const ahb_mst_out_type_adapter&, const std::string &) {}

inline int operator== (const ahb_mst_out_type_adapter& left, const ahb_mst_out_type_adapter& right) {
    return 0;
}

#endif // _AHB_MST_OUT_TYPE_ADAPTER_

/************************************/

#ifndef _AHB_SLV_IN_TYPE_
#define _AHB_SLV_IN_TYPE_

struct ahb_slv_in_type {
    sc_lv<16> hsel;
    sc_lv<32> haddr;
    sc_logic hwrite;
    sc_lv<2> htrans;
    sc_lv<3> hsize;
    sc_lv<3> hburst;
    sc_lv<32> hwdata;
    sc_lv<4> hprot;
    sc_logic hready;
    sc_lv<4> hmaster;
    sc_logic hmastlock;
    sc_lv<4> hmbsel;
    sc_logic hcache;
    sc_lv<32> hirq;
    sc_logic testen;
    sc_logic testrst;
    sc_logic scanen;
    sc_logic testoen;
};

inline ostream& operator<<(ostream& os, const ahb_slv_in_type& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const ahb_slv_in_type&, const std::string &) {}

inline int operator== (const ahb_slv_in_type& left, const ahb_slv_in_type& right) {
    return 0;
}

#endif // _AHB_MST_OUT_TYPE_ADAPTER_

/************************************/

#ifndef _AHB_SLV_OUT_TYPE_ADAPTER_
#define _AHB_SLV_OUT_TYPE_ADAPTER_

struct ahb_slv_out_type_adapter {
    sc_logic hready;
    sc_lv<2> hresp;
    sc_lv<32> hrdata;
    sc_lv<16> hsplit;
    sc_logic hcache;
    sc_lv<32> hirq;
    sc_lv<32> hconfig[8];
    int hindex;
};

inline ostream& operator<<(ostream& os, const ahb_slv_out_type_adapter& a) {
    return os;
}

inline void sc_trace(sc_trace_file *, const ahb_slv_out_type_adapter&, const std::string &) {}

inline int operator== (const ahb_slv_out_type_adapter& left, const ahb_slv_out_type_adapter& right) {
    return 0;
}

#endif // _AHB_SLV_OUT_TYPE_ADAPTER_

/************************************/

class ahbctrl_wrapper : public sc_foreign_module
{
public:
    sc_in<sc_logic> rst;
    sc_in<sc_logic> clk;
    sc_out<ahb_mst_in_type> msti;
    sc_in<ahb_mst_out_type_adapter> msto0;
    sc_in<ahb_mst_out_type_adapter> msto1;
    sc_in<ahb_mst_out_type_adapter> msto2;
    sc_in<ahb_mst_out_type_adapter> msto3;
    sc_in<ahb_mst_out_type_adapter> msto4;
    sc_in<ahb_mst_out_type_adapter> msto5;
    sc_in<ahb_mst_out_type_adapter> msto6;
    sc_in<ahb_mst_out_type_adapter> msto7;
    sc_in<ahb_mst_out_type_adapter> msto8;
    sc_in<ahb_mst_out_type_adapter> msto9;
    sc_in<ahb_mst_out_type_adapter> mstoa;
    sc_in<ahb_mst_out_type_adapter> mstob;
    sc_in<ahb_mst_out_type_adapter> mstoc;
    sc_in<ahb_mst_out_type_adapter> mstod;
    sc_in<ahb_mst_out_type_adapter> mstoe;
    sc_in<ahb_mst_out_type_adapter> mstof;
    sc_out<ahb_slv_in_type> slvi;
    sc_in<ahb_slv_out_type_adapter> slvo0;
    sc_in<ahb_slv_out_type_adapter> slvo1;
    sc_in<ahb_slv_out_type_adapter> slvo2;
    sc_in<ahb_slv_out_type_adapter> slvo3;
    sc_in<ahb_slv_out_type_adapter> slvo4;
    sc_in<ahb_slv_out_type_adapter> slvo5;
    sc_in<ahb_slv_out_type_adapter> slvo6;
    sc_in<ahb_slv_out_type_adapter> slvo7;
    sc_in<ahb_slv_out_type_adapter> slvo8;
    sc_in<ahb_slv_out_type_adapter> slvo9;
    sc_in<ahb_slv_out_type_adapter> slvoa;
    sc_in<ahb_slv_out_type_adapter> slvob;
    sc_in<ahb_slv_out_type_adapter> slvoc;
    sc_in<ahb_slv_out_type_adapter> slvod;
    sc_in<ahb_slv_out_type_adapter> slvoe;
    sc_in<ahb_slv_out_type_adapter> slvof;


    ahbctrl_wrapper(sc_module_name nm, const char* hdl_name)
     : sc_foreign_module(nm),
       rst("rst"),
       clk("clk"),
       msti("msti"),
       msto0("msto0"),
       msto1("msto1"),
       msto2("msto2"),
       msto3("msto3"),
       msto4("msto4"),
       msto5("msto5"),
       msto6("msto6"),
       msto7("msto7"),
       msto8("msto8"),
       msto9("msto9"),
       mstoa("mstoa"),
       mstob("mstob"),
       mstoc("mstoc"),
       mstod("mstod"),
       mstoe("mstoe"),
       mstof("mstof"),
       slvi("slvi"),
       slvo0("slvo0"),
       slvo1("slvo1"),
       slvo2("slvo2"),
       slvo3("slvo3"),
       slvo4("slvo4"),
       slvo5("slvo5"),
       slvo6("slvo6"),
       slvo7("slvo7"),
       slvo8("slvo8"),
       slvo9("slvo9"),
       slvoa("slvoa"),
       slvob("slvob"),
       slvoc("slvoc"),
       slvod("slvod"),
       slvoe("slvoe"),
       slvof("slvof")
    {
        elaborate_foreign_module(hdl_name);
    }
    ~ahbctrl_wrapper()
    {}

};

#endif

