#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
from waflib.Errors import ConfigurationError

def options(self):
    self.add_option(
        '--with-elf', 
        type='string', 
        help='libELF installation directory', 
        dest='elfdir', 
        default=os.environ.get("ELF")
    )

def find(self, path = None):
    if path:
        incpath = [os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, 'include')))),
                   os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, 'include', 'libelf'))))]
        libpath =  os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, 'lib'))))

        if not os.path.exists(os.path.join(libpath, self.env['cxxstlib_PATTERN'] % 'elf')) and  \
           not os.path.exists(os.path.join(libpath, self.env['cxxshlib_PATTERN'] % 'elf')):
            self.fatal('Unable to find libelf in specified path ' + libpath)

        elfHeaderFound = False
        for p in incpath:
            if os.path.exists(os.path.join(p, 'libelf.h')) and os.path.exists(os.path.join(p, 'gelf.h')):
                elfHeaderFound = True
                break

        if not elfHeaderFound:
            self.fatal('Unable to find libelf.h and/or gelf.h headers in specified path ' + str(incpath))

        if not self.check_cxx(
                stlib        = 'elf', 
                uselib_store = 'ELF_LIB', 
                #mandatory    = False, 
                libpath      = libpath
            ):
            self.check_cxx(
                lib='elf', 
                uselib_store='ELF_LIB', 
                mandatory=True, 
                libpath = libpath,
                errmsg  = "not found, using internal version. Use --with-libelf= option to provide a system wide version."
            )

        self.check(
            header_name='libelf.h', 
            uselib='ELF_LIB', 
            uselib_store='ELF_LIB', 
            features='cxx cprogram', 
            mandatory=True, 
            includes = incpath,
            errmsg  = "not found, using internal version. Use --with-libelf= option to provide a system wide version."
        )
        self.check(
            header_name='gelf.h', 
            uselib='ELF_LIB', 
            uselib_store='ELF_LIB', 
            features='cxx cprogram', 
            mandatory=True, 
            includes = incpath,
            errmsg  = "not found, using internal version. Use --with-libelf= option to provide a system wide version."
        )

    else:
        if self.check_cxx(
                stlib='elf', 
                uselib_store='ELF_LIB', 
                #mandatory = False
            ):
            self.check_cxx(
                lib='elf', 
                uselib_store='ELF_LIB', 
                mandatory=True,
                errmsg  = "not found, using internal version. Use --with-libelf= option to provide a system wide version."
            )
        self.check(
            header_name='libelf.h', 
            uselib='ELF_LIB', 
            uselib_store='ELF_LIB', 
            features='cxx cprogram', 
            mandatory=True,
            errmsg  = "not found, using internal version. Use --with-libelf= option to provide a system wide version."
        )
        self.check(
            header_name='gelf.h', 
            uselib='ELF_LIB', 
            uselib_store='ELF_LIB', 
            features='cxx cprogram', 
            mandatory=True,
            errmsg  = "not found, using internal version. Use --with-libelf= option to provide a system wide version."
        )

    if 'elf' in self.env['LIB_ELF_LIB']:
        self.check_cxx(
            fragment="""
                #include <libelf.h>
                
                int main(int argc, char *argv[]){
                    void * funPtr = (void *)elf_getphdrnum;
                    return 0;
                }
                """, 
            msg='Checking for function elf_getphdrnum', 
            use='ELF_LIB', 
            mandatory=True, 
            errmsg='Error, elf_getphdrnum not present in libelf; try to update to a newest version'
        )
    if path:
        self.env.HOME_LIBELF = path

def configure(self):
    self.check(
        header_name='cxxabi.h', 
        features='cxx cprogram', 
        mandatory=True
    )

    self.check_cxx(
        function_name='abi::__cxa_demangle', 
        header_name="cxxabi.h", 
        mandatory=True
    )

    try:
        if self.options.elfdir:
            find(self, self.options.elfdir)
        else:
            find(self)
    except ConfigurationError as e:
        name    = "libelf"
        version = "0.8.13"
        self.dep_build(
            name    = name, 
            version = version,
            tar_url = "http://www.mr511.de/software/%(base)s.tar.gz",
        )
        find(self, self.dep_path(name, version))

