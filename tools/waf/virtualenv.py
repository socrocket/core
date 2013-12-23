#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
import subprocess
from waflib.TaskGen import taskgen_method
from waflib import Context
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

@taskgen_method
def venv_site_link(self, source):
    for src in source:
      snode = self.path.find_node(src)
      dnode = snode.get_bld()
      if not os.path.exists(dnode.abspath()):
          os.symlink(os.path.relpath(snode.abspath(), os.path.join(dnode.abspath(), "..")), dnode.abspath())

    snode = self.path.get_bld().abspath()
    dnode = os.path.join(self.bldnode.abspath(), ".venv", "lib", "python2.7", "site-packages", os.path.basename(snode))
    if not os.path.exists(dnode):
        os.symlink(os.path.relpath(snode, os.path.join(dnode, "..")), dnode)
conf(venv_site_link)

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
    self.env["VENV_PATH"] = os.path.join(self.bldnode.abspath(), ".venv")
    self.cmd_and_log(
        [self.env.VIRTUALENV, self.env.VENV_PATH],
        output=Context.BOTH,
        cwd=self.bldnode.abspath()
    )
    self.end_msg("ok")
    self.find_program('python', var="VPYTHON", mandatory=True, okmsg="ok", path_list=[os.path.join(self.env.VENV_PATH, "bin")])
    self.find_program('pip', var="VPIP", mandatory=True, okmsg="ok", path_list=[os.path.join(self.env.VENV_PATH, "bin")])

