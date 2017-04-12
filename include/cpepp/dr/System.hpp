#ifndef CPEPP_DR_SYSTEM_H
#define CPEPP_DR_SYSTEM_H
#include "cpe/cfg/cfg_types.h"

namespace Cpe { namespace Dr {

class MetaLib;
class Meta;
class Entry;

class ConstDataElement;
class DataElement;
class ConstData;
class Data;

/*exceptions*/
class type_convert_error;

template<class T> struct MetaTraits;

}}

#endif

