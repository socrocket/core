#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
from waflib import Utils
from waflib.Errors import ConfigurationError

def options(self):
    self.add_option(
        '--with-greenlib', 
        type='string', 
        help='Basedir of your greenlib installation', 
        dest='greenlibdir', 
        default=os.environ.get("GREENLIB_HOME")
    )

def find(self, path = None):
    if path:
        incpath = os.path.join(path, "include")
        libpath = os.path.join(path, "lib")

    else:
        incpath = []
        libpath = []

    self.check_cxx(
      header_name   = "greensocket/initiator/single_socket.h",
      uselib_store  = 'GREENSOCS',
      mandatory     = True,
      includes      = incpath,
      uselib        = 'BOOST SYSTEMC TLM',
      okmsg         = "ok",
      fragment      = '''
           #include <systemc.h>
           #include <tlm.h>
           #include <greensocket/initiator/single_socket.h>

           extern "C" {
               int sc_main(int argc, char** argv) {
                   return 0;
               };
           }
      '''
    )
    self.check_cxx(
      header_name   = "greensocket/target/single_socket.h",
      uselib_store  = 'GREENSOCS',
      mandatory     = True,
      includes      = incpath,
      uselib        = 'GREENSOCS BOOST SYSTEMC TLM',
      okmsg         = "ok",
      fragment      = '''
           #include <systemc.h>
           #include <tlm.h>
           #include <greensocket/target/single_socket.h>

           extern "C" {
               int sc_main(int argc, char** argv) {
                   return 0;
               };
           }
      '''
    )

    # Check for GreenSocs GreenControl
    self.check_cxx(
      header_name   = "greencontrol/config.h",
      uselib_store  = 'GREENSOCS',
      mandatory     = True,
      includes      = incpath,
      uselib        = 'GREENSOCS BOOST SYSTEMC TLM',
      okmsg         = "ok",
      fragment      = '''
           #include <systemc.h>
           #include <tlm.h>
           #include <greencontrol/config.h>

           extern "C" {
               int sc_main(int argc, char** argv) {
                   return 0;
               };
           }
      '''
    )

    if path:
        self.env["HOME_GREENLIB"] = path
    
def configure(self):
    self.find_program('swig', var='SWIG', mandatory=True, okmsg="ok", errmsg="Please install SWIG to continue")
    try:
        if self.options.greenlibdir:
            find(self, self.options.greenlibdir)
        else:
            find(self)
    except ConfigurationError as e:
        name    = "greenlib"
        version = "trunk"
        self.dep_build(
            name         = name, 
            version      = version,
            git_url      = "http://git.greensocs.com/greenlib/greenlib.git",
            git_checkout = "ecfee38aebe09f91d1affd82ca03581a2bba3662",
            patch        = [os.path.join(self.path.abspath(), "core", "waf", "greenlib-2013-12-02.patch"),
                            os.path.join(self.path.abspath(), "core", "waf", "greenlib-2014-10-17.rmeyer.patch")],
            config_cmd   = Utils.subst_vars("${CMAKE} %(src)s -DSYSTEMC_PREFIX=${HOME_SYSTEMC} -DTLM_HOME=${HOME_TLM} -DCMAKE_INSTALL_PREFIX=%(prefix)s", self.env),
            build_cmd    = Utils.subst_vars("${MAKE} ${JOBS} || ${MAKE} ${JOBS}", self.env)
        )
        find(self, self.dep_path(name, version))

