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

"""
The purpose of this file is to enable waf to compile in modelsim.
To implement file handler for vhdl and verilog files is easy (see modelsim_vcom, modelsim_vlog)
More complicated is the implementation of a systemc compiler. The integration is disturebed by 
the cpp-File Handler of the c complier. wich might be used in the same prject for other targets.

Therfore we need to abuse som waf internals. We remove the cpp handler from our Target TaskGen 
object and replace it with our own (modelsim_sccom)

TODO: VARIABLES seems to be inhereted between TaskGens. They double each target.
TODO: uselib_local integration
TODO: Add testig for flags
"""
def options(opt):
  conf = opt.get_option_group("--download")
  conf.add_option('--nomodelsim', dest='modelsim', action='store_false', default=True, help='Deactivates the build of all modelsim featured targets')

def configure(ctx):
  """Detect modelsim executables and set default flags"""
  if not Options.options.modelsim:
    ctx.env["MODELSIM"] = False
  else: 
    try:
      ctx.find_program('vlib', var='VLIB', okmsg="ok")
      ctx.env['VLIBFLAGS'] = []
      
      ctx.find_program('vsim', var='VSIM', okmsg="ok")
      ctx.env['VSIMFLAGS'] = ['-quiet']
      
      ctx.find_program('sccom', var='SCCOM', okmsg="ok")
      ctx.env['SCCOMFLAGS'] = ['-nologo']
      ctx.env['SCLINKFLAGS'] = ['-nologo']
      
      ctx.find_program('scgenmod', var='SCGENMOD', okmsg="ok")
      ctx.env['SCGENMODFLAGS'] = []
      ctx.env['SCGENMODMAP'] = ['-bool', '-map', "std_logic_vector=sc_uint", '-createtemplate']
      
      ctx.find_program('vcom', var='VCOM', okmsg="ok")
      ctx.env['VCOMFLAGS'] = ['-quiet']
      
      ctx.find_program('vlog', var='VLOG', okmsg="ok")
      ctx.env['VLOGFLAGS'] = ['-quiet']
      ctx.find_sc_dir()
      ctx.env["MODELSIM"] = True
    except ctx.errors.ConfigurationError:
      ctx.env["MODELSIM"] = False

def targetdir(target):
  return target.replace('.', '_')+"_work"

FIND_SC_DIR_CODE = """
#include <systemc.h>

int main(int argc, char *argv[]) {
  return 0;
}
"""
def find_sc_dir(self):
  self.start_msg('Checking for sccom backend')
  h=Utils.h_list('modelsim')
  self.to_log("checking for modelsim systemc compiler dir")
  dir=self.bldnode.abspath()+os.sep+(sys.platform!='win32'and'.'or'')+'conf_check_'+Utils.to_hex(h)
  try:
    os.makedirs(dir)
  except:
    pass
  try:
    os.stat(dir)
  except:
    self.fatal('cannot use the configuration test folder %r'%dir)
  cachemode=getattr(Options.options,'confcache',None)
  owd = os.getcwd()
  os.chdir(dir)
  self.to_log("writing main.cpp >>%s<<" % FIND_SC_DIR_CODE)
  main = open("main.cpp", 'w')
  main.write(FIND_SC_DIR_CODE)
  main.close()
  if not os.path.isdir('work'):
    self.to_log("creating work directory with vlib")
    self.cmd_and_log(['vlib', 'work'], output=Context.STDOUT, quiet=Context.BOTH)
  if not os.path.isdir(os.path.join('work', '_sc')):
    self.to_log("compiling systemc main file with sccom")
    self.cmd_and_log(['sccom', '-work', 'work', 'main.cpp'], output=Context.STDOUT, quiet=Context.BOTH)
  self.to_log("serching for compiler path inside the work directory")
  dirs = [n for n in os.listdir(os.path.join('work', '_sc')) if os.path.isdir(os.path.join('work', '_sc', n))]
  if len(dirs) < 1:
    self.fatal("Error no systemc directory created")
  self.to_log("Found systemc compiler dir: %s" % dirs[0])
  self.env["VSIM_SC_DIR"] = dirs[0]
  os.chdir(owd)
  self.end_msg('ok (%s)' % (self.env["VSIM_SC_DIR"]))
  return 0

