// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup ahbin AHB Demonstration Input Device
/// @{
/// @file ahbin.h
/// Class definition for input device. Provides frames of data samples in regual
/// intervals. CPU is notifed about new data via IRQ.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef MODELS_AHBIN_AHBIN_H_
#define MODELS_AHBIN_AHBIN_H_

// The TLM header (to be included in each TL model)
#include <tlm.h>

// Provides methods for generating random data
#include <math.h>

// AHB TLM master socket and protocol implementation
#include "core/common/ahbmaster.h"
// Timing interface (specify clock period)
#include "core/common/clkdevice.h"

// Signal definitions for IRQ communication
#include "core/common/sr_signal.h"

// Verbosity kit - for output formatting and filtering
#include "core/common/verbose.h"

/// Definition of class AHBIn
class AHBIn : public AHBMaster<>, public CLKDevice {
  public:
    SC_HAS_PROCESS(AHBIn);
    SR_HAS_SIGNALS(AHBIn);

    /// SignalKit interrupt output
    signal<std::pair<uint32_t, bool> >::out irq;

    /// Constructor
    AHBIn(
      ModuleName name,             ///< The SystemC name of the component
      unsigned int hindex,         ///< The master index for registering with the AHB
      unsigned int hirq,           ///< The number of the IRQ raised for available data
      unsigned int framesize,      ///< The size of the data frame to be generated
      unsigned int frameaddr,      ///< The address the data is supposed to be copied to
      sc_core::sc_time interval,   ///< The interval between data frames
      bool pow_mon,                ///< Enable power monitoring
      AbstractionLayer ambaLayer   ///< TLM abstraction layer
    ) __attribute__ ((deprecated));

    AHBIn(
      ModuleName name,             ///< The SystemC name of the component
      AbstractionLayer ambaLayer,  ///< TLM abstraction layer
      unsigned int hindex,         ///< The master index for registering with the AHB
      unsigned int hirq,           ///< The number of the IRQ raised for available data
      unsigned int framesize,      ///< The size of the data frame to be generated
      unsigned int frameaddr,      ///< The address the data is supposed to be copied to
      sc_core::sc_time interval,   ///< The interval between data frames
      bool pow_mon                 ///< Enable power monitoring
    );

    /// Thread for triggering gen_frame (generates new_frame event)
    void frame_trigger();

    /// Thread for generating the data frame
    void gen_frame();

    /// Reset function
    void dorst();

    /// Deal with clock changes
    void clkcng();

    sc_core::sc_time get_clock();

  private:
    // data members
    // ------------

    /// IRQ number
    const uint32_t m_irq;

    /// Frame size in bytes
    const uint32_t m_framesize;

    /// Target address for data frame
    const uint32_t m_frameaddr;

    /// Time interval for creating data frame
    const sc_core::sc_time m_interval;

    /// ID of the AHB master
    const uint32_t m_master_id;

    /// Event for activating the gen_frame Thread
    sc_event new_frame;

    /// Pointer to the data frame
    uint32_t *frame;

    /// Power monitoring enabled/disable
    bool m_pow_mon;

    /// amba abstraction layer
    AbstractionLayer m_abstractionLayer;
};

#endif  // MODELS_AHBIN_AHBIN_H_
/// @}
