#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-

import os

def build(bld):
    if not "repository_root" in bld:
        if bld.env['LICENSE'] == 'gpl':
            bld.recurse(os.path.join('libbfd', 'elfloader'))
        else:
            bld.recurse(os.path.join('libelf', 'elfloader'))

        bld.recurse('common debugger modules osemu profiler')

        bld.install_files(os.path.join(bld.env.PREFIX, 'include'), 'trap.hpp')

    else:
        bld(
            target  = 'trap',
            features = 'cxx cxxstlib',
            source  = [
                        'osemu/osemu_base.cpp',
                        'profiler/profiler_elements.cpp',
                        'common/report.cpp',
                        'modules/register/scireg.cpp',
                        'debugger/gdb_connection_manager.cpp',
            ],
            export_includes = [self.repository_root.abspath()],
            includes        = [self.repository_root.abspath()],
            use             = 'BOOST SYSTEMC TLM AMBA GREENSOCS ELF_LIB',
            install_path    = '${PREFIX}/lib',
        )

