import sys

def options(self):
    """No options to declare"""
    pass

def configure(self):
    """If we run on windows we need winsocks"""
    if sys.platform in ['cygwin', 'mingw', 'msys', 'msys2', 'win32']:
        self.check_cxx(lib='ws2_32', uselib_store='WINSOCK', mandatory=True)

