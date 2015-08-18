// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file apbslave.h
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef CORE_COMMON_APBSLAVE_H_
#define CORE_COMMON_APBSLAVE_H_ 

#include "core/common/systemc.h"
#include "core/common/sr_register.h"
#include "core/common/apbdevicebase.h"
#include "core/common/apbdevice.h"

template<unsigned int BUSWIDTH = 32, typename ADDR_TYPE = unsigned int, typename DATA_TYPE = unsigned int>
class sr_register_amba_socket : public ::amba::amba_slave_socket<BUSWIDTH>, public ::amba_slave_base {
  public:
    sr_register_amba_socket(sc_core::sc_module_name mn,
      sc_register_bank<ADDR_TYPE, DATA_TYPE> *bank,
      ::amba::amba_bus_type type, ::amba::amba_layer_ids layer,
      bool arbiter) :
        ::amba::amba_slave_socket<BUSWIDTH>(mn, type, layer, arbiter  /* Arbitration */),
        m_register(bank) {

      // Bind amba blocking ...
      this->register_b_transport(this, &sr_register_amba_socket::b_transport);
      // Register debug transport
      this->register_transport_dbg(this, &sr_register_amba_socket::transport_dbg);
    }

    virtual ~sr_register_amba_socket() {
    }

    unsigned int transport_dbg(tlm::tlm_generic_payload &trans) {
      sc_time delay;
      b_transport(trans, delay);
      return trans.get_data_length();
    }

    void b_transport(tlm::tlm_generic_payload& gp, sc_core::sc_time&) {
      ADDR_TYPE address = gp.get_address() - get_base_addr();
      ADDR_TYPE byteaddr = address & 0x3;
      address = address & ~0x3;
      ADDR_TYPE length = gp.get_data_length();
      DATA_TYPE *data = reinterpret_cast<DATA_TYPE *>(gp.get_data_ptr());

      if (gp.is_write()) {
        //*data = 0;

        switch (length) {
          case 1:
            // shift data by byteaddr * 8 
            /// @todo We have to read the old data from the register first!!!!
            *data <<= (byteaddr << 3);
            break;
          case 2:
            SC_REPORT_FATAL(this->name(), "APBSlave Socket: 2byte access is not implementet correctly\n");
            break;
          case 3:
            SC_REPORT_FATAL(this->name(), "APBSlave Socket: 3byte access is not implementet correctly\n");
            break;
          default:
            #ifdef LITTLE_ENDIAN_BO
            swap_Endianess(*data);
            #endif
            break;
        }
        m_register->bus_write(address, *data);
      }
      if (gp.is_read()) {
        m_register->bus_read(address, *data);
        #ifdef LITTLE_ENDIAN_BO
        swap_Endianess(*data);
        #endif
        *data >>= (byteaddr << 3);
      }
      srInfo()("offset", address)("length", length)("data", data)("write", gp.is_write())("byte", byteaddr)(__PRETTY_FUNCTION__);
      gp.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    /*gs::amba::amba_slave<BUSWIDTH>& operator()() {
      return *this;
    }

    gs::amba::amba_slave<BUSWIDTH>& get_bus_port() {
      return *this;
    }*/

    virtual sc_dt::uint64 get_base_addr() = 0;
    virtual sc_dt::uint64 get_size() = 0;

    sc_register_bank<ADDR_TYPE, DATA_TYPE> *m_register;
};

template<int BUSWIDTH = 32, typename ADDR_TYPE = unsigned int, typename DATA_TYPE = unsigned int>
class APBSlaveSocket : public sr_register_amba_socket<BUSWIDTH, ADDR_TYPE, DATA_TYPE> {
  public:
    APBSlaveSocket(sc_core::sc_module_name mn, sr_register_bank<ADDR_TYPE, DATA_TYPE> *bank) :
        sr_register_amba_socket<BUSWIDTH, ADDR_TYPE, DATA_TYPE>(mn, bank, ::amba::amba_APB, ::amba::amba_LT, false) {
    }
    sc_dt::uint64 get_base_addr() {
      sc_object *obj = this->get_parent_object();
      APBDeviceBase *apb = dynamic_cast<APBDeviceBase *>(obj);
      return (apb)? apb->get_apb_base_addr(): 0;
    }

    sc_dt::uint64 get_size() {
      sc_object *obj = this->get_parent_object();
      APBDeviceBase *apb = dynamic_cast<APBDeviceBase *>(obj);
      return (apb)? apb->get_apb_size(): 0;
    }

};

class APBSlave : public APBDevice<BaseModule<DefaultBase> > {
  public:
    APBSlave(ModuleName mn, uint32_t bus_id, uint8_t vendorid, uint16_t deviceid, uint8_t version,
        uint8_t irq, AMBADeviceType type, uint16_t mask, bool cacheable, bool prefetchable, 
        uint16_t address) : 
      APBDevice<BaseModule<DefaultBase> >(mn,
          bus_id, vendorid, deviceid, version, irq, type, mask, cacheable, prefetchable, address),
      r("register"),
      apb("apb", &r) {
      r.add_associate_busport(&apb);
    }

    APBSlave(ModuleName mn) :
      APBDevice<BaseModule<DefaultBase> >(mn), 
      r("register"),
      apb("apb", &r) {
      r.add_associate_busport(&apb);
    }

    ~APBSlave() {}

    sr_register_bank<unsigned int, unsigned int> r;
    APBSlaveSocket<32, unsigned int, unsigned int> apb;
};

#endif  // CORE_OMMON_APBSLAVE_H_
/// @}
