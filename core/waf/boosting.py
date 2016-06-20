#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os

def options(self):
    """Present boost options here"""
    self.load("boost", "core/waf")

def configure(self):
    """Check for boost libraries"""
    self.load('boost')
    # Try to load options from env if not given
    if not self.options.boost_includes or self.options.boost_includes == "":
      self.options.boost_includes = os.environ.get("BOOST_DIR",None)
    if not self.options.boost_libs or self.options.boost_libs == "":
      self.options.boost_libs = os.environ.get("BOOST_LIB",None)

    boostLibs = 'thread regex date_time program_options filesystem unit_test_framework system serialization'
    boostErrorMessage = 'Unable to find ' + boostLibs + ' boost libraries of at least version 1.35, please install them and/or specify their location with the --boost-includes and --boost-libs configuration options. It can also happen that you have more than one boost version installed in a system-wide location: in this case remove the unnecessary versions.'
    
    boostLibs = 'thread regex date_time program_options filesystem system serialization'

    self.check_boost(lib=boostLibs, static=True, mandatory=True, errmsg = boostErrorMessage)
    if int(self.env.BOOST_VERSION.split('_')[1]) < 35:
        self.fatal(boostErrorMessage)
    #if not self.options.static_build:
    #    self.env.append_unique('RPATH', self.env['LIBPATH_BOOST_THREAD'])


