// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbctrl
/// @{
/// @file apbctrl.cpp
/// Implementation of the AHB APB Bridge
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include <map>
#include <utility>

#include "gaisler/apbctrl/apbctrl.h"
#include "core/common/vendian.h"
#include "core/common/verbose.h"
#include "core/common/sr_report.h"

SR_HAS_MODULE(APBCtrl);

/// Constructor of class APBCtrl
APBCtrl::APBCtrl(
    ModuleName nm,  // SystemC name
    uint32_t haddr,              // The MSB address of the AHB area. Sets the 12 MSBs in the AHB address
    uint32_t hmask,              // The 12bit AHB area address mask
    bool mcheck,                 // Check if there are any intersections between APB slave memory regions
    uint32_t hindex,             // AHB bus index
    bool pow_mon,                // Enable power monitoring
    AbstractionLayer ambaLayer) :
  AHBSlave<>(nm,
    hindex,
    0x01,                        // vendor_id: Gaisler
    0x006,                       // device_id: APBCTRL (p. 92 GRIP)
    0,
    0,
    ambaLayer,
    BAR(AHBMEM, hmask, 0, 0, haddr)),
  apb("apb", amba::amba_APB, amba::amba_LT, false),
  m_AcceptPEQ("AcceptPEQ"),
  m_TransactionPEQ("TransactionPEQ"),
  m_pnpbase(0xFF000),
  //g_haddr("haddr", haddr, m_generics),
  //g_hmask("hmask", hmask, m_generics),
  g_mcheck("mcheck", mcheck, m_generics),
  g_pow_mon("pow_mon", pow_mon, m_generics),
  m_ambaLayer(ambaLayer),
  num_of_bindings(0),
  m_total_transactions("total_transactions", 0ull, m_counters),
  m_right_transactions("successful_transactions", 0ull, m_counters),
  sta_power_norm("sta_power_norm", 2.11e+6, m_power),  // Normalized static power input
  int_power_norm("int_power_norm", 0.0, m_power),  // Normalized internal power input (activation indep.)
  dyn_read_energy_norm("dyn_read_energy_norm", 5.84e-11, m_power),  // Normalized read energy input
  dyn_write_energy_norm("dyn_write_energy_norm", 5.84e-11, m_power),  // Normalized write energy input
  sta_power("sta_power", 0.0, m_power),  // Static power output
  int_power("int_power", 0.0, m_power),  // Internal power output
  swi_power("swi_power", 0.0, m_power),  // Switching power output
  dyn_read_energy("dyn_read_energy", 0.0, m_power),  // Energy per read access
  dyn_write_energy("dyn_write_energy", 0.0, m_power),  // Energy per write access
  dyn_reads("dyn_reads", 0ull, m_power),  // Read access counter for power computation
  dyn_writes("dyn_writes", 0ull, m_power) {  // Write access counter for power computation
  // Assert generics are withing allowed ranges
  assert(haddr <= 0xfff);
  assert(hmask <= 0xfff);

  srInfo()
    ("haddr", haddr)
    ("hmask", hmask)
    ("hindex", hindex)
    ("mcheck", mcheck)
    ("pow_mon", pow_mon)
    ("ambaLayer", ambaLayer)
    ("Created an APBCtrl with this parameters");

  // initialize the slave_map_cache with some bogus numbers which will trigger MISS
  slave_info_t tmp;
  tmp.pindex = 0;
  tmp.pmask  = ~0;
  tmp.binding = ~0;
  slave_map_cache = std::pair<uint32_t, slave_info_t>(~0, tmp);
}

// Reset handler
void APBCtrl::dorst() {
  // nothing to do
}

// Destructor
APBCtrl::~APBCtrl() {
  GC_UNREGISTER_CALLBACKS();
}

void APBCtrl::init_generics() {
  g_mcheck.add_properties()
    ("name", "Memory check")
    ("If true check slaves for memory address errors");

  g_pow_mon.add_properties()
    ("name", "Power Monitoring")
    ("If true enable power monitoring");
}

/// Helper function for creating slave map decoder entries
void APBCtrl::setAddressMap(const uint32_t binding, const uint32_t pindex, const uint32_t paddr, const uint32_t pmask) {
  // Create slave map entry from slave ID and address range descriptor (slave_info_t)
  slave_info_t tmp;

  tmp.pindex = pindex;
  tmp.pmask  = pmask;
  tmp.binding = binding;

  slave_map.insert(std::pair<uint32_t, slave_info_t>(paddr, tmp));
}

