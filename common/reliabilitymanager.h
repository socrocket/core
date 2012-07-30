#ifndef RELIABILITYMANAGER_H
#define RELIABILITYMANAGER_H

#include <systemc.h>
#include <tlm.h>

#include <stdint.h>
#include <string.h>
#include <vmap.h>

class FaultDectect_if {
  public:
    virtual bool detect(tlm::generic_payload &gp) = 0;
    virtual void protect(tlm::generic_payload &gp) = 0;
};

template<class WIDTH>
struct ReliabilityCell {
  ReliabilityCell();
  enum Mode {
    NONE,
    PERMANENT_DAMAGED,
    TRANSIENT_NOTE,
    TRANSIENT_DAMAGED
  };
  enum Type {
    NONE,
    ADDRESS,
    DATA
  };
  Mode mode;
  Type type
  WIDTH andMask;
  WIDTH orMask;
};

template<uint8_t>ReliabilityCell::ReliabilityCell():
  mode(NONE), type(NONE), andMask(0xFF), orMask(0x0) {};

template<uint32_t>ReliabilityCell::ReliabilityCell():
  mode(NONE), type(NONE), andMask(0xFFFFFFFF), orMask(0x0) {};

class ReliabilityManager {
  public:
    void transaction(sc_object *self, tlm::generic_payload &gp);
    void registerModel(sc_object *self);

    void addressAddressPermanent(std::string name, uint8_t _and, uint8_t _or);
    void addressAddressTransient(std::string name, uint8_t _and, uint8_t _or);

    void damageDataPermanent(uint32_t addr, uint8_t _and, uint8_t _or);
    void damageDataPermanent(std::string name, uint8_t _and, uint8_t _or);

    void damageDataTransient(uint32_t addr, uint8_t _and, uint8_t _or);
    void damageDataTransient(std::string name, uint8_t _and, uint8_t _or);

    void script() = 0;

  private:
    gs::gcnf_api *m_Api; 

    vmap<std::string, ReliabilityCell<uint32_t> m_addresserrors;

    tlm::tlm_dbg_transport_if *mmapped_bus;
    /// Errors in memory mapped registers and memories. Addressed by bus address
    vmap<uint32_t, ReliabilityCell<uint8_t> > m_mmapped;

    /// Errors in internal registers. Addressed by gs_param name
    vmap<std::string, ReliabilityCell<uint32_t> > m_internal;
};

ReliabilityManager *RM;

#endif

