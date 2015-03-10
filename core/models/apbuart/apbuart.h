// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbuart
/// @{
/// @file apbuart.h
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef MODELS_APBUART_APBUART_H_
#define MODELS_APBUART_APBUART_H_

#include <greencontrol/all.h>
#include "core/common/grambasockets/greenreg_ambasockets.h"
#include <boost/config.hpp>
#include "core/common/systemc.h"
#include <string>
#include <vector>

#include "core/models/apbuart/io_if.h"
#include "core/models/utils/apbdevice.h"
#include "core/models/utils/clkdevice.h"
#include "core/common/signalkit.h"
#include "core/common/verbose.h"

/// @brief This class is a TLM 2.0 Model of the Aeroflex Gaisler GRLIB APBUART.
/// Further informations to the original VHDL Modle are available in the GRLIB IP Core User's Manual Section 16
class APBUART : public APBDevice<RegisterBase>, public CLKDevice {
  public:
    SC_HAS_PROCESS(APBUART);
    SK_HAS_SIGNALS(APBUART);
    GC_HAS_CALLBACKS();
    /// APB Slave socket for all bus communication
    gs::reg::greenreg_socket<gs::amba::amba_slave<32> > bus;

    signal<std::pair<uint32_t, bool> >::out irq;

    sc_event e_irq;
    sc_event s_irq; // trigger interrupt when send buffer empty

    io_if *m_backend;
    uint32_t g_pirq;
    gs::cnf::gs_config<bool> g_console;
    APBUART(ModuleName name, io_if *backend, uint16_t pindex = 0,
    uint16_t paddr = 0, uint16_t pmask = 4095, int pirq = 0,
    bool console = false,
    // bool parity = false, bool flow = false, int fifosize = 0,
    bool powmon = false);

    /// Free all counter and unregister all callbacks.
    ~APBUART();
    void init_generics();
    
    /// Execute the callback registering when systemc reaches the end of elaboration.
    void end_of_elaboration();

    // Register Callbacks
    void data_read();

    void data_write();
    
    void control_read();

    void control_write();

    void status_read();
    
    void update_level_int();

    // SCTHREADS
    void send_irq();
    void uart_ticks();

    void inc_fifo_level(uint32_t *counter);

    // Signal Callbacks
    virtual void dorst();

    const uint32_t powermon;

    static const uint32_t DATA            = 0x00000000;
    static const uint32_t STATUS          = 0x00000004;
    static const uint32_t CONTROL         = 0x00000008;
    static const uint32_t SCALER          = 0x0000000C;

    static const uint32_t DATA_DEFAULT    = 0x0;
    static const uint32_t STATUS_DEFAULT  = 0x00000086;
    static const uint32_t CONTROL_DEFAULT = 0x80000000;
    static const uint32_t SCALER_DEFAULT  = 0x0;

    static const uint32_t DATA_MASK       = 0x000000FF;
    static const uint32_t STATUS_MASK     = 0x00000000;
    //static const uint32_t CONTROL_MASK    = 0x00007FFF;
    static const uint32_t CONTROL_MASK    = 0x7FFFFFFF;
    static const uint32_t SCALER_MASK     = 0x00000FFF;

    static const uint32_t fifosize = 32; // 1, 2, 4, 8, 16, 32

    bool overrun;
    bool level_int;

    uint32_t send_buffer;
    uint32_t recv_buffer_level;
    char recv_buffer[32];
    uint32_t recv_buffer_start;
    uint32_t recv_buffer_end;
};

#endif  // MODELS_APBUART_APBUART_H_
/// @}
