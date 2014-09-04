// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbuart
/// @{
/// @file apbuart.cpp
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#include "core/models/apbuart/apbuart.h"
#include <string>

// Constructor: create all members, registers and Counter objects.
// Store configuration default value in conf_defaults.
APBUART::APBUART(ModuleName name,
  io_if *backend,
  uint16_t pindex,
  uint16_t paddr,
  uint16_t pmask,
  int pirq,
  bool console,
  bool powmon) :
  APBDevice<RegisterBase>(name, pindex, 0x1, 0x00C, 1, pirq, APBIO, pmask, false, false, paddr, 4),
  bus("bus", r, ((paddr) & pmask) << 8, (((~pmask & 0xfff) + 1) << 8), ::amba::amba_APB, ::amba::amba_LT, false),
  irq("IRQ"),
  m_backend(backend),
  g_pirq(pirq),
  powermon(powmon) {
  // TODO(all) Implement and Test interrupt thread
  // Display APB slave information
  v::info << this->name() << "APB slave @0x" << hex << v::setw(8)
          << v::setfill('0') << bus.get_base_addr() << " size: 0x" << hex
          << v::setw(8) << v::setfill('0') << bus.get_size() << " byte"
          << endl;

  /* create register */
  r.create_register("data", "UART Data Register",
    DATA,                                                          // offset
    gs::reg::STANDARD_REG | gs::reg::SINGLE_IO |                   // config
    gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
    DATA_DEFAULT,                                                  // init value
    DATA_MASK,                                                     // write mask
    32,                                                            // Register width.
    0x00);                                                         // Lock Mask: NI.
  r.create_register("status", "UART Status Register",
    STATUS,                                                        // offset
    gs::reg::STANDARD_REG | gs::reg::SINGLE_IO |                   // config
    gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
    STATUS_DEFAULT,                                                // init value
    STATUS_MASK,                                                   // write mask
    32,                                                            // Register width.
    0x00);                                                         // Lock Mask: NI.
  r.create_register("control", "UART Control Register",
    CONTROL,                                                       // offset
    gs::reg::STANDARD_REG | gs::reg::SINGLE_IO |                   // config
    gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
    CONTROL_DEFAULT,                                               // init value
    CONTROL_MASK,                                                  // write mask
    32,                                                            // Register width.
    0x00);                                                         // Lock Mask: NI.
  r.create_register("scaler", "UART Scaler Register",
    SCALER,                                                        // offset
    gs::reg::STANDARD_REG | gs::reg::SINGLE_IO |                   // config
    gs::reg::SINGLE_BUFFER | gs::reg::FULL_WIDTH,
    SCALER_DEFAULT,                                                // init value
    SCALER_MASK,                                                   // write mask
    32,                                                            // Register width.
    0x00);                                                         // Lock Mask: NI.
  // Configuration report
  v::info << this->name() << " ******************************************************************************* " <<
  v::endl;
  v::info << this->name() << " * Created UART with following parameters: " << v::endl;
  v::info << this->name() << " * ------------------------------------------ " << v::endl;
  v::info << this->name() << " * pindex: " << pindex << v::endl;
  v::info << this->name() << " * paddr/pmask: " << hex << paddr << "/" << pmask << v::endl;
  v::info << this->name() << " * pirq: " << pirq << v::endl;
  v::info << this->name() << " * console: " << console << v::endl;
  v::info << this->name() << " * pow_mon: " << powmon << v::endl;
  v::info << this->name() << " ******************************************************************************* " <<
  v::endl;
}

// Destructor: Unregister Register Callbacks.
// Destroy all Counter objects.
APBUART::~APBUART() {
  GC_UNREGISTER_CALLBACKS();
}

// Set all register callbacks
void APBUART::end_of_elaboration() {
  GR_FUNCTION(APBUART, data_read);
  GR_SENSITIVE(r[DATA].add_rule(gs::reg::PRE_READ, "data_read", gs::reg::NOTIFY));

  GR_FUNCTION(APBUART, data_write);
  GR_SENSITIVE(r[DATA].add_rule(gs::reg::POST_WRITE, "data_write", gs::reg::NOTIFY));

  GR_FUNCTION(APBUART, status_read);
  GR_SENSITIVE(r[STATUS].add_rule(gs::reg::PRE_READ, "data_read", gs::reg::NOTIFY));
}

void APBUART::data_read() {
  char c;
  uint32_t reg = 0;
  if (m_backend->receivedChars() && r[STATUS].bit_get(0)) {  // CONTROL receiver enable
    m_backend->getReceivedChar(&c);
    reg = (uint32_t)c;
    r[DATA] = reg;
    v::debug << name() << "Received chars: " << reg << v::endl;
    if (r[STATUS].bit_get(2)) {    // CONTROL receiver interrupt enable
      e_irq.notify(clock_cycle * 100);
    }
  }
}

void APBUART::data_write() {
  char c;
  uint32_t reg = 0;
  reg = r[DATA];
  c = static_cast<char>(reg & 0xFF);
  if (r[STATUS].bit_get(1)) {
    m_backend->sendChar(c);
  }
  if (r[STATUS].bit_get(3)) {
    e_irq.notify(clock_cycle * 100);
  }
}

void APBUART::status_read() {
  uint32_t received = m_backend->receivedChars();
  if (received >= 0xEF) {
    received = 0xEF;
  }
  uint32_t reg = (received) ? 0x7 : 0x6;
  reg |= (received << 26);
  v::debug << name() << "Status Read, Received chars: " << received << v::endl;
  r[STATUS] = reg;
}

void APBUART::send_irq() {
  while (true) {
    wait(e_irq);
    irq.write(std::pair<uint32_t, bool>(1<< g_pirq, true));
    wait(clock_cycle);
    irq.write(std::pair<uint32_t, bool>(1<< g_pirq, false));
  }
}

void APBUART::dorst() {
}

/// @}
