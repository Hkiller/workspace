#ifndef GDPP_APP_ATTR_FORMULAR_H
#define GDPP_APP_ATTR_FORMULAR_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/Utils.hpp"
#include "gd/app_attr/app_attr_formula.h"
#include "System.hpp"

namespace Gd { namespace AppAttr {

class AppAttrFormula : public Cpe::Utils::SimulateObject {
public:
    operator app_attr_formula_t (void) const { return (app_attr_formula_t)this; }

    const char * name(void) const { return app_attr_formula_name(*this); }
    
    AppAttrFormula & calc(dr_data_source_t addition_arg);

    template<typename T1>
    AppAttrFormula & calc(T1 const & d1) {
        dr_data_source ds1;
        ds1.m_data.m_data = (void*)&d1;
        ds1.m_data.m_size = Cpe::Dr::data_size(d1);
        ds1.m_data.m_meta = Cpe::Dr::metaOf(d1);
        ds1.m_next = NULL;
        return calc(&ds1);
    }

    template<typename T1, typename T2>
    AppAttrFormula & calc(T1 const & d1, T2 const & d2) {
        dr_data_source ds2;
        ds2.m_data.m_data = (void*)&d2;
        ds2.m_data.m_size = Cpe::Dr::data_size(d2);
        ds2.m_data.m_meta = Cpe::Dr::metaOf(d2);
        ds2.m_next = NULL;
        
        dr_data_source ds1;
        ds1.m_data.m_data = (void*)&d1;
        ds1.m_data.m_size = Cpe::Dr::data_size(d1);
        ds1.m_data.m_meta = Cpe::Dr::metaOf(d1);
        ds1.m_next = &ds2;

        return calc(&ds1);
    }
    
    int8_t asInt8(void);
    int8_t asInt8(int8_t dft) { return app_attr_formula_to_int8(*this, dft); }
    uint8_t asUInt8(void);
    uint8_t asUInt8(uint8_t dft) { return app_attr_formula_to_uint8(*this, dft); }
    int16_t asInt16(void);
    int16_t asInt16(int16_t dft) { return app_attr_formula_to_int16(*this, dft); }
    uint16_t asUInt16(void);
    uint16_t asUInt16(uint16_t dft) { return app_attr_formula_to_uint16(*this, dft); }
    int32_t asInt32(void);
    int32_t asInt32(int32_t dft) { return app_attr_formula_to_int32(*this, dft); }
    uint32_t asUInt32(void);
    uint32_t asUInt32(uint32_t dft) { return app_attr_formula_to_int32(*this, dft); }
    int64_t asInt64(void);
    int64_t asInt64(int64_t dft) { return app_attr_formula_to_int64(*this, dft); }
    uint64_t asUInt64(void);
    uint64_t asUInt64(uint64_t dft) { return app_attr_formula_to_uint64(*this, dft); }
    float asFloat(void);
    float asFloat(float dft) { return app_attr_formula_to_float(*this, dft); }
    double asDouble(void);
    double asDouble(double dft) { return app_attr_formula_to_double(*this, dft); }
    const char * asString(mem_buffer_t buffer);
    const char * asString(mem_buffer_t buffer, const char * dft) { return app_attr_formula_to_string(*this, buffer, dft); }

    static AppAttrFormula & _cast(app_attr_formula_t formula);
};

}}

#endif
