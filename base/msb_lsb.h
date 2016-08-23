// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file msb_lsb.h
///
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Thomas Schuster
///

#ifndef COMMON_MSB_LSB_H_
#define COMMON_MSB_LSB_H_
template<uint32_t w, class T>
T msb_lsb(T a) {
  int h = ((w & 1) ? (w - 1) : w) / 2;
  T r = (w & 1) ? (a & (1 << h + 1)) : 0;
  for (int i = 0; i <= h; ++i) {
    bool b1 = a & (1 << i), b2 = a & (1 << (w - i));
    r |= (b2 << i) | (b1 << (w - i));
  }
  return r;
}

#endif  // COMMON_MSB_LSB_H_
/// @}