conf(find_sc_dir)

class vini_task(Task.Task):
  """Writes a modelsim.ini for a target from another ini or a string"""
  name = 'vini'
  color = 'BLUE'
  before = ['vcom', 'vlog', 'vsim_before', 'vsim_after', 'sccom', 'sclink']
  quiet = True
  def __str__(self):
      return "vini: string -> %s\n" % (self.outputs[0].name)
    
  def run(self):
      from ConfigParser import ConfigParser
      read = ConfigParser()
      write = ConfigParser()
      oname = self.outputs[0].get_bld().abspath()
      if isinstance(self.config, str):
        read.readfp(io.BytesIO(self.config))
      else:
        read.read(self.config.abspath())
      
      for sec in read.sections():
        write.add_section(sec)
        for key, val in read.items(sec, False, {"root": self.root, "path" : self.path, "target": targetdir(self.target)}):
          write.set(sec, key, val)

      with open(oname, 'wb') as configfile:
        write.write(configfile)
      return 0

# Task to create a modelsim work library
vlib_task  = Task.simple_task_type('vlib', 'vlib ${TGT}', color='BLUE', before=['vcom', 'vlog', 'sccom', 'sclink'], shell=True)
vlib_task.quiet = True

def vlib_task_str(self):
  return "vlib: create %s ...\n" % self.target
vlib_task.__str__ = vlib_task_str

def vlib_task_runnable_status(self):
  if os.path.exists(os.path.join(self.outputs[0].abspath(), '_info')):
    return Task.SKIP_ME # ASK_LATER
  return Task.RUN_ME
vlib_task.runnable_status = vlib_task_runnable_status

# Task to execute a do script before any other work on the target is done
vsim_before  = Task.simple_task_type('vsim_before', '${VSIM} ${_VSIMFLAGS} ${_VSIMBEFORE}', color='RED', after=['vlib'], before=['vcom', 'vlog', 'sccom'], vars=['VSIM', '_VSIMFLAGS', '_VSIMBEFORE'])

#Task to execute a do stript after all other work is done on the target
vsim_after   = Task.simple_task_type('vsim_after',  '${VSIM} ${_VSIMFLAGS} ${_VSIMAFTER}', color='RED', after=['vcom', 'vlog', 'sccom', 'sclink'], vars=['VSIM', '_VSIMFLAGS', '_VSIMAFTER'])
def vsim_task_str(self):
  return "vsim: do %s -> %s\n" % (self.dotask, self.target)
vsim_before.__str__ = vsim_task_str
vsim_after.__str__ = vsim_task_str

# Task to compile vhdl files
vcom_task  = Task.simple_task_type('vcom', 'date +\'%%F %%r %%s\' > ${TGT[0].abspath()} && ${VCOM} ${_VCOMFLAGS} ${SRC[0].abspath()}', color='GREEN', shell=True, before=['scgenmod', 'sccom'], ext_in=".vhd", vars=['VCOM', '_VCOMFLAGS'])
#vcom_task  = Task.simple_task_type('vcom', '${VCOM} ${_VCOMFLAGS} ${SRC}', color='GREEN', before=['scgenmod', 'sccom'], ext_in=".vhd", vars=['VCOM', '_VCOMFLAGS'])
vcom_task.quiet = True
vcom_task.nocache = True
def vcom_task_str(self):
  ins = []
  return "vcom: %s -> %s\n" % (self.inputs[0].name, self.target)
vcom_task.__str__ = vcom_task_str

# task to compile v files
vlog_task  = Task.simple_task_type('vlog', '${VLOG} ${_VLOGFLAGS} ${SRC[0].abspath()}', color='GREEN', before=['scgenmod', 'sccom'], ext_in=".v", vars=['VLOG', '_VLOGFLAGS'])
vlog_task.quiet = True
vlog_task.nocache = True
def vlog_task_str(self):
  ins = []
  for inf in self.inputs:
    ins.append(inf.name)
  return "vlog: %s -> %s\n" % (', '.join(ins), self.target)
