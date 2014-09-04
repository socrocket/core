#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
from waflib.Build import BuildContext
from waflib import Context

def options(self):
    conf = self.add_option_group("'./waf generate' Options")
    conf.add_option(
        '-t', '--template', 
        default=None, 
        type='string', 
        help='Defines a template to generate a new platform', 
        dest='template'
    )
    conf.add_option(
        '-l', '--load', 
        default=None, 
        type='string', 
        help='Load given configuration of the template', 
        dest='configuration'
    )

def configure(self):
    pass

def generate(bld):
  """If PyQt4 is installed opens a Wizard to configure a platform"""
  from tools.generator.wizard import main
  main(bld.options.template, bld.options.configuration)

class Generate(BuildContext):
    cmd = 'generate'
    fun = 'generate'

setattr(Context.g_module, 'generate', generate)
