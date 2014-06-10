#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
from waflib import TaskGen

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
        self.find_program('oclint', path_list=[os.path.join(path, 'bin'), path], madatory=False)
    else:
        # If the path was not specified look in the search PATH
        self.find_program('oclint', mandatory=False)

def configure(self):
    try:
        if self.options.oclintdir:
            find(self, self.options.oclintdir)
        else:
            find(self)
    except:
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

