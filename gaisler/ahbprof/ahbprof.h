// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup ahbprof AHB Profiler
/// @{
/// @file ahbprof.h
///
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef MODELS_AHBPROF_AHBPROF_H_
#define MODELS_AHBPROF_AHBPROF_H_

#include "core/common/amba.h"
#include <stdint.h>
#include <tlm.h>
#include <fstream>
#include <map>

#include "core/common/ahbslave.h"
#include "core/common/clkdevice.h"
#include "core/common/msclogger.h"

struct prof_info {
  prof_info() : state(0), real_start(0), real_end(0), sim_start(sc_core::SC_ZERO_TIME), sim_end(sc_core::SC_ZERO_TIME),
    instr_start(0), instr_end(0) {}
  int state;
  uint32_t real_start;
  uint32_t real_end;
  sc_core::sc_time sim_start;
  sc_core::sc_time sim_end;
  uint64_t instr_start;
  uint64_t instr_end;
};

class AHBProf : public AHBSlave<>, public CLKDevice {
  public:
    SC_HAS_PROCESS(AHBProf);

    /// Constructor
    /// @brief Constructor for the test bench memory class
    /// @param haddr AHB address of the AHB slave socket (12 bit)
    /// @param hmask AHB address mask (12 bit)
    /// @param ambaLayer Abstraction layer used (AT/LT)
    /// @param slave_id AHB Slave id
    /// @param outfile File name of a text file to initialize the memory from
    AHBProf(const ModuleName nm,
    uint32_t index = 0,
    uint16_t addr = 0,
    uint16_t mask = 0,
    AbstractionLayer ambaLayer = amba::amba_LT);

    /// Destructor
    ~AHBProf();

    uint32_t exec_func(
        tlm::tlm_generic_payload &gp,  // NOLINT(runtime/references)
        sc_time &delay,                // NOLINT(runtime/references)
        bool debug = false);

    sc_core::sc_time get_clock();
    std::map<int, prof_info> info;

  private:
    /// 12 bit MSB address and mask (constructor parameters)
    const uint32_t m_addr;
    const uint32_t m_mask;
};

#endif  // MODELS_AHBPROF_AHBPROF_H_
/// @}
