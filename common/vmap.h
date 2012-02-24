#ifndef VMAP_H
#define VMAP_H

#ifdef __GNUC__
#  ifdef __GNUC_MINOR__
#    if ((__GNUC__ >= 4 && __GNUC_MINOR__ >= 3) && !defined(MTI_SYSTEMC))
#      include <tr1/unordered_map>
#      define vmap std::tr1::unordered_map
#    else
#      include <ext/hash_map>
#      define  vmap __gnu_cxx::hash_map
#    endif
#  else
#    include <ext/hash_map>
#    define  vmap __gnu_cxx::hash_map
#  endif
#else
#  ifdef _WIN32
#    include <hash_map>
#      define  vmap stdext::hash_map
#  else
#    include <map>
#    define  vmap std::map
#  endif
#endif

#endif
