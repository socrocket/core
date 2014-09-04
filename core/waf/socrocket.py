#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
import subprocess

def options(self):
    """No options to declare"""
    pass

def configure(self):
    """Check for SoCRocket Libs if lib dir is present"""
    libdir = os.path.join(self.path.abspath(), 'lib')
    incdir = os.path.join(self.path.abspath(), 'include')
    if os.path.isdir(libdir) and os.path.isdir(incdir):
        for lib in os.listdir(libdir):
            if lib.startswith("lib") and not lib in ['libcommon.a', 'libutils.a']:
                name = lib[3:-2]
                self.check_cxx(
                    lib          = name,
                    uselib_store = 'SOCROCKET',
                    mandatory    = True,
                    libpath      = libdir,
                ) 
                self.check_cxx(
                    header_name  = '%s.h' % (name),
                    uselib_store = 'SOCROCKET',
                    mandatory    = False,
                    includes     = incdir,
                    uselib       = 'SOCROCKET SYSTEMC TLM AMBA BOOST GREENSOCS',
                ) 
        self.check_cxx(
            lib          = 'utils',
            uselib_store = 'SOCROCKET',
            mandatory    = True,
            libpath      = libdir,
        ) 
        self.check_cxx(
            header_name  = 'ahbdevice.h',
            uselib_store = 'SOCROCKET',
            mandatory    = False,
            includes     = incdir,
            uselib       = 'SOCROCKET SYSTEMC TLM AMBA BOOST GREENSOCS',
        ) 
        self.check_cxx(
            lib          = 'common',
            uselib_store = 'SOCROCKET',
            mandatory    = True,
            libpath      = libdir,
        ) 
        self.check_cxx(
            header_name  = 'common.h',
            uselib_store = 'SOCROCKET',
            mandatory    = False,
            includes     = incdir,
            uselib       = 'SOCROCKET SYSTEMC TLM AMBA BOOST GREENSOCS',
        ) 

    # Check for RTL Adapters. If not present deactivate RTL (Co-)Simulations
    if not os.path.isdir("adapters"):
      self.env["MODELSIM"] = False
    

