#! /usr/bin/env python
# encoding: utf-8
#*********************************************************************
# Copyright 2010, Institute of Computer and Network Engineering,
#                 TU-Braunschweig
# All rights reserved
# Any reproduction, use, distribution or disclosure of this program,
# without the express, prior written consent of the authors is 
# strictly prohibited.
#
# University of Technology Braunschweig
# Institute of Computer and Network Engineering
# Hans-Sommer-Str. 66
# 38118 Braunschweig, Germany
#
# ESA SPECIAL LICENSE
#
# This program may be freely used, copied, modified, and redistributed
# by the European Space Agency for the Agency's own requirements.
#
# The program is provided "as is", there is no warranty that
# the program is correct or suitable for any purpose,
# neither implicit nor explicit. The program and the information in it
# contained do not necessarily reflect the policy of the European 
# Space Agency or of TU-Braunschweig.
#*********************************************************************
# Title:      wscript
#
# ScssId:
#
# Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
#
# Purpose:    this file contains the build system for the TLM library
#
# Method:     $ ./waf configure; ./waf # to build it
#
# Modified on $Date$
#          at $Revision$
#          by $Author$
#
# Principal:  European Space Agency
# Author:     VLSI working group @ IDA @ TUBS
# Maintainer: Rolf Meyer
# Reviewed:
#*********************************************************************
APPNAME = 'hwswtlmmodels'
VERSION = '0.5'
top = '.'
out = 'build'
dirs = ['outkit', 'signalkit', 'models', 'tests']

import os
import os.path
import fnmatch

from Tools import unittestw

def set_options(ctx): 
  from os import environ
  conf = ctx.get_option_group("--download")
  
  ctx.tool_options('compiler_cxx')
  ctx.tool_options('unittestw')
  ctx.tool_options('common', tooldir='waftools')
  ctx.tool_options('boost', option_group=conf)
  
  #ctx.add_option('--onlytests', action='store_true', default=True, help='Exec unit tests only', dest='only_tests')
  conf.add_option("--cxxflags", dest="cxxflags", help="C++ compiler flags", default=environ.get("CXXFLAGS",""))
  
  sysc = ctx.add_option_group("SystemC configuration Options")
  sysc.add_option("--systemc", dest="systemc_home", help="Basedir of your SystemC installation", default=environ.get("SYSTEMC_HOME",""))
  sysc.add_option("--tlm2", dest="tlm2_home", help="Basedir of your TLM-2.0 distribution", default=environ.get("TLM2_HOME",""))
  sysc.add_option("--tlm2tests", dest="tlm2_tests", help="Example of your TLM-2.0 distribution", default=environ.get("TLM2_TESTS",""))
  sysc.add_option("--scv", dest="scv_home", help="Basedir of your SCV distribution", default=environ.get("SCV_HOME",""))
  
  gso = ctx.add_option_group("GreenSoCs Configuration Options")
  gso.add_option("--greensocs", dest="greensocs_home", help="Basedir of your GreenSoCs instalation", default=environ.get("GREENSOCS_HOME",""))
  gso.add_option("--amba", dest="amba_home", help="Basedir of your AMBAKit distribution", default=environ.get("AMBA_HOME",""))
  gso.add_option("--grlib", dest="grlib_home", help="Basedir of your grlib distribution", default=environ.get("GRLIB",""))

  from waftools.common import get_subdirs
  ctx.recurse(get_subdirs(top))

