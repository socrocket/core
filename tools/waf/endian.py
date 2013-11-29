#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import sys

def options(self):
    """No Options to set"""
    pass

def configure(self):
    """Determ host endianess"""

    if sys.byteorder == "little":
        self.env.append_unique('DEFINES', 'LITTLE_ENDIAN_BO')
        self.msg('Checking for host endianness', 'little')
    else:
        self.env.append_unique('DEFINES', 'BIG_ENDIAN_BO')
        self.msg('Checking for host endianness', 'big')
