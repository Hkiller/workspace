#ifndef UIPP_APP_ACTION_BINDVALUE_H
#define UIPP_APP_ACTION_BINDVALUE_H
#include <list>
#include "UIActionGen_WithControl.hpp"

namespace UI { namespace App {

class UIAction_BindValue : public UIActionGen_WithControl<UIAction_BindValue> {
	struct Value_Data {
        ::std::string name;
        ::std::string value;
	};

public:
    UIAction_BindValue(Sprite::Fsm::Action & action);
    UIAction_BindValue(Sprite::Fsm::Action & action, UIAction_BindValue const & o);
	~UIAction_BindValue();

    int enter(void);
    void exit(void);

	void addValue(const char * name, const char * value);

    static const char * NAME;
    static void install(Sprite::Fsm::Repository & repo);

private:
    void onValueUpdated(void);
    void updateAttrs(void);

    ::std::list<Value_Data> m_values;
};

}}

#endif
