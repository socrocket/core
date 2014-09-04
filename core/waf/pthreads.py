#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import sys

def options(self):
    """No options to define"""
    pass
    
def configure(self):
    """Insert pthreads to every flags"""
    if not self.check_cxx(linkflags='-pthread') or \
       not self.check_cc(cxxflags='-pthread') or \
       sys.platform == 'cygwin':
        self.env.append_unique('LIB', 'pthread')
    else:
        self.env.append_unique('LINKFLAGS', '-pthread')
        self.env.append_unique('CXXFLAGS', '-pthread')
        self.env.append_unique('CFLAGS', '-pthread')
        self.env.append_unique('CCFLAGS', '-pthread')
