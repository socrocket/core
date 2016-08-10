#! /usr/bin/env python
# vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 filetype=python :
top = '..'
REPOSITORY_PATH = "core"
REPOSITORY_NAME = "SoCRocket Main Repository"
REPOSITORY_VERSION = [3,0,0]
REPOSITORY_DESC = """SoCRocket Main Repository"""
REPOSITORY_DEPS = {
  "core/sr_registry": "https://github.com/socrocket/sr_registry.git",
  "core/sr_register": "https://github.com/socrocket/sr_register.git",
  "core/sr_report": "https://github.com/socrocket/sr_report.git",
  "core/sr_param": "https://github.com/socrocket/sr_param.git",
  "core/sr_signal": "https://github.com/socrocket/sr_signal.git",
  "pysc": "https://github.com/socrocket/pysc.git",
}
REPOSITORY_TOOLS = [
    'pthreads',
    'flags',
    'boosting',
    'endian',
    'systools',
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
    'sparcelf'
]

def build(self):
  self.recurse_all()
  self.recurse_all_tests()
