#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
from waflib import TaskGen
from waflib.Errors import ConfigurationError

def options(self):
    self.add_option(
        '--with-sparc-elf', 
        type='string', 
        help='Explicit path to the cross compiler. If not given it will searched on the path', 
        dest='sparcelfdir',
        default=os.environ.get("SPARCELF_HOME")
    )
    self.add_option(
        '--sparc-elf-prefix', 
        type='string', 
        help='Defines the sparc compiler prefix', 
        dest='sparc_prefix',
        default=os.environ.get("SPARCELF_PREFIX", "sparc-elf-")
    )

def find(self, path = ""):
    """Search and configure SPARC compiler"""

    if path:
        path = os.path.abspath(os.path.expanduser(path))
    self.setenv('sparc-elf')
    self.load('compiler_c')
    self.load('compiler_cxx')
    # Check if the compiler is present

    crosscc = crossxx = crossar = None
    
    try:
        crosscc = self.find_program(self.options.sparc_prefix + 'gcc', path_list=[os.path.join(path, 'bin'), path])
    except AttributeError as e:
        # If the path was not specified look in the search PATH
        crosscc = self.find_program(self.options.sparc_prefix + 'gcc')

    try:
        crossxx = self.find_program(self.options.sparc_prefix + 'g++', path_list=[os.path.join(path, 'bin'), path])
    except AttributeError as e:
        # If the path was not specified look in the search PATH
        crossxx = self.find_program(self.options.sparc_prefix + 'g++')

    #try:
    #    crossar = self.find_program(self.options.sparc_prefix + 'ar', path_list=[os.path.join(path, 'bin'), path])
    #except AttributeError as e:
    #    # If the path was not specified look in the search PATH
    #    crossar = self.find_program(self.options.sparc_prefix + 'ar')

    self.env['CC'] = self.env['LINK_CC'] = crosscc
    self.env['CXX'] = self.env['LINK_CXX'] = crossxx
    #self.env['AR'] = [crossar]

    sparcFlags = ['-Wall', '-static', '-O3']
    self.env['SHLIB_MARKER'] = ""

    self.env.append_unique('LINKFLAGS', sparcFlags)
    if  '-Wl,-Bdynamic' in self.env['LINKFLAGS']:
      self.env['LINKFLAGS'].remove('-Wl,-Bdynamic')

    self.env.append_unique('CFLAGS', sparcFlags)
    if  '-Wl,-Bdynamic' in self.env['CFLAGS']:
      self.env['CFLAGS'].remove('-Wl,-Bdynamic')

    self.env.append_unique('CCFLAGS', sparcFlags)
    if  '-Wl,-Bdynamic' in self.env['CCFLAGS']:
      self.env['CCFLAGS'].remove('-Wl,-Bdynamic')

    self.env.append_unique('CXXFLAGS', sparcFlags)
    if  '-Wl,-Bdynamic' in self.env['CXXFLAGS']:
      self.env['CXXFLAGS'].remove('-Wl,-Bdynamic')
    
    if self.env['CFLAGS']:
        self.check_cc(cflags=self.env['CFLAGS'], mandatory=True, msg='Checking for C compilation flags')
    if self.env['CCFLAGS'] and self.env['CCFLAGS'] != self.env['CFLAGS']:
        self.check_cc(cflags=self.env['CCFLAGS'], mandatory=True, msg='Checking for C compilation flags')
    #if self.env['CXXFLAGS']:
    #    self.check_cxx(cxxflags=self.env['CXXFLAGS'], mandatory=True, msg='Checking for C++ compilation flags')
    if self.env['LINKFLAGS']:
        self.check_cc(linkflags=self.env['LINKFLAGS'], mandatory=True, msg='Checking for link flags')
    self.setenv('')
    
    
def configure(self):
    try:
        if self.options.sparcelfdir:
            find(self, self.options.sparcelfdir)
        else:
            find(self)
    except ConfigurationError as e:
        self.setenv('')
        name    = "sparc-elf"
        version = "4.4.2"
        self.dep_fetch(
            name    = name, 
            version = version,
            tar     = "%(base)s.tar.bz2",
            tar_url = "http://www.gaisler.com/anonftp/bcc/bin/linux/%(base)s-1.0.44.tar.bz2",
        )
        # This works only for linux for other platforms (windows/mac) we need to find a different solution
        # There is a build for windows as well in bcc/bin/win/%(tar)s but no for MacOSX but we can get the sources
        # Which only extend the GNU gcc sources and does not provide any compile instructions.
        find(self, self.dep_path(name, version))
        self.setenv('')

@TaskGen.before('process_source', 'process_rule')
@TaskGen.feature('sparc')
def feature_sparcelf(self):
  env = self.bld.all_envs['sparc-elf'].derive()
  self.env = env
