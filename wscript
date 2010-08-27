#! /usr/bin/env python
# encoding: utf-8
#***********************************************************************#
#* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform     *#
#*                                                                     *#
#* File:       wscript                                                 *#
#*             this file contains the build system for the TLM library *#
#*             $ ./waf configure; ./waf # to build it                  *#
#*                                                                     *#
#* Modified on $Date$   *#
#*          at $Revision$                                         *#
#*                                                                     *#
#* Principal:  European Space Agency                                   *#
#* Author:     VLSI working group @ IDA @ TUBS                         *#
#* Maintainer: Rolf Meyer                                              *#
#***********************************************************************#
APPNAME = 'hwswtlmmodels'
VERSION = '0.5'
top = '.'
out = 'build'
dirs = ['signalkit', 'models', 'tests']

import os
import os.path
import fnmatch
import find

from Tools import unittestw

def set_options(ctx): 
  from os import environ
  ctx.tool_options('compiler_cxx')
  ctx.tool_options('unittestw')
  dep = ctx.add_option_group("Boost Options")
  sysc = ctx.add_option_group("SystemC Options")
  gso = ctx.add_option_group("GreenSoCs Options")
  
  ctx.add_option('--onlytests', action='store_true', default=True, help='Exec unit tests only', dest='only_tests')
  ctx.add_option("--cxxflags", dest="cxxflags", help="C++ compiler flags", default=environ.get("CXXFLAGS",""))
  ctx.add_option("--doxygen", dest="dodoxygen", help="Enable to generate doxygen documentation", default=False, action="store_true")
  dep.add_option("--boost", dest="boost_home", help="Basedir of your Boost installation", default=environ.get("BOOST_HOME",""))
  dep.add_option("--boost_inc", dest="boost_inc", help="Include dir of your Boost installation", default=environ.get("BOOST_DIR",""))
  dep.add_option("--boost_lib", dest="boost_lib", help="Library dir of your Boost installation", default=environ.get("BOOST_LIB",""))
  sysc.add_option("--systemc", dest="systemc_home", help="Basedir of your SystemC installation", default=environ.get("SYSTEMC_HOME",""))
  sysc.add_option("--tlm2", dest="tlm2_home", help="Basedir of your TLM-2.0 distribution", default=environ.get("TLM2_HOME",""))
  sysc.add_option("--tlm2tests", dest="tlm2_tests", help="Example of your TLM-2.0 distribution", default=environ.get("TLM2_TESTS",""))
  sysc.add_option("--scv", dest="scv_home", help="Basedir of your SCV distribution", default=environ.get("SCV_HOME",""))
  gso.add_option("--greensocs", dest="greensocs_home", help="Basedir of your GreenSoCs instalation", default=environ.get("GREENSOCS_HOME",""))
  gso.add_option("--amba", dest="amba_home", help="Basedir of your AMBAKit distribution", default=environ.get("AMBA_HOME",""))
  gso.add_option("--grlib", dest="grlib_home", help="Basedir of your grlib distribution", default=environ.get("GRLIB",""))

  ctx.recurse(dirs)

