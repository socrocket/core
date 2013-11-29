import sys

def options(self):
    """No options to declare"""
    pass

def configure(self):
    """If we run on windows we need winsocks"""
    if sys.platform == 'cygwin':
        self.check_cxx(lib='ws2_32', uselib_store='WINSOCK', mandatory=True)

