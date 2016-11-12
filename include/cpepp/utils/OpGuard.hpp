#ifndef CPEPP_UTILS_OPGUARD_H
#define CPEPP_UTILS_OPGUARD_H
#include <vector>
#include <memory>
#include "ClassCategory.hpp"

namespace Cpe { namespace Utils {

class OpGuard : public Noncopyable {
public:
    class OpGuardNode {
    public:
        virtual void execute(void) = 0;
        virtual ~OpGuardNode() = 0;
    };

    OpGuard();
    ~OpGuard();

    template<typename R>
    class OpGuardNode_Fun : public OpGuardNode {
    public:
        OpGuardNode_Fun(R (*fun)(void))
            : m_fun(fun)
        {
        }

        virtual void execute(void) {
            (*m_fun)();
        }

        R (*m_fun)(void);
    };
    
    template<typename R>
    void addOp(R (*fun)(void)) {
        this->addOpNode(
            ::std::auto_ptr<OpGuardNode>(
                new OpGuardNode_Fun<R>(fun)));
    }

    template<typename ArgT, typename R>
    class OpGuardNode_Fun_1 : public OpGuardNode {
    public:
        OpGuardNode_Fun_1(R (*fun)(ArgT arg), ArgT arg)
            : m_fun(fun)
            , m_arg(arg)
        {
        }

        virtual void execute(void) {
            (*m_fun)(m_arg);
        }

        R (*m_fun)(ArgT arg);
        ArgT m_arg;
    };
    
    template<typename ArgT1, typename R>
    void addOp(R (*fun)(ArgT1), ArgT1 arg1) {
        this->addOpNode(
            ::std::auto_ptr<OpGuardNode>(
                new OpGuardNode_Fun_1<ArgT1, R>(fun, arg1)));
    }

    template<typename T, typename R>
    class OpGuardNode_MemFun : public OpGuardNode {
    public:
        OpGuardNode_MemFun(T & o, R (T::*fun)(void))
            : m_o(o)
            , m_fun(fun)
        {
        }

        virtual void execute(void) {
            (m_o.*m_fun)();
        }

        T & m_o;
        R (T::*m_fun)(void);
    };

    template<typename T, typename R>
    void addOp(T & o, R (T::*fun)(void)) {
        this->addOpNode(
            ::std::auto_ptr<OpGuardNode>(
                new OpGuardNode_MemFun<T, R>(o, fun)));
    }

    template<typename T, typename ArgT1, typename R>
    class OpGuardNode_MemFun_1 : public OpGuardNode {
    public:
        OpGuardNode_MemFun_1(T & o, R (T::*fun)(ArgT1 arg), ArgT1 arg1)
            : m_o(o)
            , m_fun(fun)
            , m_arg1(arg1) 
        {
        }

        virtual void execute(void) {
            (m_o.*m_fun)(m_arg1);
        }

        T & m_o;
        R (T::*m_fun)(void);
        ArgT1 m_arg1;
    };

    template<typename T, typename ArgT1, typename R>
    void addOp(T & o, R (T::*fun)(ArgT1), ArgT1 arg1) {
        this->addOpNode(
            ::std::auto_ptr<OpGuardNode>(
                new OpGuardNode_MemFun_1<T, ArgT1, R>(o, fun, arg1)));
    }

    void releaseControl(void);

private:
    void addOpNode(::std::auto_ptr<OpGuardNode> node);

    bool m_needExecute;
    ::std::vector<OpGuardNode*> m_ops;
};

}}

#endif