vlog_task.__str__ = vlog_task_str

# Task to generate systemc headers out of a work library
scgenmod_task  = Task.simple_task_type('scgenmod', '${SCGENMOD} ${_SCGENMODFLAGS} ${SCGENMODMAP} ${src} > ${TGT[0].abspath()}', color='BLUE', after=['vlib', 'vlog', 'vcom'], before=['sccom'], ext_out=".h", vars=['SCGENMOD', '_SCGENMODFLAGS', 'SCGENMAP'])
scgenmod_task.quiet = True

# task to compile systemc files
sccom_task = Task.simple_task_type('sccom', '${SCCOM} ${_SCCOMFLAGS} ${SRC[0].abspath()}', color='BLUE', ext_in=".cpp", after=['vlib', 'vcom', 'vlog', 'scgenmod'], before=['sclink'], vars=['SCCOM', '_SCCOMFLAGS', 'CPPFLAGS', 'CXXFLAGS', 'INCLUDES'])
sccom_task.quiet = True
sccom_task.nocache = True
def sccom_task_str(self):
  ins = []
  for inf in self.inputs:
    ins.append(inf.name)
  return "sccom: %s -> %s\n" % (', '.join(ins), self.target)
sccom_task.__str__ = sccom_task_str

# task to link systemc files
sclink_task = Task.simple_task_type('sclink', '${SCCOM} ${_SCLINKFLAGS} -link', color='YELLOW', after=['vlib', 'vcom', 'vlog', 'sccom'], vars=['SCCOM', '_SCLINKFLAGS'])
sclink_task.nocache = True
def sclink_task_str(self):
  return "sc_link: %s\n" % self.target
sclink_task.__str__ = sclink_task_str
sclink_task.oldrun = sclink_task.run
def sclink_task_run(self):
  script = self.outputs[0]
  script.write(self.exec_script % {"date" : str(datetime.datetime.now()), "entry" : "work.sc_main", "ini" : self.inifile})
  os.chmod(script.abspath(), stat.S_IXUSR | stat.S_IXGRP | stat.S_IRUSR | stat.S_IRGRP | stat.S_IWUSR | stat.S_IWGRP)
  return self.oldrun() #super(sclink_task, self).run()
sclink_task.run = sclink_task_run

