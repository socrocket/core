#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
from waflib.Errors import ConfigurationError

def options(self):
    self.add_option(
        '--with-amba', 
        type='string', 
        help='Basedir of your AmbaKit installation', 
        dest='ambadir', 
        default=os.environ.get("AMBA_HOME")
    )

def find(self, path = None):
    if path:
      incpath = [os.path.abspath(os.path.expanduser(os.path.expandvars(path))),
                 os.path.join(os.path.abspath(os.path.expanduser(os.path.expandvars(path))), "dependencies", "AMBA-PV", "include")]
    else:
      incpath = []

    self.check_cxx(
      header_name   = "amba.h",
      uselib_store  = 'AMBA',
      mandatory     = True,
      includes      = incpath,
      use           = 'BOOST SYSTEMC TLM GREENSOCS',
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
      uselib_store  = 'AMBA',
      mandatory    = True,
      execute      = True,
      rpath        = self.env.LIBPATH_SYSTEMC,
      fragment     = """
                     #include <amba.h>
                     int sc_main(int argc, char *argv[]) {
                       return !((AMBA_TLM_VERSION_MAJOR >= 1) && (AMBA_TLM_VERSION_MINOR >= 0) && (AMBA_TLM_VERSION_REVISION >= 6));
                     }
                     """,
      use          = 'BOOST SYSTEMC TLM GREENSOCS AMBA',
      okmsg        = "ok",
    )
    
def configure(self):
    try:
        if self.options.ambadir:
            find(self, self.options.ambadir)
        else:
            find(self)
    except ConfigurationError as e:
            name    = "amba_socket"
            version = "1.0.15"
        #try:
            self.dep_fetch(
                name    = name,
                version = version, 
                tar     = "amba_socket-1.0.15.tgz",
                tar_url = "amba_socket-1.0.15.tgz",
                base    = name,
                patch   = [os.path.join(self.path.abspath(), "core", "waf", "ambakit-2015-10-16-rmeyer.patch")]
            )
            find(self, self.dep_path(name, version).rstrip("-"+version))
        #except:
        #    self.fatal("failed\nYou have to register at Carbon Design Systems IP Exchange and download %s-%s.tgz via this URL https://portal.carbondesignsystems.com/Model/Carbon/TLM-2.0-AMBA. Please place the downloaded file in the socrocket src root directory")
