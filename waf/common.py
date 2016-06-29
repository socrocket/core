#! /usr/bin/env python
# vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 filetype=python :
from __future__ import print_function
import os
import fnmatch
import subprocess
from core.tools.repository import read_repos
from waflib import Errors, Context, Utils, Options, Build, Configure, TaskGen

def options(self): 
    """Setting default waf options right for SoCRocket"""
    #common = self.get_option_group("--prefix")
    #common.set_title("Common Configuration Options")
    #cpp = self.get_option_group("--check-cxx-compiler")
    #cpp.set_title("C++ Compiler Configuration Options")
    #inst = self.get_option_group("--force")
    #inst.remove_option("--destdir")
    #inst.remove_option("--force")
    #inst.set_title("Configuration Options")
    #inst.set_description(
    #    """All following options can be provided to the configure rule."""
    #)
    self.add_option(
        "--verbosity", 
        dest="verbosity", 
        help="Defines the verbosity for the build", 
        default=os.environ.get("VERBOSITY",'4')
    )

def configure(self):
    """Standard tools needed for fetching and building"""
    print("""
      To compile SoCRocket you need to have installed a basic Unix system.
      Atleast it needs to contain: 

        nm, git, patch, make, wget / curl, tar, ln, bash ans swig, python.

      Moreover you will need the development packages from readline:

        libreadline-dev

      Furthermore you need to provide cross compiler for sparc. If not provided
      we will try to fetch them from Gaisler (sparc-elf-gcc). But you will need 
      to provice a 32bit environment to execute the precompiled toolchain.

      For the ipython support blas and lapack needs to be installed.
    """)
    self.find_program('nm', mandatory=1, var='NM')
    self.find_program('git', var='GIT', mandatory=True, okmsg="ok")
    self.find_program('patch', var='PATCH', mandatory=True, okmsg="ok")
    self.find_program('make', var='MAKE', mandatory=True, okmsg="ok")
    self.find_program('wget', var='WGET', mandatory=False, okmsg="ok")
    if 'WGET' not in self.env: # fallback to 'curl'
        self.find_program('curl', var='CURL', mandatory=True, okmsg="ok" )
    self.find_program('tar', var='TAR', mandatory=True, okmsg="ok")
    self.find_program('ln', var='LN', mandatory=True, okmsg="ok")
    self.find_program('bash', var='BASH', mandatory=True, okmsg="ok")

    if self.options.jobs:
        self.env["JOBS"] = "-j%d" % self.options.jobs

    # Set verbosity level
    if self.options.verbosity:
        self.env.append_unique('DEFINES', 'GLOBALVERBOSITY=%s' % self.options.verbosity)

    if os.path.exists(os.path.join(self.srcnode.abspath(), "socrocket.md")):
        self.env["IS_SOCROCKET"] = True
    else:
        self.env["IS_SOCROCKET"] = False
    if hasattr(self.options, "socrocekt"):
        self.env["SOCROCKET_HOME"] = self.options.socrocket

def conf(f):
	def fun(*k,**kw):
		mandatory=True
		if'mandatory'in kw:
			mandatory=kw['mandatory']
			del kw['mandatory']
		try:
			return f(*k,**kw)
		except Errors.ConfigurationError as e:
			if mandatory:
				raise e
	setattr(Options.OptionsContext,f.__name__,fun)
	setattr(Configure.ConfigurationContext,f.__name__,fun)
	setattr(Build.BuildContext,f.__name__,fun)
	return f

#
# Find all project targets:
# ./waf list
#
#def target_list(ctx): 
#  """returns a list of all targets"""
#
#  bld = Build.BuildContext() 
#  proj = Environment.Environment(Options.lockfile) 
#  bld.load_dirs(proj['srcdir'], proj['blddir']) 
#  bld.load_envs()
#
#  bld.add_subdirs([os.path.split(Utils.g_module.root_path)[0]]) 
#
#  nlibs = set([])
#  ntests = set([])
#  napps = set([])
#  for x in bld.all_task_gen:
#    try:
#      if "test" in x.features:
#        ntests.add(x.name or x.target)
#      elif "cprogram" in x.features:
#        napps.add(x.name or x.target)
#      elif "cstaticlib" in x.features:
#        nlibs.add(x.name or x.target)
#        
#    except AttributeError:
#      pass
#
#  libs  = list(nlibs)
#  tests = list(ntests)
#  apps = list(napps)
#  
#  libs.sort()
#  tests.sort()
#  apps.sort()
#  print "Library targets:"
#  for name in libs:
#    print " ", name
#  print ""
#
#  print "Test targets:"
#  for name in tests:
#    print " ", name
#  print ""
#  
#  print "Executable targets:"
#  for name in apps:
#    print " ", name

def macclean(self):
  """Clean garbage files from the source tree"""
  print(subprocess.call(['find', '.', '(', '-name', '*.DS_Store', '-o', '-name', '*~', '-o', '-name', '.*~', ')', '-print', '-delete'], shell=False, stderr=subprocess.STDOUT))

setattr(Context.g_module, 'macclean', macclean)
class Macclean(Build.BuildContext):
    cmd = 'macclean'
    fun = 'macclean'

def get_subdirs(self,path='.'):
    """Return a list of all subdirectories from path"""
    REPOS = read_repos(Context.top_dir)
    result = [name
            for name in os.listdir(path) 
                if os.path.isfile(os.path.join(path, name, "wscript")) and (not os.path.relpath(
                    os.path.join(path,name), Context.top_dir) in REPOS.keys() or name == 'core') and not name.startswith('.')]
    return result
conf(get_subdirs)

def recurse_all(self):
    """Extend the Configuration to recurse all subdirectories"""
    self.recurse(self.get_subdirs(self.path.abspath()))
conf(recurse_all)

def recurse_all_tests(self):
    """Extend the Configuration to recurse all subdirectories tests dirs"""
    self.recurse(get_testdirs(self.path.abspath()))
conf(recurse_all_tests)

def get_testdirs(path='.'):
    """Return a list of all test subdirectories of path"""
    return [os.path.join(name, "tests") 
              for name in os.listdir(path) 
                  if os.path.isfile(
                      os.path.join(path, name, "tests", "wscript"))]

def getdirs(base = '.', excludes = list()):
    """
    For external resources
    Not used at the moment, maybe with grlib
    Return recursively all subdirectories of base, 
    exept directories matching excludes.
    """
    result = []
    for root, dirs, files in os.walk(base):
        if any([fnmatch.fnmatchcase(root, exc) for exc in excludes]):
            continue
        result.append(root)
    return result

def getfiles(base = '.', ext = list('*'), excludes = list()):
    """Return recursively all files in base with an extension in ext excluding all who matching excludes"""
    result = []
    for root, dirs, files in os.walk(base):
        for cdir in dirs:
            if any([fnmatch.fnmatchcase(os.path.join(root, cdir), exc) for exc in excludes]):
                del(cdir)
        
        for cfile in files:
            if any([fnmatch.fnmatchcase(os.path.join(root, cfile), exc) for exc in excludes]):
                continue
            if any([fnmatch.fnmatchcase(os.path.join(root, cfile), e) for e in ext]): 
                result.append(os.path.join(root, cfile))
            
    return result

@TaskGen.before('process_source', 'process_rule')
@TaskGen.feature('cxxstlib')
def export_has_define(self):
  defines = getattr(self, 'export_defines', [])
  defines = Utils.to_list(defines)
  defines += ["HAVE_" + self.target.replace(".", "_").upper()]
  setattr(self, "export_defines", defines)

