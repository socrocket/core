#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os
import stat
from waflib import Context
from waflib.Build import BuildContext
import atexit

def options(self):
    pass

packages = [
    "Pillow==2.2.1",
    "docutils==0.11",
    "actdiag==0.5.0",
    "nwdiag==1.0.0",
    "seqdiag==0.9.0",
    "blockdiag==1.3.0",
    "Pygments==1.6",
    "Sphinx==1.1.3",

    "cov-core==1.7",
    "coverage==3.7.1",
    "nose==1.3.0",
    "nose-cov==1.6",

    "Jinja2==2.7.1",
    "MarkupSafe==0.18",
    #"basemap==1.0.7", -> World map views, external dependency, maybe?
    "funcparserlib==0.3.6",
    "numpy==1.7.1",
    "numpydoc==0.4",
    "openpyxl==1.7.0",
    "pandas==0.14.1",
    "pyparsing==2.0.1",
    "python-dateutil==2.1",
    "pytz==2013.8",
    "pyzmq==13.1.0",
    "scipy==0.12.1",
    "six==1.4.1",
    "tornado==3.1.1",
    "version==0.1.0",
    "webcolors==1.4",
    "xlrd==0.9.2",
    #"python-qt==0.50",
    "pyside",
    #"sip",
    "matplotlib",
    "ipython==1.1.0",
    "pyuv",
    "numexpr==2.4",
    "cython==0.20.2",
    "tables==3.1.1",
    "termcolor",
    "patsy",
    "statmodels",
]

ACTIVATE = """
#!/bin/bash
. ~/.bashrc
. %(venv)s/bin/activate
export LD_LIBRARY_PATH="%(libpath)s:$LD_LIBRARY_PATH"
export PYTHONPATH="%(pythonpath)s:$PYTHONPATH"
export PATH="%(path)s:$PATH"
alias deactivate=exit
"""

def configure(self):

    for package in packages:
        self.python_get(package)

def vexecute(self, cmd = ""):
    path = []
    libpath = []
    pythonpath = []
    for key in self.env.table.keys():
        if key.startswith("PATH_"):
            for val in self.env[key]:
                if not val in path:
                    path.append(val)
        if key.startswith("LIBPATH_") and not key == "LIBPATH_ST":
            for val in self.env[key]:
                if not val in libpath:
                    libpath.append(val)
        if key.startswith("PYTHONPATH_"):
            for val in self.env[key]:
                if not val in pythonpath:
                    pythonpath.append(val)
    path.append(os.path.join(self.srcnode.abspath(), "tools"))
    pythonpath.append(self.srcnode.abspath())

    activate = self.bldnode.find_or_declare("activate")
    activate.write(ACTIVATE % {"path" : ':'.join(path), "libpath" : ':'.join(libpath), "pythonpath" : ':'.join(pythonpath), "venv": self.env.VENV_PATH})
    activate.chmod(stat.S_IXUSR | stat.S_IXGRP | stat.S_IRUSR | stat.S_IRGRP | stat.S_IWUSR | stat.S_IWGRP)
    def eexit():
        os.execve(self.env.BASH, [self.env.BASH , '--rcfile', activate.abspath(), '-i'], os.environ)
    atexit.register(eexit)

def bash(self):
    vexecute(self)

class Bash(BuildContext):
    cmd = 'bash'
    fun = 'bash'

setattr(Context.g_module, 'bash', bash)

def console(self):
    vexecute(self, "ipython qtconsole --colors=linux")

class Console(BuildContext):
    cmd = 'console'
    fun = 'console'

setattr(Context.g_module, 'console', console)

def xconsole(self):
    vexecute(self, "ipython qtconsole --colors=linux")

class XConsole(BuildContext):
    cmd = 'xconsole'
    fun = 'xconsole'

setattr(Context.g_module, 'xconsole', console)
