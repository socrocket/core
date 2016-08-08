#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
"""
    Repository Manager
    Extends Waf with the ability to handle git repositories.
"""
from __future__ import print_function
import os
import sys
import time
import json
import shlex
import shutil
import subprocess

WAF_REPO_LOCK='.lock_repo.json'
WAF_REPO_DB='.repository.json'
REPOS = {}

def update_check_for_uncommited(top_dir, directory):
    oldcwd = os.getcwd()
    os.chdir(os.path.join(top_dir, directory))
    ret = subprocess.call(['git', 'diff-index', '--quiet', 'HEAD'])
    os.chdir(oldcwd)
    return ret

def update_repository_info(top_dir, directory):
    oldcwd = os.getcwd()
    os.chdir(os.path.join(top_dir, directory))
    current_branch = subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip()
    current_remote = subprocess.check_output(["git", "config", "--get", "branch.{}.remote".format(current_branch)]).strip()
    current_merge = subprocess.check_output(["git", "config", "--get", "branch.{}.merge".format(current_branch)]).strip()
    remote_url = subprocess.check_output(["git", "config", "--get", "remote.{}.url".format(current_remote)]).strip()
    result = {directory: {"pull": { "remote_url": remote_url.strip(), "remote_branch": current_branch.strip()}}}
    try:
        remote_pushurl = subprocess.check_output(["git", "config", "--get", "remote.{}.pushurl".format(current_remote)]).strip()
        result[directory]["push"] = {"remote_url": remote_pushurl.strip(), "remote_branch": current_branch.strip()}
    except subprocess.CalledProcessError:
        pass
    os.chdir(oldcwd)
    return result

def update_repository_to_subtree(top_dir, directory, branch):
    # Delete repo to the right .git/info/exclude
    exfile = os.path.join(top_dir, ".git", "info", "exclude")
    with open(exfile) as f:
        linelist = []
        for item in f:
            rmitem = re.match(directory, item)
            if type(rmitem) == type(None):
                linelist.append(item)
        with open(exfile, "w") as f:
            f.truncate()
            for line in linelist:
                f.writelines(line)

    path, base = os.path.split(directory)
    olddir = os.path.join(top_dir, path, ".{}".format(base))
    # Move dir to .<directory>
    shutil.move(directory, olddir)
    # git subtree pull --prefix <directory> .<directory> <branch>
    subprocess.check_output(["git", "subtree", "add", "--prefix", directory, olddir, branch])
    # Copy over untracked files
    #git ls-files --exclude-standard --others --directory
    oldcwd = os.getcwd()
    os.chdir(olddir)
    untracked = subprocess.check_output(['git', 'ls-files', '--exclude-standard', '--others', '--directory'])
    os.chdir(oldcwd)
    for filename in untracked.split('\n'):
        shutil.move(os.path.join(olddir, filename), os.path.join(directory, filename))

def update_repositories(top_dir, repos):
    for repository in repos.keys():
        if repository == "core":
            continue
        if update_check_for_uncommited(top_dir, repository):
            print("You have uncommited changes in '{}'. Please commit your changes befor executing '{}' again.".format(repository, ' '.join(sys.argv)))
            sys.exit(1)
    for repository in repos.keys():
        if repository == "core":
            continue
        update_repository_to_subtree(top_dir, repository, repos[repository]["pull"]["remote_branch"])

def read_repos(top_dir):
    """Read the repository database file"""
    if not top_dir or top_dir == '':
        top_dir = os.getcwd()
    if os.path.isfile(os.path.join(os.path.abspath(top_dir), WAF_REPO_DB)):
        with open(top_dir+os.sep+WAF_REPO_DB, "r") as jsonfile:
            obj = json.load(jsonfile)
            jsonfile.close()
            return obj
    elif os.path.isfile(os.path.join(os.path.abspath(top_dir), WAF_REPO_LOCK)):
        with open(top_dir+os.sep+WAF_REPO_LOCK, "r") as jsonfile:
            old = json.load(jsonfile)
            obj = {}
            for directory in old.keys():
                obj.update(update_repository_info(top_dir, directory))
            update_repositories(top_dir, obj)
            jsonfile.close()
            return obj
    else:
        return update_repository_info(top_dir, "core")

def write_repos(top_dir, repos):
    """Write the repository database file"""
    with open(top_dir+os.sep+WAF_REPO_DB, "w") as jsonfile:
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
if "check_output" not in dir(subprocess): # duck punch it in!
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

def git_cmd(cmd, params):
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

def add_repo(cmd, params):
    import shutil
    global REPOS

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

    tempdir = os.path.join("build", "repo-%x" % int(time.time()))

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
    subprocess.call(["git", "subtree", "--prefix", directory, tempdir, 'master'])
    REPOS[directory] = {"pull": { "remote_url": repository, "remote_branch": 'master'}}
    shutils.rmtree(tempdir)

    # Adding repo to repository file
    for dep_params in vals.get("deps", {}).items():
        print("  Adding dependency %s" % dep_params[0])
        add_repo("add", dep_params)

def init_repo(cmd, params):
    vals = get_repo_vals("core")
    for dep_params in vals.get("deps", {}).items():
        print("  Adding dependency %s" % dep_params[0])
        add_repo("add", dep_params)
    
def del_repo(cmd, params):
    import shutil
    for directory in params:
        if directory == "core":
            print("You cannot delete the core repository")
            continue
        del(REPOS[directory])
        shutil.rmtree(directory)
        
def show_repo(cmd, params):
    global REPOS
    for directory, repository in REPOS.items():
        vals = get_repo_vals(directory)
        print("%s <= %s:%s" % (directory, repository['pull']['remote_url'], repository['pull']['remote_branch']))

        if "push" in repository:
            print("    upstream repository: %s:%s" % (repository['push']['remote_url'], repository['push']['remote_branch']))
        print("    name: %s" % vals.get("name", ""))
        print("    description: %s" % vals.get("desc", ""))
        print("    default path: %s" % vals.get("path", ""))

        tools = vals.get("tools", [])
        if len(tools) > 0:
            print("    tools: %s" % ', '.join(tools))

        dependencies = vals.get('deps', {}).items()
        if len(dependencies) > 0:
            print("    dependencies:")
            for directory, repository in dependencies:
                print("        %s <= %s" % (directory, repository))
        print("")

def show_help(cmd, params):
    print("usage: %s <comands> <parameters...>" % (' '.join(sys.argv[0:2])))
    print()
    print("Comands:")
    print("  help")
    print("  add <directory> <repository>")
    print("  rm <directory...>")
    print("  show")
    print("")

def execute(top_dir):
    global REPOS
    REPOS = read_repos(top_dir)
    oldcwd = os.getcwd()
    os.chdir(top_dir)
    CMDS = {
        'init': init_repo,
        'pull': git_cmd, 
        'push': git_cmd, 
        'add': add_repo, 
        'del': del_repo, 
        'show': show_repo,
        'help': show_help
    }

    argv = sys.argv[2:]
    if len(argv) == 0:
        show_repo(None, None)
    else:
        cmd = argv[0]
        params = argv[1:]

        if cmd not in CMDS:
            show_help(cmd, params)
        else:
            CMDS[cmd](cmd, params)
    write_repos(top_dir, REPOS)
    os.chdir(oldcwd)
