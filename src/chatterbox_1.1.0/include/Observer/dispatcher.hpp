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
    typedef std::function<void(Args...)> CallbackType;

    class CallbackID
    {
    public:
        CallbackID() : valid(false) {}
    private:
        friend class Dispatcher<Args...>;
        CallbackID(typename std::list<CallbackType>::iterator i)
        : iter(i), valid(true)
        {}

        typename std::list<CallbackType>::iterator iter;
        bool valid;
    };

    // register to be notified
    CallbackID addCallback(CallbackType callback)
    {
        if (callback)
        {
            callbacks.push_back(callback);
            return CallbackID(--callbacks.end());
        }
        return CallbackID();
    }

    // unregister to be notified
    void delCallback(CallbackID &id)
    {
        if (id.valid)
        {
            callbacks.erase(id.iter);
        }
    }

    void broadcast(Args... args)
    {
        for (auto &callback : callbacks)
        {
            callback(args...);
        }
    }

private:
    std::list<CallbackType> callbacks;
};

#endif