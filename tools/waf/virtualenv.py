#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
import subprocess
from waflib.TaskGen import taskgen_method
from waflib import Context
from waflib import Task
from waflib import TaskGen
from waflib import Utils
from common import conf

def options(self):
  pass

"""
    # Makes existing virtual env relocatable (for installation?)
    self.cmd_and_log(
        [self.env.VIRTUALENV, "--relocatable", self.env.VENV_PATH],
        output=Context.BOTH,
        cwd=self.bldnode.abspath()
    )
"""
def python_get(self, name):
    self.start_msg("Install %s into virtualenv" % (name))
    self.cmd_and_log(
        [self.env.VPIP, "install", name], 
        output=Context.BOTH,
        cwd=self.env.VENV_PATH
    )
    self.end_msg("ok")
conf(python_get)

class venv_link_task(Task.Task):
  """Link a Python source directory into the site-packages dir of the venv"""
  name = 'venv_link'
  color = 'BLUE'
  before = []
  quiet = True
  def __str__(self):
      return "venv: %s -> virtualenv\n" % (', '.join([n.name for n in self.inputs]))
    
  def run(self):
      for snode in self.inputs:
        dnode = snode.get_bld()
        if not os.path.exists(dnode.abspath()):
            os.symlink(os.path.relpath(snode.abspath(), os.path.join(dnode.abspath(), "..")), dnode.abspath())

      snode = self.generator.path.get_bld().abspath()
      dnode = os.path.join(self.env["VENV_PATH"], "lib", ("python%s" % self.env.PYTHON_VERSION), "site-packages", os.path.basename(snode))
      if not os.path.exists(dnode):
          os.symlink(os.path.relpath(snode, os.path.join(dnode, "..")), dnode)
      return 0

@TaskGen.before('process_source', 'process_rule')
@TaskGen.feature('venv_package')
def venv_package(self):
  if hasattr(self, "pysource"):
      srclist = []
      for src in self.pysource:
        snode = self.path.find_node(src)
        srclist.append(snode)
      dst = self.bld.bldnode.find_node(".conf_check_venv")
      links = self.create_task('venv_link', src=srclist, tgt=dst)

def configure(self):
    try:
      self.find_program('virtualenv', var="VIRTUALENV", mandatory=True, okmsg="ok")
    except:
        name    = "virtualenv"
        version = "trunk"
        self.dep_fetch(
            name    = name, 
            version = version,
            git_url = "https://github.com/pypa/virtualenv.git",
        )
        self.find_program('virtualenv.py', var="VIRTUALENV", mandatory=True, okmsg="ok", path_list=[self.dep_path(name, version)])
    self.start_msg("Create python virtualenv")
    self.env["VENV_PATH"] = os.path.join(self.bldnode.abspath(), ".conf_check_venv")
    self.cmd_and_log(
        [self.env.VIRTUALENV, self.env.VENV_PATH],
        output=Context.BOTH,
        cwd=self.bldnode.abspath()
    )
    self.end_msg("ok")
    self.find_program('python', var="VPYTHON", mandatory=True, okmsg="ok", path_list=[os.path.join(self.env.VENV_PATH, "bin")])
    self.find_program('pip', var="VPIP", mandatory=True, okmsg="ok", path_list=[os.path.join(self.env.VENV_PATH, "bin")])

