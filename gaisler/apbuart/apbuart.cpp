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

#include "gaisler/apbuart/apbuart.h"
#include "core/common/sr_registry.h"
#include <string>

SR_HAS_MODULE(APBUART);

// Constructor: create all members, registers and Counter objects.
// Store configuration default value in conf_defaults.
APBUART::APBUART(ModuleName name,
  std::string uart_backend,
  uint16_t pindex,
  uint16_t paddr,
  uint16_t pmask,
  int pirq,
  bool console,
  bool powmon) :
  APBSlave(name, pindex, 0x1, 0x00C, 1, pirq, APBIO, pmask, false, false, paddr),
  irq("IRQ"),
  g_pirq(pirq),
  g_console("console", console, m_generics),
  g_backend("backend", uart_backend, m_generics),
  powermon(powmon) {
  SC_THREAD(send_irq);
  SC_THREAD(uart_ticks);
  send_buffer = 0;
  recv_buffer_start = 0;
  recv_buffer_end = 0;
  recv_buffer_level = 0;
  overrun = false;
  level_int = false;
  // TODO(all) Implement and Test interrupt thread
  // Display APB slave information
  srInfo("/configuration/apbuart/apbslave")
     ("addr", (uint64_t)apb.get_base_addr())
     ("size", (uint64_t)apb.get_size())
     ("APB Slave Configuration");

  init_registers();

  /*srInfo()
    ("pindex", pindex)
    ("paddr", paddr)
    ("pmask", pmask)
    ("pirq", pirq)
    //("console", console)
    //("pow_mon", powmon)
    ("An APBUART is created with these generics");*/
}

// Destructor: Unregister Register Callbacks.
// Destroy all Counter objects.
APBUART::~APBUART() {
  GC_UNREGISTER_CALLBACKS();
}

void APBUART::init_generics() {
    g_console.add_properties()
    ("vhdl_name", "console"); 
}

void APBUART::init_registers() {
  /* create register */
  r.create_register("data", "UART Data Register",
    DATA,                                                          // offset
    DATA_DEFAULT,                                                  // init value
    DATA_MASK)                                                     // write mask
  .callback(SR_PRE_READ, this, &APBUART::data_read)
  .callback(SR_POST_WRITE, this, &APBUART::data_write);

  r.create_register("status", "UART Status Register",
    STATUS,                                                        // offset
    STATUS_DEFAULT,                                                // init value
    STATUS_MASK)                                                   // write mask
  .callback(SR_PRE_READ, this, &APBUART::status_read);

  r.create_register("control", "UART Control Register",
    CONTROL,                                                       // offset
    CONTROL_DEFAULT,                                               // init value
    CONTROL_MASK)                                                  // write mask
  .callback(SR_PRE_READ, this, &APBUART::control_read)
  .callback(SR_POST_WRITE, this, &APBUART::control_write);

  r.create_register("scaler", "UART Scaler Register",
    SCALER,                                                        // offset
    SCALER_DEFAULT,                                                // init value
    SCALER_MASK);                                                  // write mask
}

void APBUART::data_read() {
  uint32_t reg = 0;
  if ((recv_buffer_level > 0) && ((r[CONTROL] & 1) == 1)) {  // CONTROL receiver enable
    recv_buffer_level -= 1;
    reg = (uint32_t)recv_buffer[recv_buffer_end];
    inc_fifo_level(&recv_buffer_end);
    r[DATA] = reg;
    //v::info << name() << "Received char: " << reg << v::endl;
    if (((r[CONTROL] & (1<<2)) != 0) && (recv_buffer_level > 0)) {    // CONTROL receiver interrupt enable
      e_irq.notify(clock_cycle * 100);
    //  v::info << name() << "Triggered interrupt, still data in fifo" << v::endl;
    }
  }
}

void APBUART::data_write() {
  char c;
  uint32_t reg = 0;
  reg = r[DATA];
  c = static_cast<char>(reg & 0xFF);
  //v::info << name() << "write to data: " << c << v::endl;
  if (((r[CONTROL] & (1<<1)) != 0)) {
    if (send_buffer >= fifosize) {
      overrun = true;
      //v::info << name() << "missed char due to overrun" << v::endl;
    } else {
      m_backend->sendChar(c);
      send_buffer += 1;
      update_level_int();
      //v::info << name() << "sent char to backend" << v::endl;
    }
  } else {
      // print even if transmitter disabled
      m_backend->sendChar(c);
  }
}

