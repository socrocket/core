#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os

def options(self):
    self.add_option(
        '--with-amba', 
        type='string', 
        help='Basedir of your AmbaKit installation', 
        dest='ambadir', 
        default=os.environ.get("AMBA_HOME")
    )

def find(self, path = None):
    incpath = [os.path.abspath(os.path.expanduser(os.path.expandvars(path))),
               os.path.join(os.path.abspath(os.path.expanduser(os.path.expandvars(path))), "dependencies", "AMBA-PV", "include")]

    self.check_cxx(
      header_name   = "amba.h",
      uselib_store  = 'AMBA',
      mandatory     = True,
      includes      = incpath,
      uselib        = 'BOOST SYSTEMC TLM GREENSOCS',
      okmsg         = "ok",
      errmsg        = 'AMBAKit not found please give the location with --amba=',
      fragment      = '''
           #include <systemc.h>
           #include <tlm.h>
           #include <amba.h>

           extern "C" {
               int sc_main(int argc, char** argv) {
                   return 0;
               };
           }
      '''
    )

    # Check for AMBAKit Headers
    self.check_cxx(
      msg          = "Checking for AMBAKit Version > 1.0.5",
      mandatory    = True,
      execute      = True,
      rpath        = self.env.LIBPATH_SYSTEMC,
      fragment     = """
                     #include <amba.h>
                     int sc_main(int argc, char *argv[]) {
                       return !((AMBA_TLM_VERSION_MAJOR >= 1) && (AMBA_TLM_VERSION_MINOR >= 0) && (AMBA_TLM_VERSION_REVISION >= 6));
                     }
                     """,
      uselib       = 'BOOST SYSTEMC TLM GREENSOCS AMBA',
      okmsg        = "ok",
    )
    
def configure(self):
    try:
        if self.options.ambadir:
            find(self, self.options.ambadir)
        else:
            find(self)
    except:
        name    = "ambakit"
        version = "trunk"
        self.dep_fetch(
            name    = name, 
            version = version,
            git_url = "git@brauhaus.c3e.cs.tu-bs.de:socrocket/ambakit.git",
            tar     = "ambakit-1.1.0.tar.gz"
        )
        find(self, self.dep_path(name, version))
