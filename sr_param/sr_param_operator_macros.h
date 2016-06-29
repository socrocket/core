// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup common
/// @{
/// @file sr_param_operator_macros.h
/// @date 2013-2014
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the 
///            authors is strictly prohibited.
/// @author Rolf Meyer
#ifndef SR_PARAM_OPERATOR_MACROS_H_
#define SR_PARAM_OPERATOR_MACROS_H_

// /////////////////////////////////////////////////////////////////// //
// This file defines macros for the template specializations of sr_param

// Macros:
// GC_SPECIALISATIONS_EQUAL_OPERATORS
// GC_SPECIALISATIONS_INCREMENT_OPERATORS
// GC_SPECIALISATIONS_ARITHMETIC_OPERATORS
// GC_SPECIALISATIONS_BINARY_OPERATORS


// Operator = with my_type
// Operator = with val_type
/*#define GC_SPECIALISATIONS_EQUAL_OPERATORS                            \
my_type& operator= (const my_type& v) {                               \
m_api->setParam( m_par_name,                                          \
const_cast<my_type&>(v).get() );                                      \
return *this;                                                         \
}                                                                     \
\
my_type& operator= (val_type v) { \
this->setValue(v); \
return *this; \
}
*/

// Operator -- prefix
// Operator -- postfix
#define GC_SPECIALISATIONS_DECREMENT_OPERATORS                       \
    inline my_type&                                                    \
    operator -- ()                                                     \
  {                                                                  \
  val_type tmp_val = getValue();                                   \
  --tmp_val;                                                       \
  this->setValue( tmp_val );                                       \
  return *this;                                                    \
  }                                                                  \
  \
  inline val_type                                                    \
  operator -- (int)                                                  \
  {                                                                  \
  val_type tmp_old_val = getValue();                               \
  val_type tmp_set_val = tmp_old_val;                              \
  tmp_set_val--;                                                   \
  this->setValue( tmp_set_val );                                   \
  return tmp_old_val;                                              \
  }                                                                  \

// Operator ++ prefix
// Operator ++ postfix
#define GC_SPECIALISATIONS_INCREMENT_OPERATORS                       \
    inline my_type&                                                    \
    operator ++ ()                                                     \
  {                                                                  \
  val_type tmp_val = getValue();                                   \
  ++tmp_val;                                                       \
  this->setValue( tmp_val );                                       \
  return *this;                                                    \
  }                                                                  \
  \
  inline val_type                                                    \
  operator ++ (int)                                                  \
  {                                                                  \
  val_type tmp_old_val = getValue();                               \
  val_type tmp_set_val = tmp_old_val;                              \
  tmp_set_val++;                                                   \
  this->setValue( tmp_set_val);                                    \
  return tmp_old_val;                                              \
  }

// Operator +=
// Operator -=
// Operator /=
// Operator *=
#define GC_SPECIALISATIONS_ARITHMETIC_OPERATORS_                         \
    inline my_type& operator += (val_type val)                            \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp += val;                                                         \
  this->setValue( tmp );                                              \
  return *this;                                                       \
  }                                                                     \
  \
  inline my_type&                                                       \
  operator -= (val_type val)                                            \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp -= val;                                                         \
  this->setValue( tmp );                                              \
  return *this;                                                       \
  }                                                                     \
  \
  inline my_type&                                                       \
  operator /= (val_type val)                                            \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp /= val;                                                         \
  this->setValue( tmp );                                              \
  return *this;                                                       \
  }                                                                     \
  \
  inline my_type&                                                       \
  operator *= (val_type val)                                            \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp *= val;                                                         \
  this->setValue( tmp );                                              \
  return *this;                                                       \
  }                                                                     \
  \
  inline val_type operator* (sr_param<val_type> &second)                \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp = tmp * second.getValue();                                      \
  return tmp;                                                         \
  }                                                                     \
  inline val_type operator- (sr_param<val_type> &second)                \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp = tmp - second.getValue();                                      \
  return tmp;                                                         \
  }                                                                     \
  inline val_type operator+ (sr_param<val_type> &second)                \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp = tmp + second.getValue();                                      \
  return tmp;                                                         \
  }                                                                     \
  inline val_type operator/ (sr_param<val_type> &second)                \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp = tmp / second.getValue();                                      \
  return tmp;                                                         \
  }                                                                     \


// Operator %=
// Operator ^=
// Operator |=
// Operator &=
// Operator <<=
// Operator >>=
#define GC_SPECIALISATIONS_BINARY_OPERATORS                             \
    inline my_type&                                                       \
    operator %= (val_type val)                                            \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp %= val;                                                         \
  this->setValue( tmp );                                              \
  return *this;                                                       \
  }                                                                     \
  \
  inline my_type&                                                       \
  operator ^= (val_type val)                                            \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp ^= val;                                                         \
  this->setValue( tmp );                                              \
  return *this;                                                       \
  }                                                                     \
  \
  inline my_type&                                                       \
  operator |= (val_type val)                                            \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp |= val;                                                         \
  this->setValue( tmp );                                              \
  return *this;                                                       \
  }                                                                     \
  \
  inline my_type&                                                       \
  operator &= (val_type val)                                            \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp &= val;                                                         \
  this->setValue( tmp );                                              \
  return *this;                                                       \
  }                                                                     \
  \
  inline my_type&                                                       \
  operator <<= (val_type val)                                           \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp <<= val;                                                        \
  this->setValue( tmp );                                              \
  return *this;                                                       \
  }                                                                     \
  \
  inline my_type&                                                       \
  operator >>= (val_type val)                                           \
  {                                                                     \
  val_type tmp = getValue();                                          \
  tmp >>= val;                                                        \
  this->setValue( tmp );                                              \
  return *this;                                                       \
  }

#endif
/// @}
