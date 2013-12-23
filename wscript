#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
APPNAME = 'SoCRocket'
VERSION = '$Revision$'
top = '.'
out = 'build'

import sys
from waflib.Tools import waf_unit_test  
from tools.waf.logger import Logger

LOAD = [
    'compiler_c',
    'compiler_cxx',
    'python',
    'swig',
    'waf_unit_test',
]

TOOLS = [
    'common',
    'flags',
    'virtualenv',
    'pthreads',
    'boost',
    'endian',
    'grlib',
    'modelsim',
    'systools',
    'libelf',
    'systemc',
    'cmake',
    'winsocks',
    'trap',
#    'cult',
#    'otf',
#    'sdl',
#    'mpeg3',
    'lua',
    'blas',
    'lapack',
    'greenlib',
    'ambakit',
    'socrocket',
    'wizard',
    'docs',
    'shell',
    'sparcelf',
]

def options(self): 
    self.load(LOAD)
    self.load(TOOLS, tooldir='tools/waf')

def configure(self):
    self.load(LOAD)
    self.check_waf_version(mini='1.6.0')
    self.check_python_version((2,4,0))
    self.check_python_headers()
    self.load(TOOLS, tooldir='tools/waf')
    
def build(self):
    logger = Logger("%s/build.log" % self.bldnode.abspath())
    sys.stdout = logger
    sys.stderr = logger
    
    self.recurse_all()

    self.install_files('${PREFIX}/include', self.path.ant_glob(['**/*.h', '**/*.tpp'], excl=['**/signalkit/**', '**/tests/**', '**/extern/**', '**/contrib/**', '**/platform/**', '**/software/**', '**/.svn/**', '**/.git/**']))
    self.install_files('${PREFIX}/', ['waf', 'wscript', 'platforms/wscript'], relative_trick=True)
    self.install_files('${PREFIX}/', self.path.ant_glob('tools/**', excl=['**/*.pyc', '**/.svn/**', '**/.git/**']), relative_trick=True)
    self.install_files('${PREFIX}/', self.path.ant_glob('templates/**', excl=['**/*~', '**/.svn/**', '**/.git/**']), relative_trick=True)
    self.install_files('${PREFIX}/include', self.path.ant_glob('contrib/grambasockets/*.h', excl=['**/*~', '**/.svn/**', '**/.git/**']))
    self.add_post_fun(waf_unit_test.summary)