void APBUART::status_read() {
  uint32_t received = recv_buffer_level;
  uint32_t to_transmit = send_buffer;
  if (received >= fifosize) {
    received = fifosize;
  }
  if (send_buffer >= fifosize) {
    to_transmit = fifosize;
  }
  uint32_t reg = (received) ? 0x01 : 0x00;
  reg |= (received << 26);
  reg |= (to_transmit << 20);
  reg |= ((received >= fifosize) << 10);
  reg |= ((to_transmit >= fifosize) << 9);
  reg |= ((received >= (fifosize >> 1)) << 8);
  reg |= ((to_transmit < (fifosize >> 1)) << 7);
  reg |= overrun << 4;
  reg |= (send_buffer <= 1) << 2;
  reg |= (send_buffer == 0) << 1;
  //v::info << name() << "Status Read, Received chars: " << received << " to send: " << to_transmit << " status: " << v::uint32 << reg << v::endl;
  r[STATUS] = reg;
  overrun = false;
}

void APBUART::control_write() {
  //v::info << name() << "Control write: " << v::uint32 << uint32_t(r[CONTROL]) << v::endl;
}

void APBUART::control_read() {
  //v::info << name() << "Control read: " << v::uint32 << uint32_t(r[CONTROL]) << v::endl;
}

void APBUART::send_irq() {
  while (true) {
    wait(e_irq);
    irq.write(std::pair<uint32_t, bool>(1<< g_pirq, true));
    //v::info << name() << "Triggered IRQ " << g_pirq << v::endl;
    wait(clock_cycle);
    if (!level_int) {
      irq.write(std::pair<uint32_t, bool>(1<< g_pirq, false));
    }
  }
}

void APBUART::uart_ticks() {
  bool trigger_irq = false;
  while (true) {
    uint32_t wait_value = 10000;
    if (r[SCALER] != 0) {
      wait_value = r[SCALER] * 8;
    }
    wait(clock_cycle * wait_value);
    trigger_irq = false;
    if (send_buffer > 0) {
      if (((r[CONTROL] & (1<<3)) != 0) && (send_buffer == 1)) {
        trigger_irq = true;
        //v::info << name() << "trigger interrupt because send and fifo empty" << v::endl;
      }
      send_buffer -= 1;
      //v::info << name() << "virtually sent char" << v::endl;
      update_level_int();
    }
    if ((m_backend->receivedChars() > 0) && (recv_buffer_level < fifosize) && ((r[CONTROL] & (1<<2)) != 0)) {
      if (recv_buffer_level == 0) {
        trigger_irq = true;
        //v::info << name() << "trigger interrupt, received char and fifo was empty" << v::endl;
      }
      recv_buffer_level += 1;
      m_backend->getReceivedChar(&(recv_buffer[recv_buffer_end]));
      inc_fifo_level(&recv_buffer_end);
      //v::info << name() << "put char in recv-fifo" << v::endl;
    }
    if (trigger_irq) {
      e_irq.notify();
      //v::info << name() << "triggered interrupt"  << v::endl;
    }
  }

}

void APBUART::update_level_int() {
  if ((r[CONTROL] & (1<<9)) != 0) {
    // if transmitter level interrupt enabled
    if (send_buffer < (fifosize >> 1)) {
      level_int = true;
    } else {
      level_int = false;
    }
    irq.write(std::pair<uint32_t, bool>(1<< g_pirq, level_int));
  } else {
    level_int = false;
  }
  
}

void APBUART::inc_fifo_level(uint32_t *counter) {
  if (*counter < fifosize) {
    *counter += 1;
  } else {
    *counter = 0;
  }
}

void APBUART::dorst() {
}

void APBUART::before_end_of_elaboration() {
  sc_core::sc_object *obj = SrModuleRegistry::create_object_by_name("UARTBackend", g_backend, "backend");
  m_backend = dynamic_cast<io_if *>(obj);
  if (!m_backend) {
    srError("APBUART")
      ("backend", g_backend)
      ("UART backend not created");
  }
}

/// @}
