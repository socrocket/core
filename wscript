#! /usr/bin/env python
# vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 filetype=python :
top = '..'
REPOSITORY_PATH = "core"
REPOSITORY_NAME = "SoCRocket Main Repository"
REPOSITORY_DESC = """SoCRocket Main Repository"""
REPOSITORY_TOOLS = [
    'pthreads',
    'flags',
    'boosting',
    'endian',
    'systools',
    'libelf',
    'systemc',
    'cmake',
    'winsocks',
    'greenlib',
    'ambakit',
    'socrocket',
    'wizard',
    'docs',
    'swig',
    'cpplint',
    'oclint',
    'clang_compilation_database',
    "sparcelf"
]

def build(self):
  self.recurse_all()
  self.recurse_all_tests()
