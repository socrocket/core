#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
"""
    Repository Manager
    Extends Waf with the ability to handle git repositories.
"""
from __future__ import print_function
import os,shlex,sys,time,json,subprocess,shutil
from waflib import Context,Scripting,TaskGen
from waflib import ConfigSet,Utils,Options,Logs,Context,Build,Errors
from core.waf.common import conf
from core.waf.subcommand import subcommand, SubcommandContext
import time

WAF_REPO_LOCK='.lock_repo.json'
REPOS = {}

def read_repos():
    """Read the repository database file"""
    if os.path.isfile(Context.out_dir+os.sep+WAF_REPO_LOCK):
        with open(Context.out_dir+os.sep+WAF_REPO_LOCK, "r") as jsonfile:
            obj = json.load(jsonfile)
            jsonfile.close()
            return obj
    else:
        core = subprocess.check_output(["git", "config", "--get", "remote.origin.url"])
        return {"core": core.strip()}

def write_repos(repos):
    """Write the repository database file"""
    with open(Context.out_dir+os.sep+WAF_REPO_LOCK, "w") as jsonfile:
        json.dump(repos, jsonfile, sort_keys=True, indent=2, separators=(',', ': '))
        jsonfile.close()


def get_repo_vals(directory):
    """Return configuration key value pairs for a repository directory"""
    prefix = "repository_"
    keys = ["name", "desc", "tools", "path", "version", "deps"]
    result = {}
    obj = {}
    wscript = os.path.join(directory, "wscript")
    if os.path.isfile(wscript):
        with open(wscript, "r") as script:
            code = compile(script.read(), wscript, 'exec')
            exec(code, {}, obj)
    if "REPOSITORY" in obj:
        return obj["REPOSITORY"]
    else:
        for key in keys:
            name = (prefix + key).upper()
            if name in obj:
              result[key] = obj[name]
        return result

# for python 2.6 compatibility
if "check_output" not in dir( subprocess ): # duck punch it in!
    def f(*popenargs, **kwargs):
        if 'stdout' in kwargs:
            raise ValueError('stdout argument not allowed, it will be overridden.')
        process = subprocess.Popen(stdout=subprocess.PIPE, *popenargs, **kwargs)
        output, unused_err = process.communicate()
        retcode = process.poll()
        if retcode:
            cmd = kwargs.get("args")
            if cmd is None:
                cmd = popenargs[0]
            raise subprocess.CalledProcessError(retcode, cmd)
        return output
    subprocess.check_output = f

import re

def replace(oldstr, newstr, infile):
    '''
       Sed-like Replace function..
    '''
    linelist = []
    with open(infile) as f:
        for item in f:
            newitem = re.sub(oldstr, newstr, item)
            linelist.append(newitem)
        with open(infile, "w") as f:
            f.truncate()
            for line in linelist: f.writelines(line)

def rmlinematch(oldstr, infile):
    '''
       Sed-like line deletion function based on given string..
    '''
    linelist = []
    with open(infile) as f:
        for item in f:
            rmitem = re.match(oldstr, item)
            if type(rmitem) == type(None):
                linelist.append(item)
        with open(infile, "w") as f:
            f.truncate()
            for line in linelist: f.writelines(line)

