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
import Task
import TaskGen
import Utils
import os, io

"""
The purpose of this file is to enable waf to compile in modelsim.
To implement file handler for vhdl and verilog files is easy (see modelsim_vcom, modelsim_vlog)
More complicated is the implementation of a systemc compiler. The integration is disturebed by 
the cpp-File Handler of the c complier. wich might be used in the same prject for other targets.

Therfore we need to abuse som waf internals. We remove the cpp handler from our Target TaskGen 
object and replace it with our own (modelsim_sccom)

TODO: Dependencies between targets are not resolved in the right way.
TODO: VARIABLES seems to be inhereted between TaskGens. They double each target.
TODO: uselib_local integration
TODO: ensure that the env variables are only for modelsim tarets
TODO: Try to make targetfiles for all tasks or to ensure correct recompilation behaviour somehow else.
TODO: Make this usable
TODO: Make flags changable
TODO: Add testig for flags
TODO: Integrate CXXFlags ???
"""
def options(opt):
  pass

def configure(ctx):
  """Detect modelsim executables and set default flags"""
  ctx.find_program('vlib', var='VLIB', mandatory=True)
  ctx.env['VLIBFLAGS'] = []
  
  ctx.find_program('vsim', var='VSIM', mandatory=True)
  ctx.env['VSIMFLAGS'] = ['-quiet']
  
  ctx.find_program('sccom', var='SCCOM', mandatory=True)
  ctx.env['SCCOMFLAGS'] = ['-nologo']
  ctx.env['SCLINKFLAGS'] = ['-nologo']
  
  ctx.find_program('scgenmod', var='SCGENMOD', mandatory=True)
  ctx.env['SCGENMODFLAGS'] = []
  ctx.env['SCGENMODMAP'] = ['-bool', '-map', "std_logic_vector=sc_uint", '-createtemplate']
  
  ctx.find_program('vcom', var='VCOM', mandatory=True)
  ctx.env['VCOMFLAGS'] = ['-quiet']
  
  ctx.find_program('vlog', var='VLOG', mandatory=True)
  ctx.env['VLOGFLAGS'] = ['-quiet']

class vini_task(Task.Task):
  """Writes a modelsim.ini for a target from another ini or a string"""
  name = 'vini'
  color = 'BLUE'
  before = ['vcom', 'vlog', 'vsim_before', 'vsim_after', 'sccom', 'sclink']
  quiet = True

  def run(self):
      from ConfigParser import ConfigParser
      read = ConfigParser({"root": self.root, "path" : self.path, "target": self.target})
      write = ConfigParser({"root": self.root, "path" : self.path, "target": self.target})
      oname = self.outputs[0].get_bld().abspath()
      if isinstance(self.config, str):
        read.readfp(io.BytesIO(self.config))
      else:
        read.read(self.config.abspath())
      
      for sec in read.sections():
        write.add_section(sec)
        for key, val in read.items(sec):
          write.set(sec, key, val)

      with open(oname, 'wb') as configfile:
        write.write(configfile)
      return 0

# Task to create a modelsim work library
vlib_task  = Task.simple_task_type('vlib', 'test -d ${TGT} || vlib ${TGT}', color='BLUE', before=['vcom', 'vlog', 'sccom', 'sclink'], shell=True)
vlib_task.quiet = True
def vlib_task_str(self):
  return "vlib: %s\n" % self.target
vlib_task.__str__ = vlib_task_str

# Task to execute a do script before any other work on the target is done
vsim_before  = Task.simple_task_type('vsim_before', '${VSIM} ${VSIMFLAGS} ${VSIMBEFORE}', color='RED', after=['vlib'], before=['vcom', 'vlog', 'sccom'], vars=['VSIM', 'VSIMFLAGS', 'VSIMBEFORE'])

#Task to execute a do stript after all other work is done on the target
vsim_after   = Task.simple_task_type('vsim_after',  '${VSIM} ${VSIMFLAGS} ${VSIMAFTER}', color='RED', after=['vcom', 'vlog', 'sccom', 'sclink'], vars=['VSIM', 'VSIMFLAGS', 'VSIMAFTER'])
def vsim_task_str(self):
  return "vsim: do %s -> %s\n" % (self.dotask, self.target)
vsim_before.__str__ = vsim_task_str
vsim_after.__str__ = vsim_task_str

# Task to compile vhdl files
vcom_task  = Task.simple_task_type('vcom', '${VCOM} ${VCOMFLAGS} ${SRC}', color='GREEN', before='cxx', ext_in=".vhd", vars=['VCOM', 'VCOMFLAGS'])
vcom_task.quiet = True
def vcom_task_str(self):
  ins = []
  for inf in self.inputs:
    ins.append(inf.name)
  return "vcom: %s -> %s\n" % (', '.join(ins), self.target)
