// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup platform
/// @{
/// @file sc_main.cpp
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
#include <sys/time.h>
#include <time.h>

#include "core/common/gs_config.h"
#include "core/common/systemc.h"
#include "core/common/sr_report.h"

#include "core/platforms/newleon3mp/leon3mp.h"
#ifdef HAVE_USI
#include "pysc/usi.h"
#endif

namespace trap {
  extern int exitValue;
};

void stopSimFunction( int sig ){
  v::warn << "main" << "Simulation interrupted by user" << std::endl;
  sc_stop();
  wait(SC_ZERO_TIME);
}

int sc_main(int argc, char** argv) {
    clock_t cstart, cend;

    (void) signal(SIGINT, stopSimFunction);
    (void) signal(SIGTERM, stopSimFunction);
    (void) signal(10, stopSimFunction);

    gs::ctr::GC_Core       core;
    gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
    gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);

#ifdef HAVE_USI
    // Initialize Python
    USI_HAS_MODULE(systemc_);
    USI_HAS_MODULE(delegate);
    USI_HAS_MODULE(scireg);
    USI_HAS_MODULE(amba);
    USI_HAS_MODULE(report);
    USI_HAS_MODULE(parameter_);
    USI_HAS_MODULE(mtrace);
    usi_init(argc, argv);

    usi_load("usi.api.systemc");
    usi_load("usi.api.delegate");
    usi_load("usi.api.scireg");
    usi_load("usi.api.amba");

    usi_load("usi.log.console_reporter");
    usi_load("usi.tools.args");
    usi_load("usi.cci");
    //usi_load("tools.python.power");
    usi_load("usi.shell");

    usi_start_of_initialization();
#endif  // HAVE_USI

    Leon3mpPlatform leon3mp("leon3mp");

    cstart = cend = clock();
#ifdef HAVE_USI
    usi_start();
#else
    sc_core::sc_start();
#endif
    cend = clock();

    srInfo("main")
      ("start_of_simulation", (uint64_t)cstart)
      ("end_of_simulation", (uint64_t)cend)
      ("Simulation execution time");

    return trap::exitValue;
}
/// @}