/// Find slave index by address
int APBCtrl::get_index(const uint32_t address) {
  // Use 12 bit segment address for decoding
  uint32_t addr = (address >> 8) & 0xfff;

  slave_info_t info = slave_map_cache.second;
  if ( ! ((addr ^ slave_map_cache.first) & info.pmask) ) {
    // APB: Device == BAR)
    m_right_transactions++;
    return slave_map_cache.second.binding;
  }

  std::map<uint32_t, slave_info_t>::iterator it = --(slave_map.upper_bound( addr ));
  info = it->second;
  if ( ! ((addr ^ it->first) & info.pmask) ) {
    // MISS in the cache: update cache
    slave_map_cache = *it;
    
    // APB: Device == BAR)
    m_right_transactions++;
    return it->second.binding;
  }

  // no slave found
  return -1;
}

// Returns a PNP register from the APB configuration area (upper 4kb of address space)
uint32_t APBCtrl::getPNPReg(const uint32_t address) {
  // Calculate address offset in configuration area
  uint32_t addr = address - (get_ahb_bar_addr(0) + m_pnpbase);
  // Calculate index of the device in mSlaves pointer array (8 byte per device)
  uint32_t device = (addr >> 2) >> 1;
  // Calculate offset within device information
  uint32_t offset = (addr >> 2) & 0x1;

  if ((device < 16) && (mSlaves[device] != NULL)) {
    m_right_transactions++;
    uint32_t result = mSlaves[device][offset];
#ifdef LITTLE_ENDIAN_BO
    swap_Endianess(result);
#endif
    return result;
  } else {
    v::debug << name() << "Access to not existing PNP Register!" << v::endl;
    return 0;
  }
}

// Functional part of the model (decoding logic)
uint32_t APBCtrl::exec_func(
    tlm::tlm_generic_payload &ahb_gp,  // NOLINT(runtime/references)
    sc_time &delay,                    // NOLINT(runtime/references)
    bool debug) {

  payload_t * apb_gp = NULL;
  unsigned int i = 0;
  m_total_transactions++;
  transport_statistics(ahb_gp);

  // Extract data pointer from payload
  uint8_t *data = ahb_gp.get_data_ptr();
  // Extract address from payload
  uint32_t addr = ahb_gp.get_address();
  // Extract length from payload
  uint32_t length = ahb_gp.get_data_length();
      
  // Is this an access to the configuration area
  // The configuration area is always in the upper 0xFF000 area
  if (((addr ^ m_pnpbase) & m_pnpbase) == 0) {
    // Configuration area is read only

    if (ahb_gp.get_command() == tlm::TLM_READ_COMMAND) {
      // addr = addr - ((m_pnpbase) & (m_pnpbase));
      for (uint32_t i = 0; i < length; i++) {
        // uint32_t word = (addr + i) >> 2;
        uint32_t byte = (addr + i) & 0x3;
        uint32_t reg = getPNPReg(addr + i);

        data[i] = (reinterpret_cast<uint8_t *>(&reg))[byte];

        delay += clock_cycle;
      }

      ahb_gp.set_response_status(tlm::TLM_OK_RESPONSE);
    } else {
      v::error << name() << " Forbidden write to APBCTRL configuration area (PNP)!" << v::endl;
      ahb_gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    }

    return ahb_gp.get_data_length();
  }

  // Split up transaction into multiple 32bit APB accesses
  for (i = 0;i < length; i+=4) {

    // Find slave by address / returns slave index or -1 for not mapped
    int index = get_index(addr+i);  

    // For valid slave index
    if(index >= 0) {

      // -- For Debug only --
      uint32_t a = 0;
      socket_t *other_socket = apb.get_other_side(index, a);
      sc_core::sc_object *obj = other_socket->get_parent();

      v::debug << name() << "Forwarding request to APB slave:" << obj->name()
         << "@0x" << hex << v::setfill('0') << v::setw(8)
         << ((ahb_gp.get_address() & 0x000fffff)+i) << endl;
      // --------------------

      // Take APB transaction from pool
      apb_gp = apb.get_transaction();
      if(!apb_gp) {
        srError()("Cannot allocate transaction object");
        assert(apb_gp);
      }

      // Build APB transaction from incoming AHB transaction:
      // ----------------------------------------------------
      apb_gp->set_command(ahb_gp.get_command());
      // Substract the base address of the bridge
      apb_gp->set_address((ahb_gp.get_address() & 0x000fffff)+i);
      apb_gp->set_data_length(( length <=4 ) ? length : 4); 
      apb_gp->set_byte_enable_ptr(ahb_gp.get_byte_enable_ptr());
      apb_gp->set_data_ptr(ahb_gp.get_data_ptr()+i);

      if (!debug) {

        // Power event start
        //PM::send(this,"apb_trans", 1, sc_time_stamp(), (unsigned int)apb_gp->get_data_ptr(), g_pow_mon);

        // Forward request to the selected slave

        apb[index]->b_transport(*apb_gp, delay);

        // Add delay for APB setup cycle
        delay += clock_cycle;

        // Power Calculation
        if (g_pow_mon) {
          if (ahb_gp.get_command() == tlm::TLM_READ_COMMAND) {
            dyn_reads += (ahb_gp.get_data_length() >> 2) + 1;
          } else {
            dyn_writes += (ahb_gp.get_data_length() >> 2) + 1;
          }
        }
      } else {
        apb[index]->transport_dbg(*apb_gp);
      }
      
      // Copy back response message
      ahb_gp.set_response_status(apb_gp->get_response_status());
      // Release transaction
      apb.release_transaction(apb_gp);

    } else {
      v::warn << name() << "Access to unmapped APB address space at address " << v::uint32 << addr << endl;
      ahb_gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }
  }

  return ahb_gp.get_data_length();
}

