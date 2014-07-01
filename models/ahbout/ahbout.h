// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup ahbout Output Device
/// @{
/// @file ahbout.h
///
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef AHBOUT_H
#define AHBOUT_H

#include <amba.h>
#include <fstream>
#include <map>
#include <tlm.h>

#include "ahbslave.h"
#include "clkdevice.h"
#include "msclogger.h"

class AHBOut : public AHBSlave<>, public CLKDevice {
  public:
    SC_HAS_PROCESS(AHBOut);

    /// Constructor
    /// @brief Constructor for the test bench memory class
    /// @param haddr AHB address of the AHB slave socket (12 bit)
    /// @param hmask AHB address mask (12 bit)
    /// @param ambaLayer Abstraction layer used (AT/LT)
    /// @param slave_id AHB Slave id
    /// @param outfile File name of a text file to initialize the memory from
    AHBOut(const sc_core::sc_module_name nm,
    uint16_t haddr_,
    uint16_t hmask_ = 0,
    amba::amba_layer_ids ambaLayer = amba::amba_LT,
    uint32_t slave_id = 0,
    char *outfile = NULL);

    /// Destructor
    ~AHBOut();

    uint32_t exec_func(tlm::tlm_generic_payload &gp, sc_time &delay, bool debug = false);

    sc_core::sc_time get_clock();
  private:
    /// 12 bit MSB address and mask (constructor parameters)
    const uint32_t mhaddr;
    const uint32_t mhmask;

    ofstream outfile;
};

#endif
/// @}
