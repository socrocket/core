// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup ahbin
/// @{
/// @file ahbin.cpp
/// Class definition for AHBIn. Provides frames of data samples in regual
/// intervals. CPU is notifed about new data via IRQ.
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#include "gaisler/ahbin/ahbin.h"

/// Constructor
AHBIn::AHBIn(ModuleName name,     // The SystemC name of the component
  unsigned int hindex,                         // The master index for registering with the AHB
  unsigned int hirq,                           // The number of the IRQ raised for available data
  unsigned int framesize,                      // The size of the data frame to be generated
  unsigned int frameaddr,                      // The address the data is supposed to be copied to
  sc_core::sc_time interval,                   // The interval between data frames
  bool pow_mon,                                // Enable power monitoring
  AbstractionLayer ambaLayer) :            // TLM abstraction layer
    AHBMaster<>(name,                            // SystemC name
      hindex,                                    // Bus master index
      0x04,                                      // Vender ID (4 = ESA)
      0x00,                                      // Device ID (undefined)
      0,                                         // Version
      hirq,                                      // IRQ of device
      ambaLayer),                                // AmbaLayer
    irq("irq"),                                  // Initialize interrupt output
    m_irq(0),                                    // Initialize irq number
    m_framesize(framesize),                      // Initialize framesize
    m_frameaddr(frameaddr),                      // Initialize frameaddr
    m_interval(interval),                        // Initialize frame interval
    m_master_id(hindex),                         // Initialize bus index
    m_pow_mon(pow_mon),                          // Initialize pow_mon
    m_abstractionLayer(ambaLayer) {              // Initialize abstraction layer
  // Register frame_trigger thread
  SC_THREAD(frame_trigger);

  // Register thread for generating the actual data frame
  SC_THREAD(gen_frame);
  sensitive << new_frame;

  // Allocate data frame buffer of size framesize
  frame = new uint32_t(m_framesize);

  // Module Configuration Report
  v::info << this->name() << " ************************************************** " << v::endl;
  v::info << this->name() << " * Created AHBIn in following configuration: " << v::endl;
  v::info << this->name() << " * --------------------------------------------- " << v::endl;
  v::info << this->name() << " * abstraction Layer (LT = 8 / AT = 4): " << m_abstractionLayer << v::endl;
  v::info << this->name() << " ************************************************** " << v::endl;
}

AHBIn::AHBIn(
  ModuleName name,     // The SystemC name of the component
  AbstractionLayer ambaLayer,
  unsigned int hindex,                         // The master index for registering with the AHB
  unsigned int hirq,                           // The number of the IRQ raised for available data
  unsigned int framesize,                      // The size of the data frame to be generated
  unsigned int frameaddr,                      // The address the data is supposed to be copied to
  sc_core::sc_time interval,                   // The interval between data frames
  bool pow_mon                                 // Enable power monitoring
  ) :            // TLM abstraction layer
    AHBMaster<>(name,                            // SystemC name
      hindex,                                    // Bus master index
      0x04,                                      // Vender ID (4 = ESA)
      0x00,                                      // Device ID (undefined)
      0,                                         // Version
      hirq,                                      // IRQ of device
      ambaLayer),                                // AmbaLayer
    irq("irq"),                                  // Initialize interrupt output
    m_irq(0),                                    // Initialize irq number
    m_framesize(framesize),                      // Initialize framesize
    m_frameaddr(frameaddr),                      // Initialize frameaddr
    m_interval(interval),                        // Initialize frame interval
    m_master_id(hindex),                         // Initialize bus index
    m_pow_mon(pow_mon),                          // Initialize pow_mon
    m_abstractionLayer(ambaLayer) {              // Initialize abstraction layer
  // Register frame_trigger thread
  SC_THREAD(frame_trigger);

  // Register thread for generating the actual data frame
  SC_THREAD(gen_frame);
  sensitive << new_frame;

  // Allocate data frame buffer of size framesize
  frame = new uint32_t(m_framesize);

  // Module Configuration Report
  v::info << this->name() << " ************************************************** " << v::endl;
  v::info << this->name() << " * Created AHBIn in following configuration: " << v::endl;
  v::info << this->name() << " * --------------------------------------------- " << v::endl;
  v::info << this->name() << " * abstraction Layer (LT = 8 / AT = 4): " << m_abstractionLayer << v::endl;
  v::info << this->name() << " ************************************************** " << v::endl;
}

// Rest handler
void AHBIn::dorst() {
  // Nothing to do
}

// This thread generates a "new_frame" event at intervals "m_interval"
void AHBIn::frame_trigger() {
  while (1) {
    // Wait for m_interval SystemC time
    wait(m_interval);

    // Activate "new_frame" event
    new_frame.notify();
  }
}

// Generates a thread of frame_length
void AHBIn::gen_frame() {
  // Locals
  uint32_t tmp;
  // uint32_t debug;
  // sc_core::sc_time delay = SC_ZERO_TIME;

  // Wait for system becoming ready
  wait(1, SC_MS);

  while (1) {
    v::info << name() << "Start sending new data frame!" << v::endl;

    // Generate data frame and send to memory
    for (uint32_t i = 0; i < m_framesize; i++) {
      // Random number
      tmp = rand();

      // AHB write
      ahbwrite((m_frameaddr << 20) + (i << 2), (unsigned char *)&tmp, 4);
    }

    v::info << name() << "Transmission of frame completed" << v::endl;

    // Notify CPU by raising an interrupt
    irq.write(std::pair<uint32_t, bool>(1 << m_irq, true));

    // After one clock_cycle SystemC time lower to interrupt
    wait(clock_cycle);
    irq.write(std::pair<uint32_t, bool>(1 << m_irq, false));

    // Wait for next frame trigger
    wait();
  }
}

// Helper for setting clock cycle latency using a value-time_unit pair
void AHBIn::clkcng() {
  // nothing to do
}

sc_core::sc_time AHBIn::get_clock() {
  return clock_cycle;
}

/// @}