void APBCtrl::end_of_elaboration() {
  srInfo()
    ("haddr", get_ahb_bar_base(0))
    ("hmask", get_ahb_bar_mask(0))
    ("hindex", get_ahb_hindex())
    ("mcheck", g_mcheck)
    ("Created an APBCtrl with this parameters");

  // Register power monitor
  if (g_pow_mon) {
    GC_REGISTER_TYPED_PARAM_CALLBACK(&sta_power, gs::cnf::pre_read, APBCtrl, sta_power_cb);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&int_power, gs::cnf::pre_read, APBCtrl, int_power_cb);
    GC_REGISTER_TYPED_PARAM_CALLBACK(&swi_power, gs::cnf::pre_read, APBCtrl, swi_power_cb);
  }
}

/// Set up slave map and collect plug & play information
void APBCtrl::start_of_simulation() {
  // Get number of bindings at master socket (number of connected slaves)
  num_of_bindings = apb.size();

  // max. 16 APB slaves allowed
  assert(num_of_bindings <= 16);

  v::info << name() << "******************************************************************************* " << v::endl;
  v::info << name() << "* APB DECODER INITIALIZATION " << v::endl;
  v::info << name() << "* -------------------------- " << v::endl;

  // iterate the registered slaves
  for (uint32_t i = 0; i < num_of_bindings; i++) {
    uint32_t a = 0;

    // get pointer to socket of slave i
    socket_t *other_socket = apb.get_other_side(i, a);

    // get parent object containing slave socket i
    sc_core::sc_object *obj = other_socket->get_parent();

    // valid slaves implement the APBDevice interface
    APBDeviceBase *slave = dynamic_cast<APBDeviceBase *>(obj);

    v::info << name() << "* Slave name: " << obj->name() << v::endl;

    // slave is valid (implements APBDevice)
    if (slave) {
      // Get pointer to device information
      const uint32_t *deviceinfo = slave->get_apb_device_info();

      // Get slave id (pindex)
      const uint32_t sbusid = slave->get_apb_pindex();

      // Map device information into PNP region
      mSlaves[sbusid] = deviceinfo;

      // check 'type'filed of bar[i] (must be != 0)
      if (slave->get_apb_type()) {
        // get base address and mask from BAR
        uint32_t addr = slave->get_apb_base();
        uint32_t mask = slave->get_apb_mask();

        v::info << name() << "* BAR with MSB addr: " << hex << addr << " and mask: " << mask << v::endl;

        // insert slave region into memory map
        setAddressMap(i, sbusid, addr, mask);
      }
    } else {
      v::warn << name() << "Slave bound to socket 'apb' is not a valid APBDevice." << v::endl;
      assert(0);
    }
  }

  // End of decoder initialization
  v::info << name() << "******************************************************************************* " << v::endl;

  // Check memory map for overlaps
  if (g_mcheck) {
    checkMemMap();
  }

  // Initialize power model
  if (g_pow_mon) {
    power_model();
  }
}

