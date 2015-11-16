#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
import sys
import glob
from waflib.Errors import ConfigurationError

DEPENDS = ['common', 'pthreads']

def options(self):
    self.add_option(
        '--with-systemc', 
        type='string', 
        help='SystemC installation directory', 
        dest='systemcdir', 
        default=os.environ.get("SYSTEMC_HOME")
    )
    self.add_option(
        '--with-tlm', 
        type='string', 
        help='TLM installation directory', 
        dest='tlmdir', 
        default=os.environ.get("TLM_HOME")
    )

def find(self, scpath = "/usr", tlmpath = None):
    """
    Is SystemC compiled? Check for SystemC library
    Notice that we can't rely on lib-linux, therefore I have to find the actual platform...
    """
    if scpath and not tlmpath:
        tlmpath = scpath


    # First I set the clafgs needed by TLM 2.0  and GreenSocs for including systemc dynamic process
    # creation and so on
    self.env.append_unique('DEFINES', '_REENTRANT')
    self.env.append_unique('DEFINES', 'USE_STATIC_CASTS')
    self.env.append_unique('DEFINES', 'SC_INCLUDE_DYNAMIC_PROCESSES')
    if scpath:
        scinc = [os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(scpath, 'include'))))]
        # 2.3.0 installs it into 'lib-linux64', 2.3.1 installs it into 'lib'
        sclib = glob.glob(os.path.join(os.path.abspath(os.path.expanduser(os.path.expandvars(scpath))), 'lib*'))
    else:
        scinc = []
        sclib = []

    self.check_cxx(
        stlib        = 'systemc', 
        uselib_store = 'SYSTEMC', 
        mandatory    = True, 
        libpath      = sclib, 
        errmsg       = 'not found, compiling internal version. To use a system wide version use --with-systemc option'
    )
    if len(sclib)>0:
        self.env.append_unique("LIBPATH_SYSTEMC", sclib[0])

    # Check if systemc is compiled with quick threads or not
    if not os.path.exists(os.path.join(scinc[0], 'sysc' , 'qt')):
        self.env.append_unique('DEFINES', 'SC_USE_PTHREADS')

    elif sys.platform == 'cygwin':
        self.fatal('SystemC under cygwin must be compiled with PThread support: recompile it using the "make pthreads" command')

    # Check for SystemC header and test the library
    if not sys.platform == 'cygwin':
        systemCerrmsg = 'Error, at least version 2.2.0 required'
    else:
        systemCerrmsg = 'Error, at least version 2.2.0 required.\nSystemC also needs patching under cygwin:\nplease controll that lines 175 and 177 of header systemc.h are commented;\nfor more details refer to http://www.ht-lab.com/howto/sccygwin/sccygwin.html\nhttp://www.dti.dk/_root/media/27325_SystemC_Getting_Started_artikel.pdf'

    self.check_cxx(
        fragment = '''
            #include <systemc.h>

            extern "C" {
                int sc_main(int argc, char** argv) {
                    return 0;
                };
            }
        ''', 
        header_name  = 'systemc.h', 
        use          = 'SYSTEMC', 
        uselib_store = 'SYSTEMC', 
        mandatory    = True, 
        includes     = scinc
    )

    self.check_cxx(
        fragment = '''
            #include <systemc.h>

            #ifndef SYSTEMC_VERSION
            #error SYSTEMC_VERSION not defined in file sc_ver.h
            #endif

            #if SYSTEMC_VERSION < 20070314
            #error Wrong SystemC version
            #endif

            extern "C" {
                int sc_main(int argc, char** argv) {
                    return 0;
                };
            }
        ''', 
        msg       = 'Checking for SystemC version', 
        use       = 'SYSTEMC', 
        mandatory = True, 
        errmsg    = systemCerrmsg
    )

    self.check_cxx(
        fragment = '''
            #include <systemc.h>

            #ifndef SYSTEMC_VERSION
            #error SYSTEMC_VERSION not defined in file sc_ver.h
            #endif

            #if SYSTEMC_VERSION < 20120700
            #error Wrong SystemC version
            #endif

            extern "C" {
                int sc_main(int argc, char** argv) {
                    return 0;
                };
            }
        ''', 
        msg         = 'Checking for SystemC Version 2.3.0+', 
        use         = 'SYSTEMC', 
        define_name = "USE_SYSTEMC_2_3"
    )

    # Check for TLM header
    if tlmpath:
        tlmpath = os.path.normpath(os.path.abspath(os.path.expanduser(os.path.expandvars(tlmpath))))
        if not tlmpath.endswith('include'):
            tlmpath = os.path.join(tlmpath, 'include')
    if tlmpath:
        tlmpath = [tlmpath]
    else:
        tlmpath = []

    self.check_cxx(
        fragment='''
            #include <systemc.h>
            #include <tlm.h>

            extern "C" {
                int sc_main(int argc, char** argv) {
                    return 0;
                };
            }
        ''', 
        header_name  = 'tlm.h', 
        use          = 'SYSTEMC', 
        uselib_store = 'TLM', 
        mandatory    = True, 
        includes     = tlmpath, 
        errmsg       = 'not found, use --tlm option'
    )

    self.check_cxx(
        fragment='''
            #include <systemc.h>
            #include <tlm.h>

            #ifndef TLM_VERSION_MAJOR
            #error TLM_VERSION_MAJOR undefined in the TLM library
            #endif
            #ifndef TLM_VERSION_MINOR
            #error TLM_VERSION_MINOR undefined in the TLM library
            #endif
            #ifndef TLM_VERSION_PATCH
            #error TLM_VERSION_PATCH undefined in the TLM library
            #endif

            #if TLM_VERSION_MAJOR < 2
            #error Wrong TLM version; required 2.0
            #endif

            extern "C" int sc_main(int argc, char **argv){
                return 0;
            }
        ''', 
        msg       = 'Check for TLM version', 
        use       = 'SYSTEMC TLM', 
        mandatory = True, 
        errmsg    = 'Error, at least version 2.0 required')
    if scpath:
        self.env["HOME_SYSTEMC"] = scpath
    if tlmpath:
        self.env["HOME_TLM"] = tlmpath[0]

def configure(self):
    try:
        if self.options.systemcdir or self.options.tlmdir:
            find(self, self.options.systemcdir, self.options.tlmdir)
        else:
            find(self)
    except ConfigurationError as e:
        name    = "systemc"
        version = "2.3.1"
        self.dep_build(
            name    = name, 
            version = version,
            tar_url     = "http://accellera.org/images/downloads/standards/systemc/systemc-2.3.1.tgz",
            install_cmd = "make install && ln -sf %(build)s %(prefix)s"
        )
        find(self, self.dep_path(name, version))



