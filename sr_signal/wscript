#! /usr/bin/env python
# vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 filetype=python :
top = '../..'
REPOSITORY_PATH = "core/sr_signal"
REPOSITORY_NAME = "SoCRocket TLM Signals"
REPOSITORY_VERSION = [3,0,0]
REPOSITORY_DESC = """High level signaling library"""

def build(self):
    features = 'cxx cxxstlib'
    source = []

    if True:
        source += ['sr_signal.i']
        features += ' pyembed venv_package'

    self(
        target            = 'sr_signal',
        features          = features,
        source            = source,
        pysource          = ['__init__.py'],
        export_includes   = self.top_dir,
        includes          = [self.top_dir, '.', self.repository_root.abspath()],
        swig_flags        = '-c++ -python -Wall',
        use               = 'usi SYSTEMC TLM PYTHON',
        install_path      = '${PREFIX}/lib',
    )

