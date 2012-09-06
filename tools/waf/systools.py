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
# Title:      modelsim.py
#
# ScssId:
#
# Origin:     HW-SW SystemC Co-Simulation SoC Validation Platform
#
# Purpose:    A build system integration for modelsim.
#
# Method:     $ ./waf configure; ./waf # to build it
#
# Modified on $Date: 2010-10-07 17:40:12 +0200 (Thu, 07 Oct 2010) $
#          at $Revision: 159 $
#          by $Author$
#
# Principal:  European Space Agency
# Author:     VLSI working group @ IDA @ TUBS
# Maintainer: Rolf Meyer
# Reviewed:
#*********************************************************************
from waflib import Task
from waflib import TaskGen
from waflib import Options
from waflib.Configure import conf
from waflib import Context
import Utils
import os, io, sys, stat
import datetime, time

def options(opt):
  conf = opt.get_option_group("--download")
  conf.add_option('--nosystests', dest='systests', action='store_false', default=True, help='Deactivates all tests executed on a platform')

def configure(ctx):
  ctx.env["SYSTESTS"] = Options.options.systests

# Extended Testing support
def make_systest(self):
  if not (Options.options.systests and self.env["SYSTESTS"]):
    return

  sysname = getattr(self, 'system', None)
  romname = getattr(self, 'prom', getattr(self, 'rom', None)) 
  sdramname = getattr(self, 'ram', getattr(self, 'sdram', None))
  sramname = getattr(self, 'sram', None)
  at = getattr(self, 'at', False)
  atstr="lt"
  atbool="false"
  param = Utils.to_list(getattr(self, 'args', getattr(self, 'param', getattr(self, 'ut_param', []))))
  
  if at:
    atstr="at"
    atbool="true"

  if sysname and romname and (sdramname or sramname):
    
    systgen = self.bld.get_tgen_by_name(sysname)
    sys = systgen.path.find_or_declare(sysname)

    romtgen = self.bld.get_tgen_by_name(romname)
    rom = romtgen.path.find_or_declare(romname)

    exec_list = [sys.abspath(), "--option", "conf.mctrl.prom.elf=%s" % (rom.abspath())]
    deps_list = [sys, rom]
    filename = ""

    if sdramname:
      ramtgen = self.bld.get_tgen_by_name(sdramname)
      ram = ramtgen.path.find_or_declare(sdramname)

      exec_list.append("--option")
      exec_list.append("conf.mctrl.ram.sdram.elf=%s" % (ram.abspath()))
      exec_list.append("--option")
      exec_list.append("conf.system.osemu=%s" % (ram.abspath()))
      exec_list.append("--option")
      exec_list.append("conf.system.log=%s-%s" % (ram.abspath(), atstr))

      deps_list.append(ram)

      filename = getattr(self, 'name', getattr(self, 'target', "%s-%s" % (sdramname, atstr)))

    else:
      ramtgen = self.bld.get_tgen_by_name(sramname)
      ram = ramtgen.path.find_or_declare(sramname)

      exec_list.append("--option")
      exec_list.append("conf.mctrl.ram.sram.elf=%s" % (ram.abspath()))
      exec_list.append("--option")
      exec_list.append("conf.system.osemu=%s" % (ram.abspath()))
      exec_list.append("--option")
      exec_list.append("conf.system.log=%s-%s" % (ram.abspath(), atstr))

      deps_list.append(ram)

      filename = getattr(self, 'name', getattr(self, 'target', "%s-%s" % (sramname, atstr)))

    exec_list.append("--option")
    exec_list.append("conf.system.at=%s" % (atbool))

    test = self.create_task('utest', deps_list)
    test.filename = filename
    test.ut_exec = exec_list + param

from waflib.TaskGen import feature,after_method
feature('systest')(make_systest)
after_method('apply_link')(make_systest)
