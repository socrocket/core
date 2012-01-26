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
  pass

def configure(ctx):
  pass

@TaskGen.feature('generate')
def make_generate(self):
  # Put the execution of the generate task directly in here if any problems happen
  # Be careful only to execute the test if the platform is not jet generated
  
  l = self.target.split('.')
  template = l[0]
  load = l[1] or None

  src = self.bld.srcnode.find_resource(os.path.join("templates", template + ".tpa"))
  dstpath = os.path.join(self.bld.srcnode.abspath(),("platforms/%s-%s" % (template, load)))
  if os.path.exists(src.abspath()):
    if not os.path.isdir(dstpath):
      import subprocess
      print "Generate Platform"
      cmd = """from generator.wizard import main; main("%s", "%s")""" % (template, load)
      subprocess.call(["python", "-c", cmd ], cwd=self.bld.srcnode.abspath())
      self.bld.recurse(dstpath)
  else:
    print "Platform is not found!"

# Extended Testing support
def make_systest(self):
  sysname = getattr(self, 'system', None)
  romname = getattr(self, 'prom', getattr(self, 'rom', None)) 
  ramname = getattr(self, 'ram', getattr(self, 'sram', getattr(self, 'sdram', None)))
  
  if sysname and romname and ramname:
    systgen = self.bld.get_tgen_by_name(sysname)
    sys = systgen.path.find_or_declare(sysname)
    romtgen = self.bld.get_tgen_by_name(romname)
    rom = romtgen.path.find_or_declare(romname)
    ramtgen = self.bld.get_tgen_by_name(ramname)
    ram = ramtgen.path.find_or_declare(ramname)
    
    test = self.create_task('utest', [ram, rom, sys])
    test.filename = getattr(self, 'name', getattr(self, 'target', ramname))
    test.ut_exec = [sys.abspath(), rom.abspath(), ram.abspath()]

from waflib.TaskGen import feature,after_method
feature('systest')(make_systest)
after_method('apply_link')(make_systest)
