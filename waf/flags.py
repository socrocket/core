#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
from __future__ import print_function
import os
import sys
from waflib.Build import BuildContext

DEBUG_DEFAULT = True

def options(self):
    if DEBUG_DEFAULT:
        self.add_option(
            '-D', '--nodebug', 
            default=True, 
            action='store_false', 
            help='Disables debugging support for the targets', 
            dest='debug'
        )
    else:
        self.add_option(
            '-D', 
            '--debug', 
            default=True, 
            action='store_true', 
            help='Enables debugging support for the targets', 
            dest='debug'
        )
    self.add_option(
        '-G', '--gcov', 
        default=False, 
        action='store_true', 
        help='Enables profiling with gcov profiler', 
        dest='enable_gcov'
    )
    self.add_option(
        '-P', '--gprof', 
        default=False, 
        action='store_true', 
        help='Enables profiling with gprof profiler', 
        dest='enable_gprof'
    )
    self.add_option(
        '-V', '--vprof', 
        default=False, 
        action='store_true', 
        help='Enables profiling with vprof profiler', 
        dest='enable_vprof'
    )
    self.add_option(
        '--with-vprof', 
        type='string', 
        help='vprof installation folder', 
        dest='vprofdir'
    )
    self.add_option(
        '--with-papi', 
        type='string', 
        help='papi installation folder', 
        dest='papidir'
    )

