#ifndef UIPP_SPRITE_COMPONENT_WITH_DATA_GEN_H
#define UIPP_SPRITE_COMPONENT_WITH_DATA_GEN_H
#include "cpe/pal/pal_strings.h"
#include "ComponentGen.hpp"

namespace UI { namespace Sprite {

template<typename BaseT, typename OuterT, typename DataT> 
class ComponentWithDataGen : public ComponentGen<BaseT, OuterT> {
public:
    typedef ComponentWithDataGen ComponentBase;
    typedef DataT ComponentDataType;

    ComponentWithDataGen(UI::Sprite::Component & component)
        : ComponentGen<BaseT, OuterT>(component)
    {
        bzero(&m_data, sizeof(m_data));
    }

    ComponentWithDataGen(UI::Sprite::Component & component, ComponentWithDataGen const & o)
        : ComponentGen<BaseT, OuterT>(component)
        , m_data(o.m_data)
    {
    }

    DataT & attrData(void) { return m_data; }
    DataT const & attrData(void) const { return m_data; }

    template<typename T>
    void setAttrData(T const & data) { this->m_component.setAttrData(data); }

    static uint16_t data_start(void) { return ((const char *)&(((ComponentWithDataGen const *)((OuterT const *)1000))->m_data)) - (const char *)(1000); } ;

private:
    DataT m_data;
};

}}

#endif
