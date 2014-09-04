#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os

def options(self):
    self.add_option(
        "--with-sdl", 
        type='string', 
        dest="sdldir", 
        help="Basedir of your SDL installation", 
        default=os.environ.get("SDL_HOME")
    )

def find(self, path = None):
    libpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "lib"))))
    incpath = os.path.abspath(os.path.expanduser(os.path.expandvars(os.path.join(path, "include"))))

    self.check_cxx(
      lib          = 'SDL',
      uselib_store = 'SDL',
      mandatory    = False,
      libpath      = libpath,
      errmsg       = "SDL Library not found. Use --sdl option or set $SDL.",
      okmsg        = "ok"
    ) 
    self.check_cxx(
      header_name  = 'SDL/SDL.h',
      uselib_store = 'SDL',
      mandatory    = False,
      includes     = incpath,
      uselib       = 'SDL',
      okmsg        = "ok"
    ) 
    
    self.check_cxx(
      msg          = "Checking for SDL 1.2 or higher",
      mandatory    = False,
      execute      = True,
      fragment     = """
                     #include <SDL/SDL_version.h>
                     int main(int argc, char *argv[]) {
                       return SDL_VERSION_ATLEAST(1, 2, 0);
                     }
                     """,
      uselib       = 'SDL',
      okmsg        = "ok",
    )

def configure(self):
    try:
        if self.options.sdldir:
            find(self, self.options.sdldir)
        else:
            find(self)
    except:
        name    = "SDL"
        version = "1.2.15"
        self.dep_build(
            name    = name, 
            version = version,
            tar_url = "http://www.libsdl.org/release/%(base)s.tar.gz",
        )
        find(self, self.dep_path(name, version))

