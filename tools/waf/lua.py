#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os

def options(self):
    self.add_option(
        '--with-lua', 
        type='string', 
        help='Lua installation directory', 
        dest='luadir', 
        default=os.environ.get("LUA_HOME")
    )

def find(self, path = None):
    """Check for Lua Library and Headers"""
    if path:
      libpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "lib"))))
      incpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "include"))))
    else:
      lualib = "/usr/lib"
      luadir = "/usr/include/lua5.1"

    self.check_cxx(
      lib          = 'lua5.1',
      uselib_store = 'LUA',
      mandatory    = True,
      libpath      = libpath,
      errmsg       = "LUA Library not found. Use --with-lua option or set $LUA_HOME.",
      okmsg        = "ok"
    ) 
    self.check_cxx(
      header_name  = 'lua.h',
      uselib_store = 'LUA',
      mandatory    = True,
      includes     = incpath,
      uselib       = 'LUA',
      okmsg        = "ok"
    ) 
    
    self.check_cxx(
      msg          = "Checking for Lua 5.1.4 or higher",
      mandatory    = True,
      execute      = True,
      fragment     = """
                     #include <lua.h>
                     int main(int argc, char *argv[]) {
                       return !(LUA_VERSION_NUM > 500);
                     }
                   """,
      uselib       = 'LUA',
      okmsg        = "ok",
    )
    if path:
        self.env["HOME_LUA"] = path

def configure(self):
    try:
        if self.options.elfdir:
            find(self, self.options.elfdir)
        else:
            find(self)
    except:
        name    = "lua"
        version = "5.2.2"
        self.dep_build(
            name    = name, 
            version = version,
            tar_url = "http://www.lua.org/ftp/%(base)s.tar.gz",
        )
        find(self, self.dep_path(name, version))



