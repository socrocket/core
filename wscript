#! /usr/bin/env python
# vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 filetype=python :
APPNAME = 'SoCRocket'
VERSION = '$Revision$'
top = '.'
out = 'build'

import sys
from waflib.Tools import waf_unit_test  
from core.waf.logger import Logger

LOAD = [
    'compiler_c',
    'compiler_cxx',
    'python',
    'waf_unit_test',
]

TOOLS = [
    'common',
    'dependency',
    'repository'
]

def options(self): 
    self.load(LOAD)
    self.load(TOOLS, tooldir='core/waf')
    self.loadrepos()

def configure(self):
    self.load(LOAD)
    self.check_waf_version(mini='1.6.0')
    self.check_python_version((2,4,0))
    self.check_python_headers()
    self.load(TOOLS, tooldir='core/waf')
    self.loadrepos()
    self.env.CPPLINT_FILTERS = ','.join((
    #    '-whitespace/newline',      # c++11 lambda
    #    '-readability/braces',      # c++11 constructor
    #    '-whitespace/braces',       # c++11 constructor
    #    '-build/storage_class',     # c++11 for-range
    #    '-whitespace/blank_line',   # user pref
    #    '-whitespace/labels'        # user pref
    ))
    
def build(self):
    self.load(LOAD) # reload python2 and configuration
    logger = Logger("%s/build.log" % self.bldnode.abspath())
    sys.stdout = logger
    sys.stderr = logger
    
    self.recurse_all()
    self.recurse_all_tests()
    self.iterrepos()

    # Install headers
    self.install_files('${PREFIX}/include', self.path.ant_glob(['**/*.h', '**/*.tpp'], excl=['build','**/signalkit/**', '**/tests/**', '**/extern/**', '**/contrib/**', '**/platform/**', '**/software/**', '**/.svn/**', '**/.git/**']))
    self.install_files('${PREFIX}/', ['waf', 'wscript', 'platforms/wscript'], relative_trick=True)
    self.install_files('${PREFIX}/', self.path.ant_glob('tools/**', excl=['**/*.pyc', '**/.svn/**', '**/.git/**']), relative_trick=True)
    self.install_files('${PREFIX}/', self.path.ant_glob('templates/**', excl=['**/*~', '**/.svn/**', '**/.git/**']), relative_trick=True)
    self.install_files('${PREFIX}/include', self.path.ant_glob('contrib/grambasockets/*.h', excl=['**/*~', '**/.svn/**', '**/.git/**']))
    self.add_post_fun(waf_unit_test.summary)
