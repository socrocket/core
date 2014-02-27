#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
#***********************************************************************#
#* Project:    HW-SW SystemC Co-Simulation SoC Validation Platform
#*                                                                    
#* File:       common.py                                              
#*             python file containing filesystem search helpers       
#*             to easiefy the wscript configuration routine.           
#*                                                                    
#* Modified on $Date$   
#*          at $Revision$                                         
#*                                                                     
#* Principal:  European Space Agency                                   
#* Author:     VLSI working group @ IDA @ TUBS                         
#* Maintainer: Rolf Meyer                                              
#***********************************************************************#

import os
import shutil
import fnmatch
import subprocess
from waflib import Errors, Context, Utils, Options, Build, Configure

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
    self.find_program('nm', mandatory=1, var='NM')
    self.find_program('git', var='GIT', mandatory=True, okmsg="ok")
    self.find_program('patch', var='PATCH', mandatory=True, okmsg="ok")
    self.find_program('make', var='MAKE', mandatory=True, okmsg="ok")
    self.find_program('wget', var='WGET', mandatory=True, okmsg="ok")
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
		except Errors.ConfigurationError ,e:
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
  print subprocess.call(['find', '.', '(', '-name', '*.DS_Store', '-o', '-name', '*~', '-o', '-name', '.*~', ')', '-print', '-delete'], shell=False, stderr=subprocess.STDOUT)

setattr(Context.g_module, 'macclean', macclean)
class Macclean(Build.BuildContext):
    cmd = 'macclean'
    fun = 'macclean'

def get_subdirs(path='.'):
    """Return a list of all subdirectories from path"""
    return [name 
            for name in os.listdir(path) 
                if os.path.isfile(os.path.join(path, name, "wscript"))]

def recurse_all(self):
    """Extend the Configuration to recurse all subdirectories"""
    self.recurse(get_subdirs(self.path.abspath()))
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
    Not used at the moment, maybe with greenreg or grlib
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

def base(self, *k, **kw):
    name = kw.get("name", "Unnamed")
    base = kw.get("base", "%(name)s-%(version)s") % kw
    kw.update({"base":base})

    kw["BASE_PATH"] = kw.get("BASE_PATH", os.path.join(self.bldnode.abspath(), ".deps")) % kw
    kw["BASE_PATH_FETCH"] = kw.get("BASE_PATH_FETCH", "%(BASE_PATH)s/fetch") % kw
    kw["BASE_PATH_SRC"] = kw.get("BASE_PATH_SRC", "%(BASE_PATH)s/src") % kw
    kw["BASE_PATH_BUILD"] = kw.get("BASE_PATH_BUILD", "%(BASE_PATH)s/build") % kw
    kw["BASE_PATH_DIST"] = kw.get("BASE_PATH_DIST", "%(BASE_PATH)s/dist") % kw

    kw["src"] = kw.get("src", os.path.join(kw["BASE_PATH_SRC"], kw["base"]))
    kw["build"] = kw.get("build", os.path.join(kw["BASE_PATH_BUILD"], kw["base"]))
    kw["prefix"] = kw.get("prefix", os.path.join(kw["BASE_PATH_DIST"], kw["base"]))

    if not os.path.isdir(kw["BASE_PATH_FETCH"]):
        os.makedirs(kw["BASE_PATH_FETCH"])
    if not os.path.isdir(kw["BASE_PATH_SRC"]):
        os.makedirs(kw["BASE_PATH_SRC"])
    if not os.path.isdir(kw["BASE_PATH_BUILD"]):
        os.makedirs(kw["BASE_PATH_BUILD"])
    if not os.path.isdir(kw["BASE_PATH_DIST"]):
        os.makedirs(kw["BASE_PATH_DIST"])
    return k, kw

