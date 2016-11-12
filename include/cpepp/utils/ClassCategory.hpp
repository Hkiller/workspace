#ifndef CPEPP_UTILS_CLASSCATEGORY_H
#define CPEPP_UTILS_CLASSCATEGORY_H

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace Utils {

class Noncopyable {
    Noncopyable(Noncopyable const & o);
    Noncopyable & operator=(Noncopyable & o);

protected:
    Noncopyable() {}
};

class Nondestoryable {
    ~Nondestoryable();
};

class Noncreatable {
    Noncreatable();
};

class SimulateObject : public Noncreatable, public Nondestoryable, public Noncopyable {
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