@TaskGen.before('process_source', 'process_rule')
@TaskGen.feature('modelsim')
def modelsim(self):
  """Apply Variables for the Target create vlib task and 
     vini task as well as dobefore and doafter tasks if needed
  """
  # cxx hack replaces the cpp routine
  setattr(self.__class__, create_compiled_task.__name__, create_compiled_task) 
  self.apply_incpaths()
  if not (Options.options.modelsim and self.env["MODELSIM"]):
    return
  self.env = self.env.derive()
  self.env["_VCOMFLAGS"]     = list(self.env["VCOMFLAGS"])
  self.env["_VLOGFLAGS"]     = list(self.env["VLOGFLAGS"])
  self.env["_SCCOMFLAGS"]    = list(self.env["SCCOMFLAGS"])
  self.env["_SCGENMODFLAGS"] = list(self.env["SCGENMODFLAGS"])
  self.env["_VSIMFLAGS"]     = list(self.env["VSIMFLAGS"])
  self.env["_SCLINKFLAGS"]   = list(self.env["SCLINKFLAGS"])
  self.env["_VSIMBEFORE"]    = list()
  self.env["_VSIMAFTER"]     = list()
  
  self.env["_VCOMFLAGS"] += ['-work', 'work']
  self.env["_VLOGFLAGS"] += ['-work', 'work']
  self.env["_SCCOMFLAGS"] += ['-work', 'work']
  self.env["_SCGENMODFLAGS"] += ['-lib', 'work']
  self.env["_VSIMFLAGS"] += ['-c', '-lib', 'work']
  self.env["_SCLINKFLAGS"] += ['-work', 'work']
    
  if hasattr(self, "vcomflags"):
    self.env["_VCOMFLAGS"] += Utils.to_list(self.vcomflags)

  if hasattr(self, "vsimflags"):
    self.env["_VSIMFLAGS"] += Utils.to_list(self.vsimflags)

  if hasattr(self, "vlogflags"):
    self.env["_VLOGFLAGS"] += Utils.to_list(self.vlogflags)

  if hasattr(self, "sccomflags"):
    self.env["_SCCOMFLAGS"] += Utils.to_list(self.sccomflags)

  if hasattr(self, "sclinkflags"):
    self.env["_SCLINKFLAGS"] += Utils.to_list(self.sclinkflags)

  if hasattr(self, "scgenmodflags"):
    self.env["_SCGENMODFLAGS"] += Utils.to_list(self.scgenmodflags)

  # Collect sccom flags from defines includes and uselib
  if hasattr(self, "defines"):
    for define in Utils.to_list(self.defines):
      self.env["_SCCOMFLAGS"] += ['-D%s' % define]
      self.env["_VLINKFLAGS"] += ['-D%s' % define]
  
  for d in self.env["DEFINES"]:
    if not d.startswith("PYTHONARCHDIR="):
      self.env.append_unique("_SCCOMFLAGS", "-D%s" % (d))
  
  if hasattr(self, "includes"):
    for inc in Utils.to_list(self.includes):
      i = self.path.find_dir(inc)
      if i:
        self.env["_SCCOMFLAGS"] += ['-I%s' % i.abspath()]
  
  if hasattr(self, "uselib"):
    for lib in Utils.to_list(self.uselib):
      for path in self.env["INCLUDES_%s" % lib]:
        self.env["_SCCOMFLAGS"] += ['-I%s' % path]
      for path in self.env["LIBPATH_%s" % lib]:
        self.env["_VLINKFLAGS"] += ['-L%s' % path]
      for lib in self.env["LIB_%s" % lib]:
        self.env["_VLINKFLAGS"] += ['-l%s' % lib]
  
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
  workdir = self.path.find_or_declare(targetdir(self.target))
  vlib = self.create_task('vlib', tgt=[workdir])
  vlib.target = self.target
  vlib.env = self.env

  #self.mdeps = []
  #if hasattr(self, "after"):
  #  lst = [n for n in self.bld.all_task_gen if "modelsim" in n.features and n.target == self.after]
  #  for l in lst:
  #    for t in l.tasks:
  #      if len(t.outputs) > 0:
  #        self.mdeps += t.outputs

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
    self.env["_SCCOMFLAGS"] += ['-I%s' % tgt.parent.abspath()]
    #mod.dep_nodes += self.mdeps

  # Create modelsim ini task if needed
  tgt = None
  if hasattr(self, "config"):
    tgt = self.path.find_or_declare("modelsim_%s.ini" % self.target)
    tsk = self.create_task('vini', tgt=tgt)
    tsk.env = self.env
    tsk.root = self.bld.out_dir
    tsk.path = self.path.get_bld().abspath()
    tsk.target = self.target
    tsk.config = self.path.find_resource(self.config) or self.config
    #tsk.set_run_after(vlib)
    ini = tgt.get_bld().abspath()
    self.inifile = ini
    self.env["_VSIMFLAGS"] += ['-modelsimini', ini]
    self.env["_VCOMFLAGS"] += ['-modelsimini', ini]
    self.env["_VLOGFLAGS"] += ['-modelsimini', ini]
    self.env["_SCCOMFLAGS"] += ['-modelsimini', ini]
    self.env["_SCGENMODFLAGS"] += ['-modelsimini', ini]
    self.env["_SCLINKFLAGS"] += ['-modelsimini', ini]
 
  # Create dobefore task if needed
  if hasattr(self, "dobefore"):
    do = self.path.find_resource(self.dobefore)
    if do:
      do = do.abspath()
    else:
      do = self.dobefore
    self.env["_VSIMBEFORE"] += ['-do', do]
    before = self.create_task('vsim_before')
    before.env = self.env
    #before.dep_nodes += self.mdeps
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
    self.env["_VSIMAFTER"] += ['-do', do]
    after = self.create_task('vsim_after')
    after.env = self.env
    #after.dep_nodes += self.mdeps
    if tgt:
      after.dep_nodes += [tgt]
    after.dotask = self.doafter
    after.target = self.target

