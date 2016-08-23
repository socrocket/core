// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file vmap.h
///
/// @details To save system memory and optimize simulation performance large, 
/// sparse memories should be implemented as maps. In this case the memory 
/// address represents the key and the actual data the entry. Because the 
/// performance of the various existing map implementation strongly depends 
/// on the system environment, the SoCRocket library provides the flexible 
/// type vmap. The vmap.h header contains a macro defining vmap as either 
/// std::map, hash_map or std::tr1::unordered_map.
///
/// An example for the usage of vmap is given by the MapMemory implementation 
/// of the Generic Memory (@ref memory_p "Generic Memory SystemC Model").
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef COMMON_VMAP_H_
#define COMMON_VMAP_H_

#ifndef MTI_SYSTEMC
  #ifdef __GNUC__
    #if ((__GNUC__ * 100 + __GNUC_MINOR__) >= 403)
      #include <tr1/unordered_map>
      #define vmap std::tr1::unordered_map
    #else
      #include <ext/hash_map>
      #define  vmap __gnu_cxx::hash_map
    #endif
  #else  // ifdef __GNUC__
    #ifdef _WIN32
      #include <hash_map>
      #define  vmap stdext::hash_map
    #else
      #include <map>
      #define  vmap std::map
    #endif
  #endif
#else
  #include <map>
  #define  vmap std::map
#endif

#endif  // COMMON_VMAP_H_
/// @}