@subcommand
class repo(SubcommandContext):
    """
       The repo Context
       In here all work with the repositories is done.
       The database handlers are extern so the repository database is read in other contextes too.
    """
    cmd = 'repo'
    fun = 'repo'

    def git_cmd(self, cmd, params):
        global REPOS
        for directory, repository in REPOS.items():
            if directory == "core":
                directory = "."
            print("%s:" % (directory))
            import subprocess
            subprocess.call(("%(git)s %(cmd)s %(parameter)s" % {
              "git": "git",
              "directory": directory,
              "cmd": cmd,
              "parameter": ' '.join(params)
            }).split(), cwd=directory )

    def add_repo(self, cmd, params):
        import shutil

        if len(params) < 1:
            print("add takes 2 parameters:")
            print("usage: %s <directory> <repository>" % (' '.join(sys.argv[0:3])))
            return

        elif len(params) == 1:
            directory = None
            repository = params[0]

        elif len(params) == 2:
            directory = params[0]
            repository = params[1]
            if os.path.isdir(directory):
                print("Target directory does already exist '%s'" % directory)
                return

        tempdir = os.path("build", "repo-%x" % int(time.time()))

        try:
            print("Fetching repository %s" % repository)
            import subprocess
            subprocess.call(("%(git)s clone %(repository)s %(directory)s" % {
                "git": "git",
                "directory": tempdir,
                "repository": repository,
                "parameter": params
            }).split())
        except CalledProcessError:
            print("An error occured while repository creation on %s" % repository)
            shutil.rmtree(directory)

        
        vals = get_repo_vals(tempdir)
        if not directory:
            directory = vals["path"]

        if directory == "core":
            directory = "."

        print("Moving repository %s to %s" % (repository, directory))
        if os.path.isdir(directory):
            print("Target directory does already exist '%s'" % directory)
            return
        shutil.move(tempdir, directory)
        REPOS[directory] = repository
        for dep_params in vals.get("deps", {}).items():
            print("  Adding dependency %s" % dep_params[0])
            self.add_repo("add", dep_params)

    def init_repo(self, cmd, params):
        vals = get_repo_vals("core")
        for dep_params in vals.get("deps", {}).items():
            print("  Adding dependency %s" % dep_params[0])
            self.add_repo("add", dep_params)
        
    def mv_cmd(self, cmd, params):
        if len(params) != 2:
            print("mv takes 2 parameters:")
            print("File history will not integrated")
            print("usage: %s <from> <to>" % (' '.join(sys.argv[0:3])))
            return
        else:
            from_abs = os.path.abspath(params[0])
            from_repo = None
            to_abs = os.path.abspath(params[0])
            if os.path.exists(to_abs):
                to_abs = os.path.join(to_abs, os.path.basepath(from_abs))

            to_repo = None
            for directory, repository in REPOS.items():
                repo_abs = None
                if directory == "core":
                    repo_abs = self.path.abspath()
                else:
                    repo_abs = os.path.join(self.path.abspath(), directory)
                if from_abs.startswith(repo_abs):
                    from_repo = directory
                if to_abs.startswith(repo_abs):
                    to_repo = directory
                if from_repo and to_repo:
                    break

            if not from_repo or not to_repo:
                print("%s <from> <to> can only be used in socrocket repos" % (' '.join(sys.argv[0:3])))
                return

            if from_repo == to_repo:
                git_cmd(self, "mv", [from_abs[len(from_repo):], to_abs[len(to_repo):]])
            else:
                shutil.move(from_abs, to_abs)
                git_cmd(self, "rm", ["-rf", from_abs[len(from_repo):]])
                git_cmd(self, "add", [to_abs[len(to_repo):]])


    def del_repo(self, cmd, params):
        import shutil
        for directory in params:
            if directory == "core":
                print("You cannot delete the core repository")
                continue
            del(REPOS[directory])
            shutil.rmtree(directory)

    def show_repo(self, cmd, params):
        global REPOS
        for directory, repository in REPOS.items():
            vals = get_repo_vals(os.path.join(self.path.abspath(), directory))
            print("%s <= %s" % (directory, repository))
            print("    name: %s" % vals.get("name", ""))
            print("    description: %s" % vals.get("desc", ""))
            print("    default path: %s" % vals.get("path", ""))
            print("    tools: %s" % ', '.join(vals.get("tools", "")))
            print("    dependencies:")
            for directory, repository in vals.get("deps", {}).items():
                print("        %s <= %s" % (directory, repository))
            print("")

    def show_help(self, cmd, params):
        print("usage: %s <comands> <parameters...>" % (' '.join(sys.argv[0:2])))
        print()
        print("Comands:")
        print("  help")
        print("  add <directory> <repository>")
        print("  rm <directory...>")
        print("  show")
        print("")

    def work(self):
        global REPOS
        REPOS = read_repos()
        CMDS = {
            'init': self.init_repo,
            'pull': self.git_cmd, 
            'push': self.git_cmd, 
            'update': self.git_cmd, 
            'commit': self.git_cmd, 
            'diff': self.git_cmd,
            'status': self.git_cmd,
            'add': self.add_repo, 
            'del': self.del_repo, 
            'mv': self.mv_cmd,
            'show': self.show_repo,
            'help': self.show_help
        }

        argv = sys.argv[2:]
        if len(argv) == 0:
            self.show_repo(None, None)
        else:
            cmd = argv[0]
            params = argv[1:]

            if cmd not in CMDS:
                self.show_help(cmd, params)
            else:
                CMDS[cmd](cmd, params)
        write_repos(REPOS)

@TaskGen.before('process_source', 'process_rule')
@TaskGen.feature('cxx')
def export_have_define(self):
    """This function extends the C compiler functionality to register precompiler defines for each repository"""
    defines = getattr(self, 'defines', [])
    defines = Utils.to_list(defines)
    REPOS = read_repos()
    for repo in REPOS:
        defines += ['HAVE_REPO_' + repo.replace('.', '_').replace('/', '_').upper()]
    setattr(self, 'defines', defines)

def loadrepos(self):
    """Load repositories"""
    REPOS = read_repos()
    for d, repo in REPOS.items():
        directory = os.path.join(os.getcwd(),d)
        vals = get_repo_vals(directory)
        waf = os.path.join(directory, "waf")
        self.load(vals.get("tools", []), tooldir=[waf])

conf(loadrepos)

def iterrepos(self):
    """
        Iterate through repositories.
        It is simmilar to recurse.
        But it does not work on subdirectories but repos.
    """
    REPOS = read_repos()
    self.repositories = REPOS
    for d, repo in REPOS.items():
        if d == "core":
            continue
        self.repository_root = self.srcnode.find_node(str(d))
        self.recurse([self.repository_root.abspath()])

conf(iterrepos)
def options(self):
    pass

def configure(self):
    pass
        
    
