/**
 *
 * a ecs framework implement of cpp
 * file: event.hpp
 *
 * event subscribe/unsubscribe and event publish/dispatch implement
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-08-19
 */

#ifndef __ydk_ecs_event_hpp__
#define __ydk_ecs_event_hpp__

#include <vector>
#include <functional>

namespace ecs_cpp{

template<typename ...Args>
class event_subscriber
{
public:
    typedef std::function<void(Args ...)> event_function_type;
public:
    event_subscriber() 
        : event_func_(nullptr){
    }

    virtual ~event_subscriber(){
        if (event_func_){
            delete event_func_;
            event_func_ = nullptr;
        }
    }

public:

    void register_event_handler(const event_function_type& func){
        if (event_func_){
            *event_func_ = func;
        }
        else{
            event_func_ = new event_function_type(func);
        }
    }

    void event_dispath(Args ... args){
        if (event_func_){
            (*event_func_)(std::forward<Args>(args)...);
        }
    }

protected:
    event_function_type* event_func_;
};

template<typename ...Args>
class event_publisher
{
protected:
    std::vector<event_subscriber<Args...>*> subscribers_;

public:
    event_publisher(){}
    virtual ~event_publisher(){}

public:
    void subscribe(event_subscriber<Args...>* subscriber){
        auto iter = std::find(subscribers_.begin(), subscribers_.end(), subscriber);
        if (iter == subscribers_.end()){
            subscribers_.push_back(subscriber);
        }
    }

    void unsubscribe(event_subscriber<Args...>* subscriber){
        auto iter = std::find(subscribers_.begin(), subscribers_.end(), subscriber);
        if (iter != subscribers_.end()){
            subscribers_.erase(iter);
        }
    }

    void publish_event(Args... args){
        auto subscribers_tmp = subscribers_;
        for (auto& subscriber : subscribers_tmp){
            subscriber->event_dispath(std::forward<Args>(args)...);
        }
    }
};

class event_manager
{
public:
    event_manager(){}
    ~event_manager(){}
};

}

#endif