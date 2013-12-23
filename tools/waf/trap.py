#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
import glob
import subprocess

def options(self):
    self.add_option(
        '--with-trap', 
        type='string', 
        help='Basedir of your trap-gen installation', 
        dest='trapdir', 
        default=os.environ.get("TRAP_HOME")
    )
    self.add_option(
        '--static', 
        default=False, 
        action="store_true", 
        help='Triggers a static build, with no dependences from any dynamic library', 
        dest='static_build'
    )
    self.add_option(
        '-T', '--disable-tools', 
        default=True, 
        action="store_false", 
        help='Disables support for support tools (debuger, os-emulator, etc.) (switch)', 
        dest='enable_tools'
    )
    # Specify if instruction history has to be kept
    self.add_option(
        '-s', '--enable-history', 
        default=False, 
        action='store_true', 
        help='Enables the history of executed instructions', 
        dest='enable_history'
    )
    self.add_option(
        '--tsim-comp', 
        default=False, 
        action='store_true', 
        help='Defines the TSIM_COMPATIBILITY directive', 
        dest='define_tsim_compatibility'
    )

def check_trap_linking(self, libName, libPaths, symbol):
    for libpath in libPaths:
        libFile = os.path.join(libpath, self.env['cxxshlib_PATTERN'].split('%s')[0] + libName + self.env['cxxshlib_PATTERN'].split('%s')[1])
        if os.path.exists(libFile):
            libDump = os.popen(self.env.NM + ' -r ' + libFile).readlines()
            for line in libDump:
                if line.find(symbol) != -1:
                    return True
            break
        libFile = os.path.join(libpath, self.env['cxxstlib_PATTERN'].split('%s')[0] + libName + self.env['cxxstlib_PATTERN'].split('%s')[1])
        if os.path.exists(libFile):
            libDump = os.popen(self.env.NM + ' -r ' + libFile).readlines()
            for line in libDump:
                if line.find(symbol) != -1:
                    return True
            break
    return False


def find(self, path = None):
    # Finding trap-gen
    trapRevisionNum = 772
    trapLibErrmsg = 'not found, compiling internal version. Use --trapgen option to use system wide version. It might also be that the trap library is compiled '
    trapLibErrmsg += 'against libraries (bfd/libelf, boost, etc.) different from the ones being used now; in case '
    trapLibErrmsg += 'try recompiling trap library.'
    if path:
        libpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "lib"))))
        incpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "include"))))

        self.check_cxx(
            lib='trap', 
            use='ELF_LIB BOOST SYSTEMC', 
            uselib_store='TRAP', 
            mandatory=True, 
            libpath=libpath, 
            errmsg=trapLibErrmsg
        )
        foundShared = glob.glob(os.path.join(libpath, self.env['cxxshlib_PATTERN'] % 'trap'))
        if foundShared:
            self.env.append_unique('RPATH', self.env['LIBPATH_TRAP'])

        if not check_trap_linking(
                self, 'trap', 
                self.env['LIBPATH_TRAP'], 
                'elf_begin') \
            and 'bfd' not in self.env['LIB_ELF_LIB']:
            self.fatal('TRAP library not linked with libelf library: BFD library needed (you might need to re-create the processor specifying a GPL license) or compile TRAP using its LGPL flavour ')

        self.check_cxx(
            header_name='trap.hpp', 
            use='TRAP ELF_LIB BOOST SYSTEMC', 
            uselib_store='TRAP', 
            mandatory=True, 
            includes=incpath
        )
        self.check_cxx(
            fragment='''
              #include "trap.hpp"

              #ifndef TRAP_REVISION
              #error TRAP_REVISION not defined in file trap.hpp
              #endif

              #if TRAP_REVISION < ''' + str(trapRevisionNum) + '''
              #error Wrong version of the TRAP runtime: too old
              #endif

              int main(int argc, char * argv[]){return 0;}
            ''', 
            msg='Check for TRAP version', 
            use='TRAP ELF_LIB BOOST SYSTEMC', 
            mandatory=True, 
            includes=incpath, 
            errmsg='Error, at least revision ' + str(trapRevisionNum) + ' required'
        )
    else:
        self.check_cxx(
            stlib='trap', 
            use='ELF_LIB BOOST SYSTEMC', 
            uselib_store='TRAP', 
            mandatory=True, 
            errmsg=trapLibErrmsg
        )

        if not check_trap_linking(
                self, 'trap', 
                self.env['LIBPATH_TRAP'], 
                'elf_begin') \
            and 'bfd' not in self.env['LIB_ELF_LIB']:
            self.fatal('TRAP library not linked with libelf library: BFD library needed (you might need to re-create the processor specifying a GPL license) or compile TRAP using its LGPL flavour ')

        self.check_cxx(
            header_name='trap.hpp', 
            use='TRAP ELF_LIB BOOST SYSTEMC', 
            uselib_store='TRAP', 
            mandatory=True
        )
        self.check_cxx(
            fragment='''
                #include "trap.hpp"

                #ifndef TRAP_REVISION
                #error TRAP_REVISION not defined in file trap.hpp
                #endif

                #if TRAP_REVISION < ''' + str(trapRevisionNum) + '''
                #error Wrong version of the TRAP runtime: too old
                #endif

                int main(int argc, char * argv[]){return 0;}
            ''', 
            msg='Check for TRAP version', 
            use='TRAP ELF_LIB BOOST SYSTEMC', 
            mandatory=1, 
            errmsg='Error, at least revision ' + str(trapRevisionNum) + ' required'
        )
    
def configure(self):
    # Parsing command options
    if not self.options.enable_tools:
        self.env.append_unique('DEFINES', 'DISABLE_TOOLS')
    if self.options.static_build:
        self.env['FULLSTATIC'] = True
    if self.options.enable_history:
        self.env.append_unique('DEFINES', 'ENABLE_HISTORY')

    # Adding the custom preprocessor macros
    if self.options.define_tsim_compatibility:
        self.env.append_unique('DEFINES', 'TSIM_COMPATIBILITY')

    try:
        if self.options.trapdir:
            find(self, self.options.trapdir)
        else:
            find(self)
    except:
        name    = "trap"
        version = "trunk"
        config_cmd = "%(src)s/waf configure -o %(build)s --prefix=%(prefix)s"
        if self.env.HOME_LIB_ELF:
            config_cmd += " --with-elf="+self.env.HOME_LIB_ELF
        if self.env.HOME_SYSTEMC:
            config_cmd += " --with-systemc="+self.env.HOME_SYSTEMC
        self.dep_build(
            name        = name, 
            version     = version,
            git_url     = "git@brauhaus.c3e.cs.tu-bs.de:socrocket/trapgen.git",
            config_cmd  = config_cmd,
            config_cwd  = "%(src)s",
            build_cmd   = "%%(src)s/waf build %s" % self.env.JOBS,
            build_cwd  = "%(src)s",
            install_cmd = "%%(src)s/waf install %s" % self.env.JOBS,
            install_cwd  = "%(src)s",
        )
        find(self, self.dep_path(name, version))


