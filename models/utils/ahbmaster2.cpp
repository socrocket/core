#include "ahbmaster2.h"

using namespace std;
using namespace sc_core;
using namespace tlm;

// Constructor (sc_module version)
template<>
AHBMaster<sc_module>::AHBMaster(sc_module_name nm,
                     uint8_t hindex,
                     uint8_t vendor,
                     uint8_t device,
                     uint8_t version,
                     uint8_t irq,
                     amba::amba_layer_ids ambaLayer,
                     uint32_t bar0,
                     uint32_t bar1,
                     uint32_t bar2,
                     uint32_t bar3) :
  sc_module(nm),
  AHBDevice(hindex, 
            vendor, 
            device, 
            version, 
            irq, 
            bar0, 
            bar1,
            bar2,
            bar3),
  ahb("ahb", ::amba::amba_AHB, ambaLayer, false),
  m_ResponsePEQ("ResponsePEQ"),
  m_ambaLayer(ambaLayer) {

  if (ambaLayer == amba::amba_AT) {

    // Register backward transport function
    ahb.register_nb_transport_bw(this, &AHBMaster::nb_transport_bw);

    // Thread for response processing (read)
    SC_THREAD(ResponseThread);

  }

  clock_cycle = sc_core::sc_time(10, sc_core::SC_NS);

}

// Constructor (gr_device version)
template<>
AHBMaster<gs::reg::gr_device>::AHBMaster(sc_module_name nm,
                     uint8_t hindex,
                     uint8_t vendor,
                     uint8_t device,
                     uint8_t version,
                     uint8_t irq,
                     amba::amba_layer_ids ambaLayer,
                     uint32_t bar0,
                     uint32_t bar1,
                     uint32_t bar2,
                     uint32_t bar3) :

  gr_device(nm, gs::reg::ALIGNED_ADDRESS, 16, NULL),
  AHBDevice(hindex, 
            vendor, 
            device, 
            version, 
            irq, 
            bar0, 
            bar1,
            bar2,
            bar3),
  ahb("ahb", ::amba::amba_AHB, ambaLayer, false),
  m_ResponsePEQ("ResponsePEQ"),
  m_ambaLayer(ambaLayer) {

  if (ambaLayer == amba::amba_AT) {

    // Register backward transport function
    ahb.register_nb_transport_bw(this, &AHBMaster::nb_transport_bw);

    // Thread for response processing (read)
    SC_THREAD(ResponseThread);

  }

  clock_cycle = sc_core::sc_time(10, sc_core::SC_NS);
}
