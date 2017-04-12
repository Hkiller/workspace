#ifndef UIPP_APP_UIENTER_EXT_H
#define UIPP_APP_UIENTER_EXT_H
#include "uipp/app/UICenter.hpp"

namespace UI { namespace App {

class EnvExt;
class UICenterExt : public UICenter {
public:
    static ::std::auto_ptr<UICenterExt> create(EnvExt & env, Cpe::Cfg::Node const & config);
};

}}

#endif
