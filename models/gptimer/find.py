import os
import os.path
import fnmatch


def getdirs(base = '.', excludes = []):
    result = []
    for root, dirs, files in os.walk(base):
        if any([fnmatch.fnmatchcase(root, exc) for exc in excludes]):
            continue
        result.append(root)
    return result

def getfiles(base, ext, excludes):
    result = []
    for root, dirs, files in os.walk(base):
        for cdir in dirs:
            if any([fnmatch.fnmatchcase(os.path.join(root, cdir), exc) for exc in excludes]):
                del(cdir)
        
        for cfile in files:
            if any([fnmatch.fnmatchcase(os.path.join(root, cfile), exc) for exc in excludes]):
                continue
            if any([fnmatch.fnmatchcase(os.path.join(root, cfile), e) for e in ext]): 
                result.append(os.path.join(root, cfile))
            
    return result

if __name__ == "__main__":
    for dirs in getdirs('/home/hwswsim/tools/greensocs-4.0.0/greenreg', ['*test*','*example*']):
        print dirs

    for cxx in getfiles('/home/hwswsim/tools/greensocs-4.0.0/greenreg', ['*.cpp'], ['*test*', '*example*']):
        print cxx
