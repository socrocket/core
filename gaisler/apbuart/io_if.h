// vim : set fileencoding=utf-8 expandtab noai ts=4 sw=4 :
/// @addtogroup apbuart
/// @{
/// @file io_if.h
///
///
/// @date 2010-2015
/// @copyright All rights reserved.
///            Any reproduction, use, distribution or disclosure of this
///            program, without the express, prior written consent of the
///            authors is strictly prohibited.
/// @author Rolf Meyer
///

#ifndef MODELS_APBUART_IO_IF_H_
#define MODELS_APBUART_IO_IF_H_

#include <stdint.h>
#include "core/common/sr_registry.h"

#define \
  SR_HAS_UARTBACKEND_GENERATOR(type, factory, isinstance) \
  static SrModuleRegistry __sr_module_registry_##type##__("UARTBackend", #type, &factory, &isinstance, __FILE__); \
  volatile SrModuleRegistry *__sr_module_registry_##type = &__sr_module_registry_##type##__;

#define \
  SR_HAS_UARTBACKEND(type) \
    sc_core::sc_object *create_##type(sc_core::sc_module_name mn) { \
      return new type(mn); \
    } \
    bool isinstance_of_##type(sc_core::sc_object *obj) { \
      return dynamic_cast<type *>(obj) != NULL; \
    } \
    SR_HAS_UARTBACKEND_GENERATOR(type, create_##type, isinstance_of_##type);

class io_if {
  public:
    virtual uint32_t receivedChars() = 0;
    virtual void getReceivedChar(char *toRecv) = 0;
    virtual void sendChar(char toSend) = 0;
};

#endif  // MODELS_APBUART_IO_IF_H_
/// @}
