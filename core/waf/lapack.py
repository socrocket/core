#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os

def options(self):
    self.load('compiler_fc')
    self.add_option(
        '--with-lapack', 
        type='string', 
        help='LAPACK installation directory', 
        dest='lapackdir', 
        default=os.environ.get("LAPACK_HOME")
    )

def find(self, path = None):
    """Check for LAPACK Library and Headers"""
    if path:
      libpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "lib"))))
      incpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "include"))))
    else:
      libpath = "/usr/lib"
      incpath = "/usr/include"

    self.check_cxx(
      lib          = 'lapack',
      uselib_store = 'LAPACK',
      mandatory    = True,
      libpath      = libpath,
      errmsg       = "LAPACK Library not found. Use --with-lapack option or set $LAPACK_HOME.",
      okmsg        = "ok"
    ) 
    #self.check_cxx(
    #  header_name  = 'lapack.h',
    #  uselib_store = 'LAPACK',
    #  mandatory    = True,
    #  includes     = incpath,
    #  uselib       = 'LAPACK',
    #  okmsg        = "ok"
    #) 
    
    if path:
        self.env["HOME_LAPACK"] = path

def configure(self):
    self.load('compiler_fc')
    try:
        if self.options.elfdir:
            find(self, self.options.elfdir)
        else:
            find(self)
    except:
        self.fatal("LAPACK cannot installed automaticaly. Please install it within your system.")



