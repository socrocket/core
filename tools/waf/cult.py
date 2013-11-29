#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
import subprocess

def options(self):
    self.add_option(
        "--with-cult", 
        type='string', 
        dest="cultdir", 
        help="Basedir of your cult installation", 
        default=os.environ.get("CULT")
    )

def find(self, path = None):
    libpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "lib"))))
    incpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "include"))))

    self.check_cxx(
      stlib        = 'cult_sysc',
      uselib_store = 'CULT',
      mandatory    = True,
      libpath      = libpath,
      errmsg       = "CULT_TLM Library not found. Use --cult option or set $CULT.",
      #uselib       = 'OTF',
      okmsg        = "ok"
    )
    self.check_cxx(
      header_name  = 'cult.h',
      uselib_store = 'CULT',
      mandatory    = True,
      includes     = incpath,
      #uselib       = 'CULT OTF',
      uselib       = 'CULT',
      okmsg        = "ok"
    )
    
def configure(self):
    try:
        if self.options.cmakedir:
            find(self, self.options.cultdir)
        else:
            find(self)
    except:
        name    = "cult"
        version = "trunk"
        self.dep_build(
            name    = name, 
            version = version,
            git_url = "http://www.cmake.org/files/v2.8/%(base)s.tar.gz",
        )
        find(self, self.dep_path(name, version))




