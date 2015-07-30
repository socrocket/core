// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup ahbout AHB Demonstration Output Device
/// @{
/// @file ahbout.h
///
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef MODELS_AHBOUT_AHBOUT_H_
#define MODELS_AHBOUT_AHBOUT_H_

#include "core/common/amba.h"
#include <tlm.h>
#include <fstream>
#include <map>

#include "core/common/ahbslave.h"
#include "core/common/clkdevice.h"
#include "core/common/msclogger.h"

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
    AHBOut(const ModuleName nm,
    uint16_t haddr_,
    uint16_t hmask_ = 0,
    AbstractionLayer ambaLayer = amba::amba_LT,
    uint32_t slave_id = 0,
    char *outfile = NULL);

    /// Destructor
    ~AHBOut();

    uint32_t exec_func(
        tlm::tlm_generic_payload &gp,  // NOLINT(runtime/references)
        sc_time &delay,                // NOLINT(runtime/references)
        bool debug = false);

    sc_core::sc_time get_clock();

  private:
    /// 12 bit MSB address and mask (constructor parameters)
    const uint32_t mhaddr;
    const uint32_t mhmask;

    ofstream outfile;
};

#endif  // MODELS_AHBOUT_AHBOUT_H_
/// @}
