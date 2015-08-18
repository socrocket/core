#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
from __future__ import print_function
import sys
import subprocess
from waflib.Build import BuildContext
from waflib import Context

def options(self):
    pass

def configure(self):
    """Check for doxygen"""
    self.find_program('doxygen', var='DOXYGEN', mandatory=False, okmsg="ok")

def docs(bld):
    """build source code documentation in 'build/docs' if doxygen is installed"""
    if bld.env.DOXYGEN and bld.env.DOXYGEN != "":
        subprocess.call(["doxygen", "core/waf/docs.conf"]) 
    else:
        print("To use the ./waf docs comand please install doxygen")
        sys.exit(0)

setattr(Context.g_module, 'docs', docs)
class Docs(BuildContext):
    cmd = 'docs'
    fun = 'docs'

