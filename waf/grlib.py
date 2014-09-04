#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os

def options(self):
    self.add_option(
        "--grlib", 
        type='string', 
        dest="grlibdir", 
        help="Basedir of your grlib distribution", 
        default=os.environ.get("GRLIB_HOME")
    )
    self.add_option(
        "--grlib_tech", 
        type='string', 
        dest="grlibtech", 
        help="Basedir of your modelsim grlib work libraries", 
        default=os.environ.get("GRLIB_TECH")
    )

def configure(self):
    """We need toc export the GRLIB Paths to vsim subshells for co-simulation"""
    if self.options.grlibdir:
      self.env['GRLIB_HOME'] = self.options.grlibdir
    if self.options.grlibtech:
      self.env['GRLIB_TECH'] = self.options.grlibtech