def configure(self):
    # Small hack to adjust common usage of CPPFLAGS
    for flag in self.env['CPPFLAGS']:
        if flag.startswith('-D'):
            self.env.append_unique('DEFINES', flag[2:])

    # Check for standard tools
    if self.env.CC_VERSION:
        if int(self.env.CC_VERSION[0]) > 3:
            self.msg('Checking for compiler version', 'ok - ' + '.'.join(self.env.CC_VERSION))
        else:
            self.fatal('Compiler Version' + '.'.join(self.env.CC_VERSION) + ' too old: at least version 4.x required')

    # Determining gcc search dirs
    compilerExecutable = ''
    if len(self.env['CXX']):
        compilerExecutable = self.env['CXX'][0]
    elif len(self.env['CC']):
        compilerExecutable = self.env['CC'][0]
    else:
        self.fatal('CC or CXX environment variables not defined: Error, is the compiler correctly detected?')

    result = os.popen(compilerExecutable + ' -print-search-dirs')
    searchDirs = []
    localLibPath = os.path.join('/', 'usr', 'lib64')
    if os.path.exists(localLibPath):
        searchDirs.append(localLibPath)
    localLibPath = os.path.join('/', 'usr', 'local', 'lib')
    if os.path.exists(localLibPath):
        searchDirs.append(localLibPath)
    localLibPath = os.path.join('/', 'sw', 'lib')
    if os.path.exists(localLibPath):
        searchDirs.append(localLibPath)
    gccLines = result.readlines()
    for curLine in gccLines:
        startFound = curLine.find('libraries: =')
        if startFound != -1:
            curLine = curLine[startFound + 12:-1]
            searchDirs_ = curLine.split(':')
            for i in searchDirs_:
                if os.path.exists(i) and not os.path.abspath(i) in searchDirs:
                    searchDirs.append(os.path.abspath(i))
            break
    self.msg('Determining gcc search path', 'ok')


    # Since I want to build fast simulators, if the user didn't
    # specify any flags I set optimized flags
    #if not self.env['CXXFLAGS'] and not self.env['CCFLAGS']:
    #self.env.append_unique('LINKFLAGS', '-lmudflap')
    #self.env.append_unique('CXXFLAGS', '-fmudflap')
    #self.env.append_unique('CCFLAGS', '-fmudflap')
    testFlags = ['-O1', '-march=native', '-pipe', '-finline-functions', '-ftracer', '-fomit-frame-pointer', '-fpermissivE', '-fPIC']
    if self.check_cxx(cxxflags=testFlags, msg='Checking for g++ optimization flags', mandatory=False) and self.check_cc(cflags=testFlags, msg='Checking for gcc optimization flags'):
        self.env.append_unique('CXXFLAGS', testFlags)
        self.env.append_unique('CCFLAGS', testFlags)
        if not self.options.debug:
          self.env.append_unique('DEFINES', 'NDEBUG')
    else:
        testFlags = ['-O1', '-pipe', '-finline-functions', '-fomit-frame-pointer', '-fpermissive', '-fPIC']
        if self.check_cxx(cxxflags=testFlags, msg='Checking for g++ optimization flags') and self.check_cc(cflags=testFlags, msg='Checking for gcc optimization flags'):
            self.env.append_unique('CXXFLAGS', testFlags)
            self.env.append_unique('CCFLAGS', testFlags)
            if not self.options.debug:
                self.env.append_unique('DEFINES', 'NDEBUG')

    if self.env['CFLAGS']:
        self.check_cc(cflags=self.env['CFLAGS'], mandatory=True, msg='Checking for C compilation flags')
    if self.env['CCFLAGS'] and self.env['CCFLAGS'] != self.env['CFLAGS']:
        self.check_cc(cflags=self.env['CCFLAGS'], mandatory=True, msg='Checking for C compilation flags')
    if self.env['CXXFLAGS']:
        self.check_cxx(cxxflags=self.env['CXXFLAGS'], mandatory=True, msg='Checking for C++ compilation flags')
    if self.env['LINKFLAGS']:
        self.check_cxx(linkflags=self.env['LINKFLAGS'], mandatory=True, msg='Checking for link flags')

    # Check support for debugging
    if self.options.debug:
        if not '-g' in self.env['CCFLAGS']:
            self.env.append_unique('CCFLAGS', '-g')
        if not '-Wall' in self.env['CCFLAGS']:
            self.env.append_unique('CCFLAGS', '-Wall')
        if not '-g' in self.env['CXXFLAGS']:
            self.env.append_unique('CXXFLAGS', '-g')
        if not '-Wall' in self.env['CXXFLAGS']:
            self.env.append_unique('CXXFLAGS', '-Wall')
        if not self.options.verbosity:
            self.env.append_unique('DEFINES', 'GLOBALVERBOSITY=4')

    # Check support for profilers
    if self.options.enable_gprof and self.options.enable_vprof:
        self.fatal('Only one profiler among gprof and vprof can be enabled at the same time')
    if self.options.enable_gcov:
        self.find_program('gcov', var='gcov', mandatory=False, errmsg='gcov not found! Post execution preperation is disabled.')
        self.find_program('lcov', var='lcov', mandatory=False, errmsg='lcov not found! Post execution summeration is disabled.')
        self.find_program('genhtml', var='genhtml', mandatory=False)
        self.env.append_unique('CCFLAGS', '-fprofile-arcs')
        self.env.append_unique('CCFLAGS', '-ftest-coverage')
        self.env.append_unique('CCFLAGS', '--coverage')
        self.env.append_unique('CXXFLAGS', '-fprofile-arcs')
        self.env.append_unique('CXXFLAGS', '-ftest-coverage')
        self.env.append_unique('CXXFLAGS', '--coverage')
        self.env.append_unique('LINKFLAGS', '-fprofile-arcs')
        if '-O2' in self.env['CCFLAGS']:
            self.env['CCFLAGS'].remove('-O2')
        if '-O3' in self.env['CCFLAGS']:
            self.env['CCFLAGS'].remove('-O3')
        if '-O2' in self.env['CXXFLAGS']:
            self.env['CXXFLAGS'].remove('-O2')
        if '-O3' in self.env['CXXFLAGS']:
            self.env['CXXFLAGS'].remove('-O3')
        self.options.enable_gprof = True
    if self.options.enable_gprof:
        if not '-g' in self.env['CCFLAGS']:
            self.env.append_unique('CCFLAGS', '-g')
        if not '-g' in self.env['CXXFLAGS']:
            self.env.append_unique('CXXFLAGS', '-g')
        if '-fomit-frame-pointer' in self.env['CCFLAGS']:
            self.env['CCFLAGS'].remove('-fomit-frame-pointer')
        if '-fomit-frame-pointer' in self.env['CXXFLAGS']:
            self.env['CXXFLAGS'].remove('-fomit-frame-pointer')
        self.env.append_unique('CCFLAGS', '-pg')
        self.env.append_unique('CXXFLAGS', '-pg')
        self.env.append_unique('LINKFLAGS', '-pg')
        self.env.append_unique('STLINKFLAGS', '-pg')
    if self.options.enable_vprof:
        if not '-g' in self.env['CCFLAGS']:
            self.env.append_unique('CCFLAGS', '-g')
        if not '-g' in self.env['CXXFLAGS']:
            self.env.append_unique('CXXFLAGS', '-g')
        # I have to check for the vprof and papi libraries and for the
        # vmonauto_gcc.o object file
        vmonautoPath = ''
        if not self.options.vprofdir:
            self.check_cxx(lib='vmon', uselib_store='VPROF', mandatory=True)
            for directory in searchDirs:
                if 'vmonauto_gcc.o' in os.listdir(directory):
                    vmonautoPath = os.path.abspath(os.path.expanduser(os.path.expandvars(directory)))
                    break;
        else:
            self.check_cxx(lib='vmon', uselib_store='VPROF', mandatory=True, libpath = os.path.abspath(os.path.expanduser(os.path.expandvars(self.options.vprofdir))))
            self.env.append_unique('RPATH', self.env['LIBPATH_VPROF'])
            self.env.append_unique('LIBPATH', self.env['LIBPATH_VPROF'])
            vmonautoPath = self.env['LIBPATH_VPROF'][0]
        self.env.append_unique('LIB', 'vmon')

        if not self.options.papidir:
            self.check_cxx(lib='papi', uselib_store='PAPI', mandatory=True)
        else:
            self.check_cxx(lib='papi', uselib_store='PAPI', mandatory=True, libpath = os.path.abspath(os.path.expanduser(os.path.expandvars(self.options.papidir))))
            self.env.append_unique('RPATH', self.env['LIBPATH_PAPI'])
            self.env.append_unique('LIBPATH', self.env['LIBPATH_PAPI'])
        self.env.append_unique('LIB', 'papi')

        # now I check for the vmonauto_gcc.o object file
        taskEnv = self.env.copy()
        taskEnv.append_unique('LINKFLAGS', os.path.join(vmonautoPath, 'vmonauto_gcc.o'))
        self.check_cxx(fragment='int main(){return 0;}', uselib='VPROF', mandatory=True, env=taskEnv)
        self.env.append_unique('LINKFLAGS', os.path.join(vmonautoPath, 'vmonauto_gcc.o'))

