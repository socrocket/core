#! /usr/bin/env python
# vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 filetype=python :
top = '../..'
REPOSITORY_PATH = "core/sr_registry"
REPOSITORY_NAME = "SoCRocket SystemC Model Registry"
REPOSITORY_VERSION = [3,0,0]
REPOSITORY_DESC = """Stores SystemC Models in Groups to Dynamiclay create and identify"""

def build(self):
    self(
        target            = 'sr_registry',
        features          = 'cxx cxxstlib pyembed venv_package',
        source            = [
            'sr_registry.cpp',
            'sr_registry.i',
        ],
        pysource          = [
            '__init__.py',
        ],
        export_includes   = self.top_dir,
        includes          = [self.top_dir, '.', self.repository_root.abspath()],
        swig_flags        = '-c++ -python -Wall',
        use               = 'usi SYSTEMC TLM PYTHON',
        install_path      = '${PREFIX}/lib',
    )

