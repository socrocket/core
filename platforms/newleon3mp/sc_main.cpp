// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup platforms
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

#include "common/gs_config.h"
#include "common/systemc.h"
#include "common/report.h"

#include "platforms/newleon3mp/leon3mp.h"

namespace trap {
  extern int exitValue;
};

void stopSimFunction( int sig ){
  v::warn << "main" << "Simulation interrupted by user" << std::endl;
#ifndef HAVE_PYTHON
  PythonModule::signal(sig);
#else
  sc_stop();
  wait(SC_ZERO_TIME);
#endif  // HAVE_PYTHON
}

int sc_main(int argc, char** argv) {
    clock_t cstart, cend;

    (void) signal(SIGINT, stopSimFunction);
    (void) signal(SIGTERM, stopSimFunction);
    (void) signal(10, stopSimFunction);

    gs::ctr::GC_Core       core;
    gs::cnf::ConfigDatabase cnfdatabase("ConfigDatabase");
    gs::cnf::ConfigPlugin configPlugin(&cnfdatabase);

#ifdef HAVE_PYSC
    PythonModule python("python_interpreter", NULL, argc, argv);

    python.load("tools.python.arguments");
    python.load("tools.python.config");
    python.load("tools.python.power");

    python.start_of_initialization();
#endif  // HAVE_PYSC

    Leon3mpPlatform leon3mp("leon3mp");

#ifdef HAVE_PYSC
    python.end_of_initialization();
#endif
    cstart = cend = clock();
    sc_core::sc_start();
    cend = clock();

    srInfo("main")
      ("start_of_simulation", (uint64_t)cstart)
      ("end_of_simulation", (uint64_t)cend)
      ("Simulation execution time");

#ifdef HAVE_PYSC
    python.start_of_evaluation();
    python.end_of_evaluation();
#endif
    return trap::exitValue;
}
/// @}