def fetch(self, *k, **kw):
    if kw.has_key("git_url"):
        self.start_msg("Cloning %(name)s" % kw)
        git_url = kw.get("git_url")
        fetch_path = os.path.join(kw["BASE_PATH_FETCH"], kw["base"])
        if not os.path.isdir(fetch_path):
            self.cmd_and_log(
                [self.env.GIT, "clone", git_url, fetch_path], 
                output=Context.BOTH,
                cwd=kw["BASE_PATH_FETCH"]
            )
            if kw.has_key("git_checkout"):
                self.cmd_and_log(
                    [self.env.GIT, "checkout", kw["git_checkout"]],
                    output=Context.BOTH,
                    cwd=fetch_path
                )
            self.end_msg("Ok")

        else:
            self.end_msg("Already done")
        if not os.path.isdir(kw["src"]):
            shutil.copytree(fetch_path, kw["src"])

    elif kw.has_key("tar_url"):
        tar_url = kw.get("tar_url", "") % kw
        kw["tar"] = kw.get("tar", "%(base)s.tar.gz") % kw
        self.start_msg("Fetching %s" % kw["name"])
        if not os.path.exists(os.path.join(kw["BASE_PATH_FETCH"], kw["tar"])):
            self.cmd_and_log(
                [self.env.WGET, tar_url, "-q", "-O", kw["tar"]], 
                output=Context.BOTH, 
                cwd=kw["BASE_PATH_FETCH"]
            )
            self.end_msg("Ok")
        else:
            self.end_msg("Already done")

    if kw.has_key("tar") or kw.has_key("tar_url"):
        kw["tar"] = kw.get("tar", "%(base)s.tar.gz") % kw
        fetch_path = os.path.join(kw["BASE_PATH_FETCH"], kw["tar"])
        self.start_msg("Extracting %s" % kw["name"])
        if not os.path.exists(fetch_path):
            msg = kw.get("no_tar_msg",
                "Searched for %(tar)s and it was not found. "
                "please download it yourself an place it in %(path)s"
            )
            self.fatal(msg % {"tar": kw["tar"], "path": kw["BASE_PATH_FETCH"]})
        if not os.path.isdir(kw["src"]):
            self.cmd_and_log(
                [self.env.TAR, "-xf", fetch_path], 
                output=Context.BOTH, 
                cwd=kw["BASE_PATH_SRC"]
            )
            self.end_msg("Ok")
        else:
            self.end_msg("Already done")

    elif not kw.has_key("git_url"):
        self.fatal("You need to specify git_url, tar_url or tar")

    if kw.has_key("patch"):
        self.start_msg("Patching %s" % kw["name"])
        try:
            self.cmd_and_log(
                [self.env.PATCH, "-p1", "-Nsi", kw["patch"], "-d", kw["src"]],
                output=Context.BOTH, 
                cwd=kw["src"],
            )
            self.end_msg("Ok")
        except:
            self.end_msg("Faild, make sure it was already applied")

    return k, kw


def dep_fetch(self, *k, **kw):
    k, kw = base(self, *k, **kw)
    kw["src"] = kw["prefix"]
    k, kw = fetch(self, *k, **kw)
conf(dep_fetch)

def dep_build(self, *k, **kw):
    k, kw = base(self, *k, **kw)
    k, kw = fetch(self, *k, **kw)

    self.start_msg("Building %s" % kw["name"])
    if not os.path.isdir(kw["prefix"]):
        config_cmd = kw.get("config_cmd", "%(src)s/configure --prefix=%(prefix)s")
        build_cmd = kw.get("build_cmd", "%s %s" % (self.env.MAKE, self.env.JOBS))
        install_cmd = kw.get("install_cmd", "%s %s install" % (self.env.MAKE, self.env.JOBS))
        self.end_msg("...")

        if not os.path.exists(kw["build"]):
            if kw.get("build_dir", True):
                os.makedirs(kw["build"])

            self.start_msg("  Configure %s" % kw["name"])
            self.cmd_and_log(
                    (config_cmd % kw).split(' '), 
                output=Context.BOTH, 
                cwd=kw.get("config_cwd",kw["build"]) % kw
            )
            self.end_msg("Ok")

            self.start_msg("  Compile %s" % kw["name"])
            self.cmd_and_log(
                (build_cmd % kw).split(' '), 
                output=Context.BOTH, 
                cwd=kw.get("build_cwd",kw["build"]) % kw
            )
            self.end_msg("Ok")
        
        if not os.path.exists(kw["prefix"]):
            self.start_msg("  Install %s" % kw["name"])
            self.cmd_and_log(
                (install_cmd % kw).split(' '), 
                output=Context.BOTH, 
                cwd=kw.get("install_cwd",kw["build"]) % kw
            )
            self.end_msg("Ok")
    else:
        self.end_msg("Already done")

conf(dep_build)

def dep_path(self, name, version, *k, **kw):
    kw.update({
        "name": name,
        "version": version
    })
    k, kw = base(self, *k, **kw)
    return kw["prefix"]
conf(dep_path)

