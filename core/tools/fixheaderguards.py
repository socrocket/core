from __future__ import print_function
from builtins import str
#!/usr/bin/env python

################################################################################
#   Include Guard Fixer
#   Copyright (C) 2011-2012 Martin Preisler <preisler.m@gmail.com>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
################################################################################

import re
import os.path

def default_guard_define_name_generator(filename, directory):
    altered_filename = filename.upper()
    altered_filename = altered_filename.replace(".", "_")
    
    dirnames_str = "_"
    dirnames_current = directory
    while dirnames_current:
        altered_dirname = str(os.path.basename(dirnames_current)).upper()
        altered_dirname = altered_dirname.replace(".", "_")
        
        assert("/" not in altered_dirname)
        
        dirnames_str = "%s%s%s" % ("" if "/" not in dirnames_current else "_", altered_dirname, dirnames_str)
        dirnames_current = os.path.dirname(dirnames_current)
    
    return "%s%s_" % (dirnames_str, altered_filename)

def clean_include_guard(file, guard_define_name_generator, check_only):
    filename = os.path.basename(file)
    directory = os.path.dirname(file)
    guard_name = guard_define_name_generator(filename, directory)
    
    file_source = open(file, "r").read()
    
    # what this roughly does:
    # looks for a line with whitespaces followed by #pragma once,
    # only whitespaces or comments may follow
    pragma_once_regex = re.compile(r"^[ \t]*#pragma once(?:[ \t]|(?:/\*.*\*/))*(?://.*)?\n", re.MULTILINE)
    
    if pragma_once_regex.search(file_source) is not None:
        # the file has #pragma once in it!
        print("[W] PRAGMA ONCE: '%s'" % (file))
        
    else:
        # we are likely to be dealing with normal header guards or no header guards at all        
        
        # what this roughly does:
        # looks for a line with whitespaces followed by #ifdef SOMETHING and COMMENTS OR NO COMMENTS
        # makes sure a line with whitespaces followed by #define SAME_SOMETHING and COMMENTS OR NO COMMENTS follows that line
        header_guard_regex = re.compile(r"^[ \t]*(?:#ifndef)[ \t]+([a-zA-Z0-9_]+)(?:[ \t]|(?:/\*.*\*/))*(?://.*)?[ \n\t]*#define[ \t]+\1(?:[ \t]|(?:/\*.*\*/))*(?://.*)?", re.MULTILINE)
        
        endif_regex = re.compile(r"^[ \t]*(?:#endif)(?:[ \t]|(?:/\*.*\*/))*(?://.*)?", re.MULTILINE)
        
        potential_header_guard = header_guard_regex.search(file_source)
        
        if potential_header_guard:
            # TODO: we need to check that only comments and whitespaces appear before this potential header guard
            print("Processed '%s' (header guard)" % (file))
            
            if not check_only:
                file_source = file_source[0:potential_header_guard.start()] + "#ifndef %s\n#define %s" % (guard_name, guard_name) + file_source[potential_header_guard.end():]
                
                match = None
                for match in re.finditer(endif_regex, file_source):
                    pass
                
                # if this assert fails, your C/C++ file is not valid! #ifdefs and #endifs likely do not match
                assert(match is not None)
                
                file_source = file_source[0:match.start()] + "#endif // %s" % (guard_name) + file_source[match.end():]
                
                open(file, "w").write(file_source)
        
        else:
            print("[W] NO MULTIPLE INCLUSION PROTECTION: '%s'" % (file))
    
def main():
    import argparse
            
    parser = argparse.ArgumentParser(description = "Clean up header guards of given files")
        
    parser.add_argument("--check", default = False, help = "If included, we only do checks, we don't fix any files.")
    parser.add_argument("globs", metavar = "F", type = str, nargs= "+", help = "File (or glob) to process.")
    
    args = parser.parse_args()

    import glob
    files = []
    for g in args.globs:
        gfiles = glob.glob(g)
        files.extend(gfiles)
        if len(gfiles) == 0:
            print("No file matches '%s'" % (g))
    
    for file in files:
        clean_include_guard(file, guard_define_name_generator = lambda filename, directory: default_guard_define_name_generator(filename, directory), check_only = args.check)
        
if __name__ == "__main__":
    main()