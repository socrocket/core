#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
"""
    Waf cli command wrapper
    Extends Waf with the ability to handle subcommands.
    $ ./waf repo init
"""
import os
from waflib.Configure import ConfigurationContext
from waflib import Logs,Context,Build

COMMANDS = {}

def init(self):
    """
        Initialize subcommand manager
        As soon as we detect that the first argument to waf is a subcomand.
        Remove all other arguments from the list.
        This ensures that ./waf <comand> can handle its own sub commands and arguments.
    """
    from waflib import Options
    global COMMANDS
    if Options.commands[0] in COMMANDS:
        del(Options.commands[1:])

if Context.g_module:
    setattr(Context.g_module, 'init', init) # Detect repo argument

def subcommand(context):
    """Register subcommand"""
    global COMMANDS
    COMMANDS[context.cmd] = context
    if Context.g_module:
        setattr(Context.g_module, context.cmd, context)
    return context

class SubcommandContext(ConfigurationContext):
    """
       The SubcommandContext
       Here we initialize a Context especialy for subcommands
    """
    cmd=None
    def __init__(self, **kw):
        super(SubcommandContext, self).__init__(**kw)
    def execute(self):
        self.init_dirs()
        self.cachedir=self.bldnode.make_node(Build.CACHE_DIR)
        self.cachedir.mkdir()
        path=os.path.join(self.bldnode.abspath(),self.cmd+".log")
        self.logger=Logs.make_logger(path,'cfg')
        app=getattr(Context.g_module,'APPNAME','')
        if app:
            ver=getattr(Context.g_module,'VERSION','')
            if ver:
              app="%s (%s)"%(app,ver)
        if id(self.srcnode)==id(self.bldnode):
          Logs.warn('Setting top == out (remember to use "update_outputs")')
        elif id(self.path)!=id(self.srcnode):
          if self.srcnode.is_child_of(self.path):
            Logs.warn('Are you certain that you do not want to set top="." ?')
        Context.top_dir=self.srcnode.abspath()
        Context.out_dir=self.bldnode.abspath()
        self.work()

    def work(self):
        pass