def coverage(self):
    """If configured with -G and lcov and gcof are installed it generates a code coverage report"""
    from subprocess import call, STDOUT
    if self.env["gcov"] and self.env["gcov"] != "" and self.env["lcov"] and self.env["lcov"] != "":
        print(call([self.env['lcov'], '-b', '.', '-t', 'SoCRocket', '-o', 'lcov_all.info', '-d', 'models', '-d', '../models', '-c'], shell=False, cwd=self.bldnode.apbspath(), stderr=STDOUT))
        print(call(["%s -r lcov_all.info 'amba*' 'ahbin*' 'ahbout*' 'ahbprof*' 'apbuart*' 'c++*' 'green*' 'boost*' 'TLM*' 'sysc*' 'test*' 'extern*' 'tools*' 'utils*' 'usr*' > lcov.info" % self.env['lcov']], shell=True, cwd=self.bldnode.abspath(), stderr=STDOUT))
        if self.env['genhtml'] and self.env['genhtml'] != "":
            print(call([self.env['genhtml'], '-s', '--demangle-cpp', '-o', 'coverage', 'lcov.info'], shell=False, cwd=self.bldnode.abspath(), stderr=STDOUT))
            print("Code coverage report generated: %s/coverage/index.html" % (self.bldnode.abspath()))
    else:
      print("To use the coverage pleas install gcov and lcov and configure the library with -G")
      sys.exit(0)


class Coverage(BuildContext):
    cmd = 'coverage'
    fun = 'coverage'