@TaskGen.extension('.vhd')
def modelsim_vcom(self, node):
  """Create a vcom_task on each vhd file and ensure the order"""
  if 'modelsim' in self.features and Options.options.modelsim and self.env["MODELSIM"]:
    tgt = self.path.find_or_declare(os.path.join(targetdir(self.target),node.name + ".hdo"))
    tsks = [n for n in self.tasks if n.name in ['vcom', 'vlog']]
    tsk = self.create_task('vcom', [node], [tgt])
    tsk.env = self.env
    tsk.target = self.target
    if tsks:
      tsk.set_inputs(tsks[-1].outputs[0])
  
    #tsk.dep_nodes += self.mdeps
    
@TaskGen.extension('.v')
def modelsim_vlog(self, node):
  """Create a vlog_task on each v file and ensure the order"""
  if 'modelsim' in self.features and Options.options.modelsim and self.env["MODELSIM"]:
    tsks = [n for n in self.tasks if n.name in ['vcom', 'vlog']]
    tsk = self.create_task('vlog', [node])
    tsk.env = self.env
    if tsks:
      tsk.run_after.add(tsks[-1])

    #tsk.dep_nodes += self.mdeps

VSIM_FAKE_EXEC="""
#!/bin/bash
# This file is autogenerated at: %(date)s
# All modifications to this files will be lost after recompilation!

if [ -z "${VSIM}" ]; then
  VSIM=vsim
fi

params=""
if [ "${1}" != "--gui" ]; then
  params=-c ${params}
  if [ "${2}" = "--wave" ]; then
    params=${params} --do 'add wave -r sim:/sc_main/*'
  fi
else
  shift
fi

params=${params} $@

${VSIM} -modelsimini %(ini)s ${params} %(entry)s -do 'run -all;exit' | tee ${0}.log
test "$(tail -n 1 ${0}.log)" = "# Result: 0"
"""

#@TaskGen.extension('.cpp')
def modelsim_sccom(self, node):
  """Create a sccom_task on each cpp file and create a system c link task as well"""
  if 'modelsim' in self.features and Options.options.modelsim and self.env["MODELSIM"]:
    tsks = [n for n in self.tasks if getattr(n, 'name', None) == 'sccom']
    cwd = self.path.find_or_declare(targetdir(self.target))
    tgt = self.path.find_or_declare('%s/_sc/%s/%s' % (targetdir(self.target), self.env['VSIM_SC_DIR'], os.path.splitext(node.name)[0]+'.o'))
    tsk = self.create_task('sccom', [node], [tgt])
    tsk.env = self.env
    tsk.target = self.target
    tsk.cwd = cwd.abspath()
    if tsks:
      #tsk.run_after.add(tsks[-1])
      self.link_task.inputs.append(tgt)
    else:
      lib = self.path.find_or_declare('%s/_sc/%s/%s' % (targetdir(self.target), self.env['VSIM_SC_DIR'], 'systemc.so'))
      self.link_task = self.create_task('sclink', [tgt], [self.path.find_or_declare(self.target), lib])
      self.link_task.target = self.target
      self.link_task.exec_script = VSIM_FAKE_EXEC
      self.link_task.inifile = self.inifile
      if 'test' in self.features:
        self.default_install_path=None
        test = self.create_task('utest', self.link_task.outputs[0])
        test.ut_exec = ['sh', self.link_task.outputs[0].abspath()]
    #tsk.dep_nodes += self.mdeps

@TaskGen.before('process_source', 'process_rule')
@TaskGen.feature('sparc')
def sparc(self):
  self.env = self.bld.env_of_name('sparc')

def create_compiled_task(self, name, node):
  if 'modelsim' in self.features:
    modelsim_sccom(self, node)
  else:
    from waflib.Tools.ccroot import create_compiled_task
    create_compiled_task(self, name, node)
