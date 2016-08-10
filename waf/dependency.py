#! /usr/bin/env python
# vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 filetype=python :
import os
import sys
import shutil
from waflib import Errors, Context, Utils, Logs
from waflib.Logs import debug, error
from waflib.Tools import c_preproc
from core.waf.common import conf

def base(self, *k, **kw):
    name = kw.get("name", "Unnamed")
    base = kw.get("base", "%(name)s-%(version)s") % kw
    kw.update({"base":base})

    kw["BASE_PATH"] = kw.get("BASE_PATH", os.path.join(self.bldnode.abspath(), ".conf_check_deps")) % kw
    kw["BASE_PATH_FETCH"] = kw.get("BASE_PATH_FETCH", "%(BASE_PATH)s/fetch") % kw
    kw["BASE_PATH_SRC"] = kw.get("BASE_PATH_SRC", "%(BASE_PATH)s/src") % kw
    kw["BASE_PATH_BUILD"] = kw.get("BASE_PATH_BUILD", "%(BASE_PATH)s/build") % kw
    kw["BASE_PATH_DIST"] = kw.get("BASE_PATH_DIST", "%(BASE_PATH)s/dist") % kw

    kw["fdeps"] = kw.get("fallback", os.path.join(self.srcnode.abspath()))
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
    """Fetch Dependency Sourcen"""
    if "tar" in kw and os.path.isfile(os.path.join(kw['fdeps'], kw['tar'] % kw)):
        """First try the fallback ./deps folder"""
        fallback_file = os.path.join(kw['fdeps'], kw['tar'] % kw)
        shutil.copyfile(fallback_file, os.path.join(kw["BASE_PATH_FETCH"], kw['tar'] % kw))
    elif "git_url" in kw:
        """Then search for a git repo"""
        self.start_msg("Cloning %(name)s" % kw)
        git_url = kw.get("git_url")
        fetch_path = os.path.join(kw["BASE_PATH_FETCH"], kw["base"])
        if not os.path.isdir(fetch_path):
            self.cmd_and_log(
                [Utils.subst_vars('${GIT}',self.env), "clone", git_url, fetch_path], 
                output=Context.BOTH,
                cwd=kw["BASE_PATH_FETCH"]
            )
            if "git_checkout" in kw:
                self.cmd_and_log(
                    [Utils.subst_vars('${GIT}',self.env), "checkout", kw["git_checkout"]],
                    output=Context.BOTH,
                    cwd=fetch_path
                )
            self.end_msg("Ok")

        else:
            self.end_msg("Already done")
        if not os.path.isdir(kw["src"]):
            shutil.copytree(fetch_path, kw["src"])

    elif "tar_url" in kw:
        """Finaly try to download a tar file yourself"""
        kw["tar"] = kw.get("tar", "%(base)s.tar.gz") % kw
        tar_url = kw.get("tar_url", "") % kw
        self.start_msg("Fetching %s" % kw["name"])
        if not os.path.exists(os.path.join(kw["BASE_PATH_FETCH"], kw["tar"])):
            if os.path.exists(tar_url):
                shutil.copyfile(tar_url, os.path.join(kw["BASE_PATH_FETCH"], kw["tar"]))
            else:
                import urllib
                urllib.urlretrieve(tar_url, os.path.join(kw["BASE_PATH_FETCH"],kw["tar"]))
            self.end_msg("Ok")
        else:
            self.end_msg("Already done")

    if ("tar" in kw or "tar_url" in kw) and 'git_url' not in kw:
        """If there was a tar file (Either block 1 or 3) extract it"""
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
                [Utils.subst_vars('${TAR}',self.env), "-xf", fetch_path], 
                output=Context.BOTH, 
                cwd=os.path.dirname(kw["src"])
            )
            self.end_msg("Ok")
        else:
            self.end_msg("Already done")

    elif "git_url" not in kw:
        """If nothing of the above applied we are doomed"""
        self.fatal("You need to specify git_url, tar_url or tar")

    if "patch" in kw:
        """Try to patch the source if neccecary (patch is defined in the parameters)"""
        for patch in Utils.to_list(kw["patch"]):
           try:
               self.start_msg("Patching %s with %s" % (kw["name"], patch))
               self.cmd_and_log(
                   [Utils.subst_vars('${PATCH}',self.env), "-p1", "-Nsi", patch, "-d", kw["src"]],
                   output=Context.BOTH,
                   cwd=kw["src"],
               )
               self.end_msg("Ok")
           except Errors.WafError as e:
               self.end_msg("Failed, make sure patch %s was already applied" % patch)
    return k, kw


def dep_fetch(self, *k, **kw):
    """Fetches and extracts the source to dist"""
    k, kw = base(self, *k, **kw)
    kw["src"] = kw["prefix"]
    k, kw = fetch(self, *k, **kw)
conf(dep_fetch)

def dep_build(self, *k, **kw):
    """Configure/Make/Make install to the dist directory"""
    k, kw = base(self, *k, **kw)
    k, kw = fetch(self, *k, **kw)

    self.start_msg("Building %s" % kw["name"])
    if not os.path.isdir(kw["prefix"]):
        config_cmd = kw.get("config_cmd", "%(src)s/configure --prefix=%(prefix)s")
        build_cmd = kw.get("build_cmd", Utils.subst_vars('${MAKE} ${JOBS}',self.env))
        install_cmd = kw.get("install_cmd", Utils.subst_vars('${MAKE} ${JOBS} install', self.env))
        self.end_msg("...")

        if not os.path.exists(kw["build"]):
            if kw.get("build_dir", True):
                os.makedirs(kw["build"])

            self.start_msg("  Configure %s" % kw["name"])
            self.cmd_and_log(
                config_cmd % kw, 
                #(config_cmd % kw).split(' '), 
                output=Context.BOTH, 
                shell=True,
                cwd=kw.get("config_cwd",kw["build"]) % kw
            )
            self.end_msg("Ok")

            self.start_msg("  Compile %s" % kw["name"])
            self.cmd_and_log(
                build_cmd % kw, 
                #(build_cmd % kw).split(' '), 
                output=Context.BOTH, 
                shell=True,
                cwd=kw.get("build_cwd",kw["build"]) % kw
            )
            self.end_msg("Ok")
        
        if not os.path.exists(kw["prefix"]):
            self.start_msg("  Install %s" % kw["name"])
            self.cmd_and_log(
                install_cmd % kw, 
                #(install_cmd % kw).split(' '), 
                output=Context.BOTH, 
                shell=True,
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