def configure(ctx):
  from Options import options
  import os.path
  ctx.env['CXXFLAGS'] += ['-g', 
                         '-Wall', 
                         '-D_REENTRANT', 
                         '-DUSE_STATIC_CASTS', 
                         '-DSC_INCLUDE_DYNAMIC_PROCESSES'] + options.cxxflags.split()
  
  ctx.check_tool('compiler_cxx')
  ctx.find_program('doxygen', var='DOXYGEN', mandatory=True)
  #ctx.find_program('vlib', var='VLIB', mandatory=True)
  #ctx.find_program('vsim', var='VSIM', mandatory=True)
  #ctx.find_program('sccom', var='SCCOM', mandatory=True)
  #ctx.find_program('scgenmod', var='SCGENMOD', mandatory=True)
  #ctx.find_program('vcom', var='VCOM', mandatory=True)
  ctx.check_tool('unittestw')
  

  GREENSOCS_INC = [".",
                   "greensocket",
                   "gsgpsocket",
                   "greencontrol",
                   os.path.join("signalsocket","green-signal-socket","include"),
                   "greenreg"]
           
  ctx.env["GREENREGROOT"] = os.path.join(options.greensocs_home, 'greenreg')
  if options.greensocs_home and options.greensocs_home != "":
    ctx.env["CPPPATH_GREENSOCS"] = [os.path.join(options.greensocs_home, n) for n in GREENSOCS_INC] + find.getdirs(ctx.env["GREENREGROOT"], ['*test*', '*examples*'])
 
    ctx.env["LIBPATH_GREENSOCS"] =  os.path.join(options.greensocs_home, "greenreg")
    ctx.env["LIB_GREENSOCS"] = "greenreg"
    
  if options.boost_home and options.boost_home != "":
    #ctx.env["CXXFLAGS_BOOST"]
    ctx.env["CPPPATH_BOOST"] = os.path.join(options.boost_home, "include")
    ctx.env["LIBPATH_BOOST"] =  os.path.join(options.boost_home, "lib")
    #ctx.env["LIB_BOOST"] = "boost"
  elif options.boost_inc and options.boost_lib and options.boost_inc != "" and options.boost_inc != "":
    ctx.env["CPPPATH_BOOST"] = options.boost_inc
    ctx.env["LIBPATH_BOOST"] =  options.boost_lib
    #ctx.env["LIB_BOOST"] = "boost"
  
  if options.systemc_home and options.systemc_home != "":
    ctx.env["CPPPATH_SYSC"] = os.path.join(options.systemc_home, "include")
    ctx.env["LIBPATH_SYSC"] =  os.path.join(options.systemc_home, "lib-linux")
    ctx.env["LIB_SYSC"] = "systemc"
    
  if options.tlm2_home and options.tlm2_home != "":
    ctx.env["CPPPATH_TLM2"] = os.path.join(options.tlm2_home, "include/tlm")
    
  if options.scv_home and options.scv_home != "":
    ctx.env["CPPPATH_SCV"] = os.path.join(options.scv_home, "include")
    ctx.env["LIBPATH_SCV"] =  os.path.join(options.scv_home, "lib-linux")
    ctx.env["LIB_SCV"] = "scv"
    
  if options.amba_home and options.amba_home != "":
    ctx.env["CPPPATH_AMBA"] = options.amba_home
  
  if options.tlm2_tests and options.tlm2_tests != "":
    ctx.env["CPPPATH_GREENSOCS"] += [os.path.join(options.tlm2_tests, "tlm", "multi_sockets", "include")]
  elif options.tlm2_home and options.tlm2_home != "":
    ctx.env["CPPPATH_GREENSOCS"] += [os.path.join(options.tlm2_home, "unit_test", "tlm", "multi_sockets", "include")]
    
  ctx.check_cxx(
    msg          = "Checking for SystemC 2.2.0 or higher",
    mandatory    = True,
    execute      = True,
    fragment     = """
                   #include <systemc.h>
                   int main(int argc, char *argv) {
                     return !(SYSTEMC_VERSION > 20070313);
                   }
    """,
    uselib       = 'SYSC',
  )
  
  ctx.check_cxx(
    msg          = "Checking for TLM 2.0.x",
    mandatory    = True,
    execute      = True,
    fragment     = """
                   #include <tlm.h>
                   int main(int argc, char *argv) {
                     return !((TLM_VERSION_MAJOR == 2) && (TLM_VERSION_MINOR == 0));
                   }
    """,
    uselib       = 'SYSC TLM2',
  )
  
  ctx.check_cxx(
    msg          = "Checking for SCV Library 2.2 or higher",
    mandatory    = True,
    execute      = True,
    fragment     = """
                   #include <scv.h>
                   int main(int argc, char *argv) {
                     return !(SC_VERSION > 2001999);
                   }
    """,
    uselib       = 'SCV SYSC',
  )
  
  ctx.check_cxx(
    msg          = "Checking for Boost 1.37 or higher",
    mandatory    = True,
    execute      = True,
    fragment     = """
                   #include <boost/version.hpp>
                   int main(int argc, char *argv) {
                     return !(BOOST_VERSION > 103699);
                   }
    """,
    uselib       = 'BOOST',
  )
  

  ctx.check(
    compiler_mode = 'cxx',
    header_name   = "greenreg.h",
    msg           = "Checking for GreenSocs Header",
    mandatory     = True,
    uselib        = 'GREENSOCS BOOST SYSC TLM2',
  )
  
  ctx.check(
    compiler_mode = 'cxx',
    header_name   = "amba.h",
    msg           = "Checking for AMBAKit Header",
    mandatory     = True,
    uselib        = 'GREENSOCS AMBA BOOST SYSC TLM2',
  )
  
  ctx.check(
    compiler_mode = 'cxx',
    header_name   = [ "tlm.h",
                      "tlm_utils/multi_passthrough_initiator_socket.h",
                      "tlm_utils/multi_passthrough_target_socket.h",
                      "simpleAddressMap.h",
                      "extensionPool.h"
                    ],
    
    msg           = "Checking for extensionPool.h from TLM2",
    mandatory     = True,
    uselib        = 'GREENSOCS SYSC TLM2',
  )
  
  ctx.recurse(dirs)

def build(bld):
  bld.add_subdirs(dirs)
  bld.add_post_fun(unittestw.summary)

def docs(bld):
  import subprocess
  #sources = find.getfiles(
  #    '.',
  #    ['*.c', '*.cpp', '*.cxx', '*.h', '*.hpp', '*.tpp'], 
  #    ['.hg', '.svn', 'build']
  #)
  subprocess.call(["doxygen", "Doxyfile"])  

## Nice to have to set svn props:
## It's maybe wothy to make a target out of it
# grep --exclude=**.svn** -rn '\$Date\$' tlmsignals models | cut -f1 -d: | xargs -I {} svn propset svn:keywords "Date Revision" {}