// Calculate power/energy values from normalized input data
void APBCtrl::power_model() {
  // Static power calculation (pW)
  sta_power = sta_power_norm * num_of_bindings;

  // Dynamic power (switching independent internal power)
  int_power = int_power_norm * num_of_bindings * 1 / (clock_cycle.to_seconds());

  // Energy per read access (uJ)
  dyn_read_energy = dyn_read_energy_norm * num_of_bindings;

  // Energy per write access (uJ)
  dyn_write_energy = dyn_write_energy_norm * num_of_bindings;
}

// Static power callback
gs::cnf::callback_return_type APBCtrl::sta_power_cb(
    gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
    gs::cnf::callback_type reason) {
  // Nothing to do !!
  // Static power of APBCTRL is constant !!
  return GC_RETURN_OK;
}

// Internal power callback
gs::cnf::callback_return_type APBCtrl::int_power_cb(
    gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
    gs::cnf::callback_type reason) {
  // Nothing to do !!
  // RTL APBCTRL has no internal power - constant.
  return GC_RETURN_OK;
}

// Switching power callback
gs::cnf::callback_return_type APBCtrl::swi_power_cb(
    gs::gs_param_base &changed_param,  // NOLINT(runtime/references)
    gs::cnf::callback_type reason) {
  swi_power =
    ((dyn_read_energy *
      dyn_reads) + (dyn_write_energy * dyn_writes)) / (sc_time_stamp() - power_frame_starting_time).to_seconds();
  return GC_RETURN_OK;
}

// Print execution statistic at end of simulation
void APBCtrl::end_of_simulation() {
  v::report << name() << " ********************************************" << v::endl;
  v::report << name() << " * APBCtrl Statistic:" << v::endl;
  v::report << name() << " * ------------------" << v::endl;
  v::report << name() << " * Successful Transactions: " << m_right_transactions << v::endl;
  v::report << name() << " * Total Transactions:      " << m_total_transactions << v::endl;
  v::report << name() << " *  " << v::endl;
  v::report << name() << " * AHB Slave interface reports: " << v::endl;
  print_transport_statistics(name());
  v::report << name() << " ******************************************** " << v::endl;
}

sc_core::sc_time APBCtrl::get_clock() {
  return clock_cycle;
}

struct apb_check_slave_type {
  uint32_t start;
  uint32_t end;
  uint32_t index;
};

// Check the memory map for overlaps
void APBCtrl::checkMemMap() {
  map<uint32_t, apb_check_slave_type> slaves;
  typedef map<uint32_t, apb_check_slave_type>::iterator iter_t;
  struct apb_check_slave_type last;
  last.start = 0;
  last.end = 0;
  last.index = ~0;

  for (std::map<uint32_t, slave_info_t>::iterator iter = slave_map.begin(); iter != slave_map.end(); iter++) {
    uint32_t start_addr = (iter->first & iter->second.pmask) << 12;
    uint32_t size = ((~iter->second.pmask & 0xFFF) + 1) << 12;
    struct apb_check_slave_type obj;
    obj.start = start_addr;
    obj.end = start_addr + size - 1;
    obj.index = iter->second.pindex;
    slaves.insert(make_pair(start_addr, obj));
  }
  for (iter_t iter = slaves.begin(); iter != slaves.end(); iter++) {
    // First Slave need it in last to start
    if (last.index != ~0u) {
      // All other elements
      // See if the last element is begining and end befor the current
      if ((last.start >= iter->second.start) || (last.end >= iter->second.start)) {
        uint32_t a = 0;
        socket_t *other_socket = apb.get_other_side(last.index, a);
        sc_core::sc_object *obj = other_socket->get_parent();

        other_socket = apb.get_other_side(iter->second.index, a);
        sc_core::sc_object *obj2 = other_socket->get_parent();

        v::error << name() << "Overlap in AHB memory mapping." << v::endl;
        v::error << name() << obj->name() << " " << v::uint32 << last.start << " - " << v::uint32 << last.end << endl;
        v::error << name() << obj2->name() << " " << v::uint32 << iter->second.start << " - " << v::uint32 <<
        iter->second.end << endl;
      }
    }
    last = iter->second;
  }
}

/// @}
