#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
"""
    Repository Manager
    Extends Waf with the ability to handle git repositories.
"""
from __future__ import print_function
import os
from core.waf.common import conf
from core.tools.repository import read_repos, get_repo_vals
from waflib import Context,TaskGen,Utils,Errors

REPOS = None

@TaskGen.before('process_source', 'process_rule')
@TaskGen.feature('cxx')
def export_have_define(self):
    """This function extends the C compiler functionality to register precompiler defines for each repository"""
    global REPOS
    defines = getattr(self, 'defines', [])
    defines = Utils.to_list(defines)
    if not REPOS:
        REPOS = read_repos(Context.top_dir)
    for repo in REPOS.keys():
        defines += ['HAVE_REPO_' + repo.replace('.', '_').replace('/', '_').upper()]
    setattr(self, 'defines', defines)

def loadrepos(self):
    """Load repositories"""
    global REPOS
    if not REPOS:
        REPOS = read_repos(Context.top_dir)
    for d, repo in REPOS.items():
        directory = os.path.join(Context.top_dir,d)
        if d == ".waf":
            continue
        vals = get_repo_vals(directory)
        waf = os.path.join(directory, "waf")
        for val in vals.get("tools", []):
            try:
                self.load(val, tooldir=[waf])
            except (ImportError, self.errors.ConfigurationError):
                self.load(val, tooldir=None)

conf(loadrepos)

def iterrepos(self):
    """
        Iterate through repositories.
        It is simmilar to recurse.
        But it does not work on subdirectories but repos.
    """
    global REPOS
    if not REPOS:
        REPOS = read_repos(Context.top_dir)
    self.repositories = REPOS
    for d, repo in REPOS.items():
        if d == "core" or d == ".waf":
            continue
        self.repository_root = self.srcnode.find_node(str(d))
        self.recurse([self.repository_root.abspath()])

conf(iterrepos)
def options(self):
    pass

def configure(self):
    pass
