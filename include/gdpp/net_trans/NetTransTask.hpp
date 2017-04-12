#ifndef GDPP_NETTRANS_TASK_H
#define GDPP_NETTRANS_TASK_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "gd/net_trans/net_trans_task.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
# pragma warning(disable:4407)
#endif

namespace Gd { namespace NetTrans {

class NetTransTask : public Cpe::Utils::SimulateObject {
public:
    operator net_trans_task_t (void) const { return (net_trans_task_t)this; }

    uint32_t id(void) const { return net_trans_task_id(*this); }

    net_trans_errno_t err(void) const { return net_trans_task_errno(*this); }

    net_trans_task_state_t state(void) const { return net_trans_task_state(*this); }
    const char * state_str(void) const { return net_trans_task_state_str(net_trans_task_state(*this)); }

    net_trans_task_result_t result(void) const { return net_trans_task_result(*this); }
    const char * result_str(void) const { return net_trans_task_result_str(net_trans_task_result(*this)); }

    void * data(void) { return net_trans_task_data(*this); }
    void const  * data(void) const { return net_trans_task_data(*this); }
    size_t capacity(void) const { return net_trans_task_data_capacity(*this); }

    template <typename DataT>
    DataT & data(void) {
        check_capacity(Cpe::Dr::MetaTraits<DataT>::META);
        return *(DataT*)data();
    }

    template <typename DataT>
    DataT const & data(void) const {
        check_capacity(Cpe::Dr::MetaTraits<DataT>::META);
        return *(DataT*)data();
    }

    template<typename T> 
    void set_commit_to(T & r, void (T::*fun)(NetTransTask & task)) {
#ifdef _MSC_VER
        return this->set_commit_to(
            r, static_cast<NetTransProcessFun>(fun)
            , *((NetTransProcessor*)((void*)&r)));
#else
        return this->set_commit_to(
            static_cast<NetTransProcessor&>(r), static_cast<NetTransProcessFun>(fun));
#endif
    }

    /* VC编译器处理成员函数地址时有错误，没有生成垫片函数，所以为了正确调用函数指针，必须直接把传入T类型的对象绑定在调用的对象上
       所以传入的realResponser为真实的Responser地址，而useResponser是T的this地址，用于调用函数的
     */
	void set_commit_to(
        NetTransProcessor& realResponser, NetTransProcessFun fun
#ifdef _MSC_VER
        , NetTransProcessor& useResponser
#endif
        );

    template<typename T> 
    void set_on_progress(T & r, void (T::*fun)(NetTransTask & task, double dltotal, double dlnow)) {
#ifdef _MSC_VER
        return this->set_on_progress(
            r, static_cast<NetTransProgressFun>(fun)
            , *((NetTransProcessor*)((void*)&r)));
#else
        return this->set_on_progress(
            static_cast<NetTransProcessor&>(r), static_cast<NetTransProgressFun>(fun));
#endif
    }

    /* VC编译器处理成员函数地址时有错误，没有生成垫片函数，所以为了正确调用函数指针，必须直接把传入T类型的对象绑定在调用的对象上
       所以传入的realResponser为真实的Responser地址，而useResponser是T的this地址，用于调用函数的
     */
	void set_on_progress(
        NetTransProcessor& realResponser, NetTransProgressFun fun
#ifdef _MSC_VER
        , NetTransProcessor& useResponser
#endif
        );

    template<typename T> 
    void set_on_write(T & r, void (T::*fun)(NetTransTask & task, void * data, size_t data_size)) {
#ifdef _MSC_VER
        return this->set_on_write(
            r, static_cast<NetTransWriteFun>(fun)
            , *((NetTransProcessor*)((void*)&r)));
#else
        return this->set_on_write(
            static_cast<NetTransProcessor&>(r), static_cast<NetTransWriteFun>(fun));
#endif
    }

    /* VC编译器处理成员函数地址时有错误，没有生成垫片函数，所以为了正确调用函数指针，必须直接把传入T类型的对象绑定在调用的对象上
       所以传入的realResponser为真实的Responser地址，而useResponser是T的this地址，用于调用函数的
     */
	void set_on_write(
        NetTransProcessor& realResponser, NetTransWriteFun fun
#ifdef _MSC_VER
        , NetTransProcessor& useResponser
#endif
        );
    
    template<typename DataT> 
    void set_post_to(const char * uri, DataT const & data) {
        set_post_to(uri, &data, Cpe::Dr::MetaTraits<DataT>::data_size(data));
    }

    void set_debug(bool b) { net_trans_task_set_debug(*this, b ? 1 : 0); }
    void set_post_to(const char * uri, const char * data, size_t data_len);
    void set_get(const char * uri);
    void set_skip_data(ssize_t skip_size);
    void set_timeout(uint64_t timeout_ms);
    
    void start(void);

    const char * buffer_to_string(void);
    mem_buffer_t buffer(void) { return net_trans_task_buffer(*this); }

private:
    void check_capacity(LPDRMETA meta) const;
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
