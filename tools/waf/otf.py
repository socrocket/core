#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os

def options(self):
    self.add_option(
        "--with-otf", 
        type='string', 
        dest="otfdir", 
        help="Basedir of your otf installation", 
        default=os.environ.get("OTF_HOME")
    )

def find(self, path = None):
    libpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "lib"))))
    incpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "include", "open-trace-format"))))

    self.check_cxx(
      stlib        = 'open-trace-format',
      uselib_store = 'OTF',
      mandatory    = True,
      libpath      = libpath,
      errmsg       = "OTF Library not found. Use --otf option or set $OTF.",
      okmsg        = "ok"
    ) 
    self.check_cxx(
      stlib        = 'otfaux',
      uselib_store = 'OTF',
      mandatory    = True,
      libpath      = libpath,
      errmsg       = "OTF Library not found. Use --otf option or set $OTF.",
      okmsg        = "ok"
    ) 
    self.check_cxx(
      header_name  = 'otf.h',
      uselib_store = 'OTF',
      mandatory    = True,
      includes     = incpath,
      uselib       = 'OTF',
      okmsg        = "ok"
    ) 

def configure(self):
    try:
        if self.options.cmakedir:
            find(self, self.options.otfdir)
        else:
            find(self)
    except:
        name    = "otf"
        version = "trunk"
        self.dep_build(
            name    = name, 
            version = version,
            git_url = "http://www.cmake.org/files/v2.8/%(base)s.tar.gz",
        )
        find(self, self.dep_path(name, version))
