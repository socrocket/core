#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os,shlex,sys,time,json,subprocess
from waflib import Context,Scripting,TaskGen
from waflib.Configure import ConfigurationContext
from waflib import ConfigSet,Utils,Options,Logs,Context,Build,Errors
from core.waf.common import conf

WAF_CONFIG_LOG='repo.log'
REPOS = {}

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
        REPOS = self.read_repos()
        self.work()
        self.write_repos(REPOS)
        self.store()

    def read_repos(self):
        if os.path.isfile(Context.out_dir+os.sep+".conf_check_repos.json"):
            with open(Context.out_dir+os.sep+".conf_check_repos.json", "r") as jsonfile:
                obj = json.load(jsonfile)
                jsonfile.close()
                return obj
        else:
            core = subprocess.check_output(["git", "config", "--get", "remote.origin.url"])
            return {"core": core}

    def write_repos(self, repos):
        with open(Context.out_dir+os.sep+".conf_check_repos.json", "w") as jsonfile:
            json.dump(repos, jsonfile)
            jsonfile.close()

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
        if len(params) != 2:
            print "add takes 2 parameters:"
            print "usage: %s <directory> <repository>" % (' '.join(sys.argv[0:3]))
            return
        directory = params[0]
        repository = params[1]
        if directory == "core":
            directory = "."

        REPOS[directory] = repository
        import subprocess
        subprocess.call(("%(git)s clone %(repository)s %(directory)s" % {
            "git": "git",
            "directory": directory,
            "repository": repository,
            "parameter": params
        }).split())

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
            print "    description: %s" % vals["description"]
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
    for repo in REPOS:
        defines += ['HAVE_REPO_' + repo.replace('.', '_').upper()]
    setattr(self, 'defines', defines)

def loadrepos(self):
    for d, repo in REPOS.iteritems():
        directory = os.path.join(self.root.abspath(),d)
        vals = get_repo_vals(directory)
        waf = os.path.join(directory, "waf")
        self.load(vals["tools"], waf)

conf(loadrepos)

def iterrepos(self):
    for d, repo in REPOS.iteritems():
        self.recurse(os.path.join(self.root.abspath(),d))

conf(iterrepos)
def options(self):
    pass

def configure(self):
    pass
        
    
