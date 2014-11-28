SoCRocket Repository Manager {#repository_p}
============================================

SoCRocket is providing a repository manager as small layer on top of git.
Hence the dependency structures projects in the scientific domain have the 
number of requirements in form of legal, copyright or simply API stability is rising.
To counter these complexity SoCRocket will provide the ability to split project 
specific code from public code.

Moreover it provides the ability that different code can optionaly depend on each other.

Let's look at an example:
First we fetch the SoCRocket Core Repository which provides us with the basic feature set and functinality:

    $ git clone git@brauhaus.c3e.cs.tu-bs.de:socrocket-core.git socrocket
    $ cd socrocket                                                       

But we want to play with some video IO and build some nice filters for it.
So we add the socrocket-media repository to socrocket:

    $ ./waf repo add git@brauhaus.c3e.cs.tu-bs.de:socrocket-media.git

A new folder will appear in the main folder: media. It will contain all the sources from the media repository.

Building the Project is still the same. Just execute the following:

    $ ./waf configure
    $ ./waf build --target=leon3mp.platform

Newly added components can be build as well by using `--target`.
Morover `./waf list` will list all targets from all repositories.

Repositories are simply git repositories and can be handled as such. See http://git-scm.com for information.
The usual commit, push, pull cycles are allowd as well as all other diry voodoo.

The additions are that it you specify a git folder as a SoCRocket Repository waf will be aware of it.
That means that in your code you can check for the availability of repositories:

Either in the `wscript`:
~~~
def build(self):
    if "media" in self.repositories:                              # Check if we have the media repository
        self(
            # ...
            export_includes = self.repository_root.abspath()      # The Absolute repository root for the current repository
            # ...
        )
~~~

Or in your SystemC Code:
~~~{.cpp}
#ifdef HAVE_REPOSITORY_MEDIA
// ...
~~~

To declare a SoCRocket Repository out of a git repository add a toplevel wscript with the following constant definitions at the top:
~~~
top='..'
REPOSITORY_PATH='media'                       # Default location inside the SoCRocket Working Folder
REPOSITORY_NAME='SoCRocket Media Repository'  # A human readable tile to identify the repository
REPOSITORY_DESC='Some Video IO'               # A human readable description of the repository
REPOSITORY_TOOLS=['av', 'sdl']                # Tools used by your repository. Must be located in the `waf`-folder in your repository

def options(self):
    pass  # Repository specific cli options

def configuration(self):
    pass  # Repository specific configuration

def build(self):
    pass  # Repository specific target building
~~~

To manage installed repositories we export some more commands to use:

    $ ./waf repo <command>

Here command can be:
* show: List all installed repositories
* rm: Remove specific repository

