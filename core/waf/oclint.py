#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
from __future__ import print_function
import os
from waflib import TaskGen, Context, Build
from waflib.Errors import ConfigurationError

def options(self):
    self.add_option(
        '--with-oclint', 
        type='string', 
        help='Explicit path to the oclint executable. If not given it will searched on the path', 
        dest='oclintdir',
        default=os.environ.get("OCLINT_HOME")
    )

def find(self, path = None):
    """Search and configure OC Linter"""

    if path:
        path = os.path.abspath(os.path.expanduser(path))
    # Check if the compiler is present

    if path:
        self.find_program('oclint', var='OCLINT', path_list=[os.path.join(path, 'bin'), path])
    else:
        # If the path was not specified look in the search PATH
        self.find_program('oclint', var='OCLINT')

def configure(self):
    try:
        if self.options.oclintdir:
            find(self, self.options.oclintdir)
        else:
            find(self)
    except ConfigurationError as e:
        name    = "oclint"
        version = "0.7-x86_64-linux-ubuntu-12.04"
        self.dep_fetch(
            name    = name, 
            version = version,
            tar_url = "http://archives.oclint.org/releases/0.7/%(tar)s",
        )
        # This works only for linux for other platforms (windows/mac) we need to find a different solution
        # There is a build for windows as well in bcc/bin/win/%(tar)s but no for MacOSX but we can get the sources
        # Which only extend the GNU gcc sources and does not provide any compile instructions.
        find(self, self.dep_path(name, version))

def oclint(self):
    """Use oclint to check all files"""
    import json
    import argparse
    import re
    import subprocess
    import sys

    database_file = self.bldnode.make_node('compile_commands.json')

    def get_source_path(file_attr, dir_attr):
        if file_attr.startswith("/"):
            return file_attr
        elif dir_attr[-1] == "/":
            return dir_attr + file_attr
        else:
            return dir_attr + "/" + file_attr

    if os.path.isfile(self.env["OCLINT"]):
        if os.path.isfile(database_file.abspath()):
            compilation_database = json.load(open(database_file.abspath()))
            source_list = []
            for file_item in compilation_database:
                file_path = get_source_path(file_item["file"], file_item["directory"])
                if os.path.isfile(file_path):
                    source_list.append(file_path)
            source_paths = '"' + '" "'.join(source_list) + '"'
            oclint_arguments = ''
            oclint_invocation = self.env["OCLINT"] + oclint_arguments + ' ' + source_paths
            exit_code = subprocess.call(oclint_invocation, shell=True)
            sys.exit(exit_code)
        else:
            print("Error: compile_commands.json not found at current location.")
    else:
        print("Error: OCLint executable file not found.")

setattr(Context.g_module, 'oclint', oclint)
class OCLint(Build.BuildContext):
    cmd = 'oclint'
    fun = 'oclint'


