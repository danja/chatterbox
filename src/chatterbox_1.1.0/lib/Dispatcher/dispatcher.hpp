/*
based on https://www.theimpossiblecode.com/blog/c11-generic-observer-pattern/
*/

#ifndef __DISPATCHER_H
#define __DISPATCHER_H

#include <functional>
#include <list>

template <typename... Args>
class Dispatcher
{
public:
    typedef std::function<void(Args...)> CBType;

    class CBID
    {
    public:
        CBID() : valid(false) {}
    private:
        friend class Dispatcher<Args...>;
        CBID(typename std::list<CBType>::iterator i)
        : iter(i), valid(true)
        {}

        typename std::list<CBType>::iterator iter;
        bool valid;
    };

    // register to be notified
    CBID addCB(CBType cb)
    {
        if (cb)
        {
            cbs.push_back(cb);
            return CBID(--cbs.end());
        }
        return CBID();
    }

    // unregister to be notified
    void delCB(CBID &id)
    {
        if (id.valid)
        {
            cbs.erase(id.iter);
        }
    }

    void broadcast(Args... args)
    {
        for (auto &cb : cbs)
        {
            cb(args...);
        }
    }

private:
    std::list<CBType> cbs;
};

#endif