vcom_task.__str__ = vcom_task_str

# task to compile v files
vlog_task  = Task.simple_task_type('vlog', '${VLOG} ${VLOGFLAGS} ${SRC}', color='GREEN', before='cxx', ext_in=".v")
vlog_task.quiet = True
def vlog_task_str(self):
  ins = []
  for inf in self.inputs:
    ins.append(inf.name)
  return "vlog: %s -> %s\n" % (', '.join(ins), self.target)
vlog_task.__str__ = vlog_task_str

# Task to generate systemc headers out of a work library
scgenmod_task  = Task.simple_task_type('scgenmod', '${SCGENMOD} ${SCGENMODFLAGS} ${SCGENMODMAP} ${src} > ${TGT}', color='BLUE', after=['vlib', 'vlog', 'vcom'], before=['sccom'], ext_out=".h", vars=['SCGENMOD', 'SCGENMODFLAGS', 'SCGENMAP'])
scgenmod_task.quiet = True

# task to compile systemc files
#sccom_task = Task.simple_task_type('sccom', '${SCCOM} ${SCCOMFLAGS} ${CPPFLAGS} ${CPPFLAGS} ${CXXFLAGS} ${INCLUDES} ${SRC}', color='BLUE', ext_in=".cpp", after=['vlib', 'vcom', 'vlog', 'scgenmod'], before=['sclink'], vars=['SCCOM', 'SCCOMFLAGS', 'CPPFLAGS', 'CXXFLAGS', 'INCLUDES'])
sccom_task = Task.simple_task_type('sccom', '${SCCOM} ${SCCOMFLAGS} ${SRC}', color='BLUE', ext_in=".cpp", after=['vlib', 'vcom', 'vlog', 'scgenmod'], before=['sclink'], vars=['SCCOM', 'SCCOMFLAGS', 'CPPFLAGS', 'CXXFLAGS', 'INCLUDES'])
sccom_task.quiet = True
def sccom_task_str(self):
  ins = []
  for inf in self.inputs:
    ins.append(inf.name)
  return "sccom: %s -> %s\n" % (', '.join(ins), self.target)
sccom_task.__str__ = sccom_task_str

# task to link systemc files
sclink_task = Task.simple_task_type('sclink', '${SCCOM} ${SCLINKFLAGS} -link', color='YELLOW', after=['vlib', 'vcom', 'vlog', 'sccom'], vars=['SCCOM', 'SCLINKFLAGS', 'LIBRARIES'])
def sclink_task_str(self):
  return "sc_link: %s\n" % self.target
sclink_task.__str__ = sclink_task_str

