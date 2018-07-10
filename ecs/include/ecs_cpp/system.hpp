/**
 *
 * a ecs framework implement of cpp
 * file: system.hpp
 *
 * the system only has function, has no data
 * the detial logic is processed here
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-08-19
 */

#ifndef __ydk_ecs_system_hpp__
#define __ydk_ecs_system_hpp__

#include <unordered_map>
#include <memory>

namespace ecs_cpp{

typedef double time_delta; 
class system
{
public:
    typedef std::shared_ptr<system> ptr;
public:
    virtual ~system(){}

public:
    virtual void initialize() = 0;
    virtual void update(time_delta dt) = 0;
    virtual void fixed_update(time_delta dt) = 0;
};

class system_manager
{
protected:
    std::unordered_map<uint32_t, system::ptr> systems_list_;
public:
    system_manager(){
    }

    ~system_manager(){
    }

public:
    template<typename S>
    system_manager& add(std::shared_ptr<S> sys){
        uint32_t type_id = typeid(S).hash_code();
        systems_list_[type_id] = sys;
        return *this;
    }

    template<typename S, typename ...Args>
    system_manager& add(Args&& ... args){
        std::shared_ptr<S> s(new S(std::forward<Args>(args) ...));
        return add(s);
    }

    template<typename S>
    std::shared_ptr<S> system(){
        uint32_t type_id = typeid(S).hash_code();
        auto iter = systems_list_.find(type_id);
        if (iter == systems_list_.end()){
            return std::shared_ptr<S>();
        }
        return std::static_pointer_cast<S>(iter->second);
    }

    template<typename S>
    void initialize(){
        std::shared_ptr<S> sys = system<S>();
        if (sys){
            sys->initialize();
        }
    }

    template<typename S>
    void update(time_delta dt){
        std::shared_ptr<S> sys = system<S>();
        if (sys){
            sys->update(dt);
        }
    }

    template<typename S>
    void fixed_update(time_delta dt){
        std::shared_ptr<S> sys = system<S>();
        if (sys){
            sys->fixed_update(dt);
        }
    }

    void initialize(){
        for (auto& sys_pair : systems_list_){
            sys_pair.second->initialize();
        }
    }

    void update(time_delta dt) {
        for (auto& sys_pair : systems_list_){
            sys_pair.second->update(dt);
        }
    }

    void fixed_update(time_delta dt) {
        for (auto& sys_pair : systems_list_){
            sys_pair.second->fixed_update(dt);
        }
    }
};

}

#endif