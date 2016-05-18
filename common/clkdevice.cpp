// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup utils
/// @{
/// @file clkdevice.cpp
/// implementation of clkdevice base class
///
/// @date 2010-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#include "core/common/clkdevice.h"
#include "core/common/verbose.h"

CLKDevice::CLKDevice() :
  rst(&CLKDevice::onrst, "rst"),
  clk(&CLKDevice::onclk, "clk"),
  clock_cycle(10, sc_core::SC_NS) {
}


CLKDevice::~CLKDevice() {
}

void CLKDevice::onrst(const bool &value, const sc_core::sc_time &time) {
  // The reset is active while the input is false.
  // On the rising edge we want to perform the reset:
  if (value == true) {
    v::debug << "CLKDevice" << "Initiating Reset" << v::endl;
    dorst();
  }
}

void CLKDevice::onclk(const sc_core::sc_time &value, const sc_core::sc_time &time) {
  clock_cycle = value;
  clkcng();
}

// Extract basic cycle rate from a sc_clock
void CLKDevice::set_clk(sc_core::sc_clock &clk) {  // NOLINT(runtime/references)
  clock_cycle = clk.period();
  clkcng();
}

// Extract basic cycle rate from a clock period
void CLKDevice::set_clk(sc_core::sc_time period) {
  clock_cycle = period;
  clkcng();
}

// Extract basic cycle rate from a clock period in double
void CLKDevice::set_clk(double period, sc_core::sc_time_unit base) {
  clock_cycle = sc_time(period, base);
  clkcng();
}

// Extract basic cycle rate from a clock period in double
sc_core::sc_time &CLKDevice::get_clk() {
  return clock_cycle;
}

void CLKDevice::dorst() {
  v::debug << "CLKDevice" << "Doing Reset" << v::endl;
}

/// @}
