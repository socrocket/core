#! /usr/bin/env python
# vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 filetype=python :
top = '..'
REPOSITORY_PATH = "core"
REPOSITORY_NAME = "SoCRocket Main Repository"
REPOSITORY_VERSION = [3,0,0]
REPOSITORY_DESC = """SoCRocket Main Repository"""
REPOSITORY_DEPS = {
  "core/common/sr_registry": "https://github.com/socrocket/sr_registry.git",
  "core/common/sr_register": "https://github.com/socrocket/sr_register.git",
  "core/common/sr_report": "https://github.com/socrocket/sr_report.git",
  #"core/common/sr_config": "https://github.com/socrocket/sr_config.git",
  #"core/common/signalkit": "https://github.com/socrocket/signalkit.git",
}
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
