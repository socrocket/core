#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os

def options(self):
    self.add_option(
        '--with-greenlib', 
        type='string', 
        help='Basedir of your greenlib installation', 
        dest='greenlibdir', 
        default=os.environ.get("GREENLIB_HOME")
    )

def find(self, path = None):
    incpath = os.path.join(path, "include")
    libpath = os.path.join(path, "lib")

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

    # Check for GreenSocs GreenReg Library

    self.check_cxx(
      stlib        = 'greenreg',
      uselib_store = 'GREENSOCS',
      mandatory    = True,
      libpath      = libpath,
      okmsg        = "ok",
    ) 

    ##################################################
    # Check for GreenSocs GreenReg Header
    ##################################################
    self.check_cxx(
      header_name   = "greenreg/greenreg.h",
      uselib_store  = 'GREENSOCS',
      mandatory     = True,
      includes      = incpath,
      uselib        = 'GREENSOCS BOOST SYSTEMC TLM',
      okmsg        = "ok",
      fragment='''
           #include <systemc.h>
           #include <tlm.h>
           #include <greenreg/greenreg.h>

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
    except:
        name    = "greenlib"
        version = "trunk"
        self.dep_build(
            name         = name, 
            version      = version,
            git_url      = "git://git.greensocs.com/greenlib.git",
            git_checkout = "ecfee38aebe09f91d1affd82ca03581a2bba3662",
            #patch        = os.path.join(self.srcnode.abspath(), "contrib", "greenlib-2013-08-27.patch"),
            config_cmd   = "%(cmake)s %%(src)s -DSYSTEMC_PREFIX=%(systemc)s -DTLM_HOME=%(tlm)s -DCMAKE_INSTALL_PREFIX=%%(prefix)s" % {"cmake":self.env.CMAKE, "systemc":self.env.HOME_SYSTEMC, "tlm": "%s/include" % self.env.HOME_TLM}
        )
        find(self, self.dep_path(name, version))

