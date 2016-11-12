#ifndef GDPP_NETTRANS_TASK_BUILDER_H
#define GDPP_NETTRANS_TASK_BUILDER_H
#include <cassert>
#include <stdio.h>
#include "NetTransTask.hpp"

namespace Gd { namespace NetTrans {

struct NetTransTaskBuilderRef {
    explicit NetTransTaskBuilderRef(net_trans_task_t task) : m_task(task) {}
    net_trans_task_t m_task;
};

class NetTransTaskBuilder {
public:
    explicit NetTransTaskBuilder(net_trans_task_t task = NULL)
        : m_task(task)
    {
    }
    
    NetTransTaskBuilder(NetTransTaskBuilderRef task_ref)
        : m_task(task_ref.m_task)
    {
    }
    
    NetTransTaskBuilder(NetTransTaskBuilder & o)
        : m_task(o.m_task)
    {
        o.m_task = NULL;
    }
    
    ~NetTransTaskBuilder() {
        if (m_task) {
            net_trans_task_free(m_task);
            m_task = NULL;
        }
    }

    NetTransTaskBuilder & operator=(NetTransTaskBuilderRef o) {
        if (m_task) net_trans_task_free(m_task);
        m_task = o.m_task;
        return *this;
    }
    
    NetTransTaskBuilder & operator=(NetTransTaskBuilder & o) {
        if (m_task) net_trans_task_free(m_task);
        m_task = o.m_task;
        o.m_task = NULL;
        return *this;
    }

    NetTransTaskBuilder & debug(bool b) {
        assert(m_task);
        ((NetTransTask*)m_task)->set_debug(b);
        return *this;
    }
    
    template<typename T> 
    NetTransTaskBuilder & commit_to(T & r, void (T::*fun)(NetTransTask & task)) {
        assert(m_task);
        ((NetTransTask*)m_task)->set_commit_to<T>(r, fun);
        return *this;
    }

    template<typename T> 
    NetTransTaskBuilder & on_progress(T & r, void (T::*fun)(NetTransTask & task, double dltotal, double dlnow)) {
        assert(m_task);
        ((NetTransTask*)m_task)->set_on_progress<T>(r, fun);
        return *this;
    }

    template<typename T> 
    NetTransTaskBuilder & on_write(T & r, void (T::*fun)(NetTransTask & task, void * data, size_t data_size)) {
        assert(m_task);
        ((NetTransTask*)m_task)->set_on_write<T>(r, fun);
        return *this;
    }
    
    template<typename DataT> 
    NetTransTaskBuilder & post_to(const char * uri, DataT const & data) {
        assert(m_task);
        ((NetTransTask*)m_task)->set_post_to<DataT>(uri, data);
        return *this;
    }
    
    NetTransTaskBuilder & post_to(const char * uri, const char * data, int data_len) {
        assert(m_task);
        ((NetTransTask*)m_task)->set_post_to(uri, data, data_len);
        return *this;
    }

    NetTransTaskBuilder & get(const char * uri) {
        assert(m_task);
        ((NetTransTask*)m_task)->set_get(uri);
        return *this;
    }

    NetTransTaskBuilder & skip_data(ssize_t skip_size) {
        assert(m_task);
        ((NetTransTask*)m_task)->set_skip_data(skip_size);
        return *this;
    }

    NetTransTaskBuilder & timeout(uint64_t timeout_ms) {
        assert(m_task);
        ((NetTransTask*)m_task)->set_timeout(timeout_ms);
        return *this;
    }

    NetTransTask & start(void) {
        assert(m_task);
        NetTransTask & t = *(NetTransTask*)m_task;
        t.start();
        m_task = NULL;
        return t;
    }
    
    operator NetTransTaskBuilderRef () {
        NetTransTaskBuilderRef r(m_task);
        m_task = NULL;
        return r;
    }
    
private:
    net_trans_task_t m_task;
};

}}

#endif