@TaskGen.before('apply_core')
@TaskGen.feature('modelsim')
def modelsim(self):
  """Apply Variables for the Target create vlib task and 
     vini task as well as dobefore and doafter tasks if needed
  """
  # cxx hack replaces the cpp routine
  setattr(self,modelsim_sccom.__name__,modelsim_sccom)
  self.mappings['.cpp']=modelsim_sccom
  
  self.env["VCOMFLAGS"] += ['-work', self.target]
  self.env["VLOGFLAGS"] += ['-work', self.target]
  self.env["SCCOMFLAGS"] += ['-work', self.target]
  self.env["SCGENMODFLAGS"] += ['-lib', self.target]
  self.env["VSIMFLAGS"] += ['-c', '-lib', self.target]
  self.env["SCLINKFLAGS"] += ['-work', self.target]
    
  # Collect sccom flags from defines includes and uselib
  if hasattr(self, "defines"):
    for define in Utils.to_list(self.defines):
      self.env["SCCOMFLAGS"] += ['-D%s' % define]
      self.env["VLINKFLAGS"] += ['-D%s' % define]
      
  if hasattr(self, "includes"):
    for inc in Utils.to_list(self.includes):
      i = self.path.find_dir(inc)
      if i:
        self.env["SCCOMFLAGS"] += ['-I%s' % i.abspath()]
      
  if hasattr(self, "uselib"):
    for lib in Utils.to_list(self.uselib):
      for path in self.env["INCLUDES_%s" % lib]:
        self.env["SCCOMFLAGS"] += ['-I%s' % path]
      for path in self.env["LIBPATH_%s" % lib]:
        self.env["VLINKFLAGS"] += ['-L%s' % path]
      for lib in self.env["LIB_%s" % lib]:
        self.env["VLINKFLAGS"] += ['-l%s' % lib]
  
  #if hasattr(self, "use"):
  #  for lib in Utils.to_list(self.use):
  #    print lib
  #    print dir(self.bld)
      #for path in self.env["INCLUDES_%s" % lib]:
      #  self.env["SCCOMFLAGS"] += ['-I%s' % path]
      #for path in self.env["LIBPATH_%s" % lib]:
      #  self.env["VLINKFLAGS"] += ['-L%s' % path]
      #for lib in self.env["LIB_%s" % lib]:
      #  self.env["VLINKFLAGS"] += ['-l%s' % lib]
   
  # Create vlib task on target
  workdir = self.path.find_or_declare(self.target)
  vlib = self.create_task('vlib', tgt=[workdir])
  vlib.target = self.target

  self.mdeps = []
  if hasattr(self, "after"):
    lst = [n for n in self.bld.all_task_gen if "modelsim" in n.features and n.target == self.after]
    for l in lst:
      for t in l.tasks:
        if len(t.outputs) > 0:
          self.mdeps += t.outputs

  # If given generate scgenmod tasks to get systemc header
  if hasattr(self, "generate"):
    for p in Utils.to_list(self.generate):
      ls = p.split(':')
      if len(ls)!=2:
        print "a generator pair has to be like '<object>:<header.h>'"
        import sys
        sys.exit(1)
    tgt = self.path.find_or_declare(ls[1])
    env = self.env.derive()
    env.append_value("src", ls[0])
    mod = self.create_task('scgenmod', tgt=tgt)
    mod.env = env
    self.env["SCCOMFLAGS"] += ['-I%s' % tgt.parent.bldpath()]
    mod.dep_nodes += self.mdeps

  # Create modelsim ini task if needed
  tgt = None
  if hasattr(self, "config"):
    tgt = self.path.find_or_declare("modelsim_%s.ini" % self.target)
    tsk = self.create_task('vini', tgt=tgt)
    tsk.root = self.bld.out_dir
    tsk.path = self.path.get_bld().abspath()
    tsk.target = self.target
    tsk.config = self.path.find_resource(self.config) or self.config
    tsk.set_run_after(vlib)
    ini = tgt.get_bld().abspath()
    self.env["VSIMFLAGS"] += ['-modelsimini', ini]
    self.env["VCOMFLAGS"] += ['-modelsimini', ini]
    self.env["VLOGFLAGS"] += ['-modelsimini', ini]
    self.env["SCCOMFLAGS"] += ['-modelsimini', ini]
    self.env["SCGENMODFLAGS"] += ['-modelsimini', ini]
    self.env["SCLINKFLAGS"] += ['-modelsimini', ini]
 
  # Create dobefore task if needed
  if hasattr(self, "dobefore"):
    do = self.path.find_resource(self.dobefore)
    if do:
      do = do.abspath()
    else:
      do = self.dobefore
    self.env["VSIMBEFORE"] += ['-do', do]
    before = self.create_task('vsim_before')
    before.dep_nodes += self.mdeps
    if tgt:
      before.dep_nodes += [tgt]
    before.dotask = self.dobefore
    before.target = self.target
    
  # Create doafter task if needed
  if hasattr(self, "doafter"):
    do = self.path.find_resource(self.doafter)
    if do:
      do = do.abspath()
    else:
      do = self.doafter
    self.env["VSIMAFTER"] += ['-do', do]
    after = self.create_task('vsim_after')
    after.dep_nodes += self.mdeps
    if tgt:
      after.dep_nodes += [tgt]
    after.dotask = self.doafter
    after.target = self.target  
   
@TaskGen.extension('.vhd')
def modelsim_vcom(self, node):
  """Create a vcom_task on each vhd file and ensure the order"""
  tsks = [n for n in self.tasks if n.name in ['vcom', 'vlog']]
  tsk = self.create_task('vcom', [node])
  tsk.target = self.target
  if tsks:
    tsk.run_after.append(tsks[-1])
  
  tsk.dep_nodes += self.mdeps
    
@TaskGen.extension('.v')
def modelsim_vlog(self, node):
  """Create a vlog_task on each v file and ensure the order"""
  tsks = [n for n in self.tasks if n.name in ['vcom', 'vlog']]
  tsk = self.create_task('vlog', [node])
  if tsks:
    tsk.run_after.append(tsks[-1])

  tsk.dep_nodes += self.mdeps

#@TaskGen.extension('.cpp')
def modelsim_sccom(self, node):
  """Create a sccom_task on each cpp file and create a system c link task as well"""
  if 'modelsim' in self.features:
    tsks = [n for n in self.tasks if n.name == 'sccom']
    tsk = self.create_task('sccom', [node])
    tsk.target = self.target
    if tsks:
      tsk.run_after.append(tsks[-1])
    else:
      lnk = self.create_task('sclink')
      lnk.target = self.target
      
    tsk.dep_nodes += self.mdeps

