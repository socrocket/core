#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
import subprocess

def options(self):
    self.add_option(
        '--with-mpeg3', 
        type='string', 
        help='Basedir of your mpeg3 installation', 
        dest='mpeg3dir', 
        default=os.environ.get("MPEG3_HOME")
    )

def find(self, path = None):
    libpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "lib"))))
    incpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "include"))))

    self.check_cxx(
      lib          = 'mpeg3',
      uselib_store = 'MPEG3',
      mandatory    = False,
      libpath      = libpath,
      errmsg       = "mpeg3 Library not found. Use --mpeg3 option or set $MPEG3.",
      okmsg        = "ok"
    ) 
    self.check_cxx(
      header_name  = 'libmpeg3.h',
      uselib_store = 'MPEG3',
      mandatory    = False,
      includes     = incpath,
      uselib       = 'MPEG3',
      okmsg        = "ok"
    ) 
    
def configure(self):
    try:
        if self.options.mpeg3dir:
            find(self, self.options.mpeg3dir)
        else:
            find(self)
    except:
        name    = "libmpeg3"
        version = "1.8"
        self.dep_build(
            name    = name, 
            version = version,
            tar_url = "http://downloads.sourceforge.net/project/heroines/releases/081108/%(base)s.tar.gz"
        )
        find(self, self.dep_path(name, version))