def configure(ctx):
  from Options import options
  import os.path
  #import waftools
  ctx.env['CXXFLAGS'] += ['-g',
                          '-Wall',
                          '-O2'] + options.cxxflags.split()

  ctx.env['CPPFLAGS'] += ['-D_REENTRANT',
                          '-DUSE_STATIC_CASTS',
                          '-DSC_INCLUDE_DYNAMIC_PROCESSES']

  ctx.check_tool('compiler_cxx')
  ctx.find_program('doxygen', var='DOXYGEN', mandatory=False)
  #ctx.check_tool('doxygen', tooldir='waftools')
  
  ## Modelsim:
  #ctx.check_tool('modelsim', tooldir='wadtools')
  ctx.check_tool('unittestw')
  ctx.check_tool('boost')
  from os import environ
  if not options.boostincludes or options.boostincludes == "":
    options.boostincludes = environ.get("BOOST_DIR","")
    
  if not options.boostlibs or options.boostlibs == "":
    options.boostlibs = environ.get("BOOST_LIB","")
  
  # It seems that we just need the header for static & shared pointer, algorithem trim but libs for program_options in greensocs, tokenizer filesystem and iostream in greenreg
  boostLibs = 'program_options filesystem iostream' # Trap: 'regex thread program_options filesystem system'
  ctx.check_boost(lib=boostLibs, static='both', min_version='1.35.0', mandatory = True, errmsg = 'Unable to find ' + boostLibs + ' boost libraries of at least version 1.35, please install them and specify their location with the --boost-includes and --boost-libs configuration options')
  import glob
  
  # SystemC
  ctx.check_cxx(
    lib          = 'systemc',
    uselib_store = 'SYSC',
    mandatory    = True,
    libpath      = glob.glob(os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(options.systemc_home, "lib-*"))))), # not always lib-linux -> lib-* or lib-<target>
    errormsg     = "SystemC Library not found. Use --systemc option or set $SYSTEMC_HOME."
  ) 
  ctx.check_cxx(
    header_name  = 'systemc.h',
    uselib_store = 'SYSC',
    mandatory    = True,
    includes     = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(options.systemc_home, "include")))),
    uselib       = 'SYSC'
  ) 
  ctx.check_cxx(
    msg          = "Checking for SystemC 2.2.0 or higher",
    mandatory    = True,
    execute      = True,
    fragment     = """
                   #include <systemc.h>
                   int main(int argc, char *argv[]) {
                     return !(SYSTEMC_VERSION > 20070313);
                   }
    """,
    uselib       = 'SYSC',
  )
 
  # TLM 2.0
  ctx.check_cxx(
    header_name  = 'tlm.h',
    uselib_store = 'TLM2',
    mandatory    = True,
    includes     = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(options.tlm2_home, "include/tlm")))),
    uselib       = 'SYSC',
  ) 
  ctx.check_cxx(
    msg          = "Checking for TLM 2.0.x",
    mandatory    = True,
    execute      = True,
    fragment     = """
                   #include <tlm.h>
                   int main(int argc, char *argv[]) {
                     return !((TLM_VERSION_MAJOR == 2) && (TLM_VERSION_MINOR == 0));
                   }
    """,
    uselib       = 'SYSC TLM2',
  )
 
  # TODO: It has to be checked if we realy need them
  # SCV
  #ctx.check_cxx(
  #  lib          = 'scv',
  #  uselib_store = 'SCV',
  #  mandatory    = True,
  #  libpath      = glob.glob(os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(options.scv_home, "lib-linux"))))), # not always lib-linux -> lib-* or lib-<target>
  #  errormsg     = "SystemC Verification Library not found. Use --scv option or set $SYSTEMC_HOME."
  #) 
  #ctx.check_cxx(
  #  header_name  = 'scv.h',
  #  uselib_store = 'SYSC',
  #  mandatory    = True,
  #  includes     = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(options.scv_home, "include")))),
  #  uselib       = 'SYSC SCV'
  #) 
  #ctx.check_cxx(
  #  msg          = "Checking for SCV Library 2.2 or higher",
  #  mandatory    = True,
  #  execute      = True,
  #  fragment     = """
  #                 #include <scv.h>
  #                 int main(int argc, char *argv[]) {
  #                   return !(SC_VERSION > 2001999);
  #                 }
  #  """,
  #  uselib       = 'BOOST SYSC SCV',
  #)
  
  ctx.check_cxx(
    lib          = 'greenreg',
    uselib_store = 'GREENSOCS',
    mandatory    = True,
    libpath      = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(options.greensocs_home, "greenreg")))),
  ) 
  ctx.check_cxx(
    header_name   = "greensocket/initiator/single_socket.h",
    uselib_store  = 'GREENSOCS',
    mandatory     = True,
    includes      = os.path.abspath(os.path.expanduser(os.path.expandvars(options.greensocs_home))),
    uselib        = 'BOOST SYSC TLM2',
  )
  ctx.check_cxx(
    header_name   = "target/single_socket.h",
    uselib_store  = 'GREENSOCS',
    mandatory     = True,
    includes      = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(options.greensocs_home, "greensocket")))),
    uselib        = 'GREENSOCS BOOST SYSC TLM2',
  )
  ctx.check_cxx(
    header_name   = "config.h",
    uselib_store  = 'GREENSOCS',
    mandatory     = True,
    includes      = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(options.greensocs_home, "greencontrol")))),
    uselib        = 'GREENSOCS BOOST SYSC TLM2',
  )
  ctx.check_cxx(
    header_name   = "greenreg.h",
    uselib_store  = 'GREENSOCS',
    mandatory     = True,
    includes      = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(options.greensocs_home, "greenreg")))),
    uselib        = 'GREENSOCS BOOST SYSC TLM2',
  )

  # AMBAKit 
  # TODO: Check for version, its included in the new version
  ctx.check_cxx(
    header_name   = "amba.h",
    uselib_store  = 'AMBA',
    mandatory     = True,
    includes      = os.path.abspath(os.path.expanduser(os.path.expandvars(options.amba_home))),
    uselib        = 'BOOST SYSC TLM2 GREENSOCS',
  )

  # Check for AMBAKit extended GreenSocs
  import Configure
  try:
    ctx.check_cxx(
      header_name   = "greenreg_ambasocket.h",
      uselib_store  = 'GREENSOCS',
      mandatory     = True,
      includes      = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(options.greensocs_home, "greenreg", "greenreg_socket")))),
      uselib        = 'GREENSOCS BOOST SYSC TLM2 AMBA',
    )
  except Configure.ConfigurationError:
    # use own old files
    ctx.env["CPPPATH_GREENSOCS"] += os.path.join(ctx.srcdir,'contrib', 'greensocs-4.0.0_patch'),
    # TODO: Check if it works
  
  # Extend GREENSOCS
  ctx.check_cxx(
    header_name   = [ "tlm.h",
                      "tlm_utils/multi_passthrough_initiator_socket.h",
                      "tlm_utils/multi_passthrough_target_socket.h",
                      "simpleAddressMap.h",
                      "extensionPool.h"
                    ],
    uselib_store  = 'GREENSOCS',
    msg           = "Checking for extensionPool.h from TLM2",
    mandatory     = True,
    includes      = [os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(options.tlm2_tests, "tlm", "multi_sockets", "include")))),
                     os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(options.tlm2_home, "unit_test", "tlm", "multi_sockets", "include"))))],
    uselib        = 'SYSC TLM2 GREENSOCS',
  )
  
  from waftools.common import get_subdirs
  ctx.recurse(get_subdirs(top))
  
  ctx.check_cxx(cxxflags=ctx.env['CXXFLAGS'], mandatory=1, msg='Checking for G++ compilation flags')
  ctx.check_cxx(linkflags=ctx.env['LINKFLAGS'], mandatory=1, msg='Checking for link flags')

def build(bld):
  from waftools.common import get_subdirs
  bld.add_subdirs(get_subdirs())
  bld.add_post_fun(unittestw.summary)

