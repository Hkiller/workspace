#ifndef UIPP_APP_ACTION_SETVALUE_H
#define UIPP_APP_ACTION_SETVALUE_H
#include <list>
#include "cpepp/utils/ObjRef.hpp"
#include "UIActionGen_WithControl.hpp"

namespace UI { namespace App {

class UIAction_SetValue : public UIActionGen_WithControl<UIAction_SetValue> {

	struct Value_Data {
        ::std::string name;
        ::std::string value;
	};

public:
    UIAction_SetValue(Sprite::Fsm::Action & action);
    UIAction_SetValue(Sprite::Fsm::Action & action, UIAction_SetValue const & o);
	~UIAction_SetValue();

    int enter(void);
    void exit(void);

	void addValue(const char * name, const char * value);

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
    ::std::list<Value_Data> m_values;
};

}}

#endif
