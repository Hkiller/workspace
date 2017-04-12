#ifndef GDPP_APP_ATTR_FORMULAR_IT_H
#define GDPP_APP_ATTR_FORMULAR_IT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gd/app_attr/app_attr_formula.h"
#include "System.hpp"

namespace Gd { namespace AppAttr {

class AppAttrFormulaIt : public Cpe::Utils::Noncopyable {
public:
    explicit AppAttrFormulaIt(app_attr_request_t request) {
        app_attr_formulas_in_request(request, &m_it);
    }

    AppAttrFormula * next(void) {
        app_attr_formula_t formula = app_attr_formula_it_next(&m_it);
        return formula ? (AppAttrFormula*)formula : NULL;
    }
    
private:
    app_attr_formula_it m_it;
};

}}

#endif
