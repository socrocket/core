#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os,shlex,sys,time,json,subprocess,shutil
from waflib import Context,Scripting,TaskGen
from waflib.Configure import ConfigurationContext
from waflib import ConfigSet,Utils,Options,Logs,Context,Build,Errors
from core.waf.common import conf

WAF_CONFIG_LOG='repo.log'
WAF_REPO_LOCK='.lock_repo.json'
REPOS = {}

def read_repos():
    if os.path.isfile(Context.out_dir+os.sep+WAF_REPO_LOCK):
        with open(Context.out_dir+os.sep+WAF_REPO_LOCK, "r") as jsonfile:
            obj = json.load(jsonfile)
            jsonfile.close()
            return obj
    else:
        core = subprocess.check_output(["git", "config", "--get", "remote.origin.url"])
        return {"core": core.strip()}

def write_repos(repos):
    with open(Context.out_dir+os.sep+WAF_REPO_LOCK, "w") as jsonfile:
        json.dump(repos, jsonfile, sort_keys=True, indent=2, separators=(',', ': '))
        jsonfile.close()


def get_repo_vals(directory):
    prefix = "repository_"
    keys = ["name", "desc", "tools", "path"]
    result = {}
    obj = {}
    wscript = os.path.join(directory, "wscript")
    if os.path.isfile(wscript):
        execfile(wscript, {}, obj)
    for key in keys:
        name = (prefix + key).upper()
        result[key] = obj.get(name, "")
    return result


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

def init(self):
  if Options.commands[0] == "repo":
    del(Options.commands[1:])

class repo(ConfigurationContext):
    cmd = 'repo'
    fun = 'repo'
    def execute(self):
        self.init_dirs()
        self.cachedir=self.bldnode.make_node(Build.CACHE_DIR)
        self.cachedir.mkdir()
        path=os.path.join(self.bldnode.abspath(),WAF_CONFIG_LOG)
        self.logger=Logs.make_logger(path,'cfg')
        app=getattr(Context.g_module,'APPNAME','')
        if app:
            ver=getattr(Context.g_module,'VERSION','')
            if ver:
              app="%s (%s)"%(app,ver)
        if id(self.srcnode)==id(self.bldnode):
          Logs.warn('Setting top == out (remember to use "update_outputs")')
        elif id(self.path)!=id(self.srcnode):
          if self.srcnode.is_child_of(self.path):
            Logs.warn('Are you certain that you do not want to set top="." ?')
        global REPOS
        Context.top_dir=self.srcnode.abspath()
        Context.out_dir=self.bldnode.abspath()
        REPOS = read_repos()
        self.work()
        write_repos(REPOS)
        #self.store()

    def git_cmd(self, cmd, params):
        global REPOS
        for directory, repository in REPOS.iteritems():
            if directory == "core":
                directory = "."
            print "%s:" % (directory)
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
            print "add takes 2 parameters:"
            print "usage: %s <directory> <repository>" % (' '.join(sys.argv[0:3]))
            return

        elif len(params) == 1:
            directory = None
            repository = params[0]

        elif len(params) == 2:
            directory = params[0]
            repository = params[1]

        tempdir = "build/repo-tmp"

        try:
            import subprocess
            subprocess.call(("%(git)s clone %(repository)s %(directory)s" % {
                "git": "git",
                "directory": tempdir,
                "repository": repository,
                "parameter": params
            }).split())
        except CalledProcessError:
            shutil.rmtree(directory)

        
        vals = get_repo_vals(tempdir)
        if not directory:
            directory = vals["path"]

        if directory == "core":
            directory = "."

        shutil.move(tempdir, directory)
        REPOS[directory] = repository

    def mv_cmd(self, cmd, params):
        if len(params) != 2:
            print "mv takes 2 parameters:"
            print "File history will not integrated"
            print "usage: %s <from> <to>" % (' '.join(sys.argv[0:3]))
            return
        else:
            from_abs = os.path.abspath(params[0])
            from_repo = None
            to_abs = os.path.abspath(params[0])
            if os.path.exists(to_abs):
                to_abs = os.path.join(to_abs, os.path.basepath(from_abs))

            to_repo = None
            for directory, repository in REPOS.iteritems():
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
                print "%s <from> <to> can only be used in socrocket repos" % (' '.join(sys.argv[0:3]))
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
                print "You cannot delete the core repository"
                continue
            del(REPOS[directory])
            shutil.rmtree(directory)

    def show_repo(self, cmd, params):
        global REPOS
        for directory, repository in REPOS.iteritems():
            vals = get_repo_vals(os.path.join(self.path.abspath(), directory))
            print "%s <= %s" % (directory, repository)
            print "    name: %s" % vals["name"]
            print "    default path: %s" % vals["path"]
            print "    tools: %s" % ', '.join(vals["tools"])
            print "    description: %s" % vals["desc"]
            print ""

    def show_help(self, cmd, params):
        print "usage: %s <comands> <parameters...>" % (' '.join(sys.argv[0:2]))
        print
        print "Comands:"
        print "  help"
        print "  add <directory> <repository>"
        print "  rm <directory...>"
        print "  show"
        print ""

    def work(self):
        CMDS = {
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

            if not CMDS.has_key(cmd):
                self.show_help(cmd, params)
                return
            CMDS[cmd](cmd, params)

setattr(Context.g_module, 'init', init)
setattr(Context.g_module, 'repo', repo)

@TaskGen.before('process_source', 'process_rule')
@TaskGen.feature('cxx')
def export_have_define(self):
    defines = getattr(self, 'defines', [])
    defines = Utils.to_list(defines)
    REPOS = read_repos()
    for repo in REPOS:
        defines += ['HAVE_REPO_' + repo.replace('.', '_').upper()]
    setattr(self, 'defines', defines)

def loadrepos(self):
    REPOS = read_repos()
    for d, repo in REPOS.iteritems():
        directory = os.path.join(os.getcwd(),d)
        vals = get_repo_vals(directory)
        waf = os.path.join(directory, "waf")
        self.load(vals["tools"], tooldir=[waf])

conf(loadrepos)

def iterrepos(self):
    REPOS = read_repos()
    self.repositories = REPOS
    for d, repo in REPOS.iteritems():
        if d == "core":
            continue
        self.repository_root = self.srcnode.find_node(str(d))
        self.recurse([self.repository_root.abspath()])

conf(iterrepos)
def options(self):
    pass

def configure(self):
    pass
        
    
