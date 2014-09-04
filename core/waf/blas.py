#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os

def options(self):
    self.add_option(
        '--with-blas', 
        type='string', 
        help='BLAS installation directory', 
        dest='blasdir', 
        default=os.environ.get("BLAS_HOME")
    )

def find(self, path = None):
    """Check for BLAS Library and Headers"""
    if path:
      libpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "lib"))))
      incpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "include"))))
    else:
      libpath = "/usr/lib"
      incpath = "/usr/include"

    self.check_cxx(
      lib          = 'blas',
      uselib_store = 'BLAS',
      mandatory    = True,
      libpath      = libpath,
      errmsg       = "BLAS Library not found. Use --with-blas option or set $BLAS_HOME.",
      okmsg        = "ok"
    ) 
    self.check_cxx(
      header_name  = 'cblas.h',
      uselib_store = 'BLAS',
      mandatory    = True,
      includes     = incpath,
      uselib       = 'BLAS',
      okmsg        = "ok"
    ) 
    
    if path:
        self.env["HOME_BLAS"] = path

def configure(self):
    try:
        if self.options.blasdir:
            find(self, self.options.blasdir)
        else:
            find(self)
    except:
        self.fatal("BLAS cannot installed automaticaly. Please install it within your system.")



