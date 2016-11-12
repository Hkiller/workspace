#ifndef CPE_UTILS_TEST_EXTERN_H
#define CPE_UTILS_TEST_EXTERN_H
#include "test-fixture.hpp"

namespace testenv {

template <typename TList> struct env_gen;

template <class Head, class Tail>
struct env_gen<Loki::Typelist<Head, Tail> >
    : public Head
    , public env_gen<Tail>
{
    void SetUp() {
        Head::SetUp();
        env_gen<Tail>::SetUp();
    }

    void TearDown() {
        env_gen<Tail>::TearDown();
        Head::TearDown();
    }
};

template <class Head>
struct env_gen<Loki::Typelist<Head, Loki::NullType> >
    : public Head
{
    void SetUp() {
        Head::SetUp();
    }

    void TearDown() {
        Head::TearDown();
    }
};

template <>
struct env_gen<Loki::NullType> {
    void SetUp() {}
    void TearDown() {}
};

template<typename EnvListT = Loki::NullType>
class env : public env_gen<EnvListT> {
public:
    typedef env_gen<EnvListT> Base;

    virtual ~env() {}

    template<typename T2>
    T2 * tryEnvOf(void) { return dynamic_cast<T2 *>(this); }

    template<typename T2>
    T2 & envOf(void) { return dynamic_cast<T2 &>(*this); }

    mem_allocrator_t t_tmp_allocrator() { return envOf<Test>().t_tmp_allocrator(); }
    void * t_tmp_alloc(size_t size) { return envOf<Test>().t_tmp_alloc(size); }
    char * t_tmp_strdup(const char * str) { return envOf<Test>().t_tmp_strdup(str); }
    void * t_tmp_memdup(void const * buf, size_t size) { return envOf<Test>().t_tmp_memdup(buf, size); }
    char * t_tmp_hexdup(void const * buf, size_t size) { return envOf<Test>().t_tmp_hexdup(buf, size); }

    mem_allocrator_t t_allocrator() { return envOf<Test>().t_allocrator(); }
    void * t_alloc(size_t size) { return envOf<Test>().t_alloc(size); }
    char * t_strdup(const char * str) { return envOf<Test>().t_strdup(str); }
};

}

#endif
