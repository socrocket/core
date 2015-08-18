#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
import subprocess
from waflib.Errors import ConfigurationError

if "check_output" not in dir( subprocess ): # duck punch it in!
    def f(*popenargs, **kwargs):
        if 'stdout' in kwargs:
            raise ValueError('stdout argument not allowed, it will be overridden.')
        process = subprocess.Popen(stdout=subprocess.PIPE, *popenargs, **kwargs)
        output, unused_err = process.communicate()
        retcode = process.poll()
        if retcode:
            cmd = kwargs.get("args")
            if cmd is None:
                cmd = popenargs[0]
            raise subprocess.CalledProcessError(retcode, cmd)
        return output
    subprocess.check_output = f

def options(self):
    self.add_option(
        '--with-cmake', 
        type='string', 
        help='Basedir of your cmake installation', 
        dest='cmakedir', 
        default=os.environ.get("CMAKE_HOME")
    )

def find(self, path = None):
    if path:
        path = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "bin"))))

    self.find_program('cmake', var='CMAKE', mandatory=True, okmsg="ok")

    if "CMAKE" in self.env:
        self.start_msg("Checking cmake version")
        cmake_version_str = subprocess.check_output(["cmake", "--version"])
        cmake_version_str = cmake_version_str.decode('utf-8').split('\n')[0]
        cmake_version = [int(v) for v in cmake_version_str.split(" ")[2].split(".")]
        cmake_version = cmake_version[0] * 1000000 + cmake_version[1] * 10000 + cmake_version[2] * 100

    if not ("CMAKE" in self.env) or cmake_version < 2081101:
        self.end_msg(cmake_version_str[:-2] + " is not enough using internal version")
        self.fatal("You need at least CMAKE version 2.8.12")
    else:
        self.end_msg(cmake_version_str[:-2] + " is ok")
    
def configure(self):
    try:
        if self.options.cmakedir:
            find(self, self.options.cmakedir)
        else:
            find(self)
    except ConfigurationError as e:
        name    = "cmake"
        version = "2.8.12"
        self.dep_build(
            name    = name, 
            version = version,
            tar_url = "http://www.cmake.org/files/v2.8/%(base)s.tar.gz",
            config_cmd = "%(src)s/bootstrap --prefix=%(prefix)s",
        )
        self.find_program('cmake', var='CMAKE', mandatory=True, okmsg="ok",
            path_list=[os.path.join(self.dep_path(name, version), 'bin')])

