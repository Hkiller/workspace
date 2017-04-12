#ifndef GDPP_NM_TEST_MANAGERTEST_H
#define GDPP_NM_TEST_MANAGERTEST_H
#include "cpe/nm/tests-env/with_nm.hpp"
#include "cpe/utils/tests-env/test-fixture.hpp"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpepp/nm/Object.hpp"
#include "cpepp/nm/Group.hpp"
#include "cpepp/nm/Manager.hpp"

typedef LOKI_TYPELIST_1(gd::nm::testenv::with_nm) NmTestBase;

class NmTest : public testenv::fixture<NmTestBase> {
public:
    NmTest();

    class TestObject : public Cpe::Nm::Object {
    public:
        TestObject(NmTest & t, int v) : _t(t), _value(v) {
        }

        virtual ~TestObject() {
            ++_t._destoryCount;
        }

        NmTest & _t;
        int _value;
    };

    class TestGroup : public Cpe::Nm::Group {
    public:
        TestGroup(NmTest & t, int v) : _t(t), _value(v) {
        }

        virtual ~TestGroup() {
            ++_t._destoryCount;
        }

        NmTest & _t;
        int _value;
    };

    Cpe::Nm::Manager & mgr(void) { return *((Cpe::Nm::Manager*)t_nm()); }
    Cpe::Nm::Manager const & mgr_const(void) { return mgr(); }

    int _destoryCount;
};

#endif
