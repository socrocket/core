// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file socrocket.h
/// Central header file for global constants and interface typ definitions.
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef COMMON_SOCROCKET_H_
#define COMMON_SOCROCKET_H_

// Snooping information (AHBCTRL -> MMU_CACHE)
typedef struct {
  unsigned int master_id;
  unsigned int address;
  unsigned int length;
} t_snoop;

#endif  // COMMON_SOCROCKET_H_
/// @}
