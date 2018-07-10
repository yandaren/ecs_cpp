/**
 *
 * a ecs framework implement of cpp
 * file: entity_manager_iface.hpp
 *
 * the interface of entify manager
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-08-23
 */

#ifndef __ydk_ecs_entity_manager_iface_hpp__
#define __ydk_ecs_entity_manager_iface_hpp__

#include <utility/pool/memory_pool.hpp>
#include <utility/sync/null_mutex.hpp>
#include <cstdint>
#include <unordered_map>

namespace ecs_cpp
{
class entity;
class component;
typedef uint32_t component_id;

typedef utility::memory_pool<utility::sync::null_mutex> memory_pool_type;

class entity_manager_iface
{
protected:
    std::unordered_map<component_id, memory_pool_type*> component_pool_map_;

public:
    entity_manager_iface(){}
    virtual ~entity_manager_iface(){
        for (auto& cp_pool_kv : component_pool_map_){
            delete cp_pool_kv.second;
        }
        component_pool_map_.clear();
    }

public:
    memory_pool_type* get_component_pool(component_id id){
        auto iter = component_pool_map_.find(id);
        if (iter != component_pool_map_.end()){
            return iter->second;
        }
        return nullptr;
    }

    template<typename C>
    memory_pool_type* check_or_create_component_pool(){
        component_id type_id = typeid(C).hash_code();
        memory_pool_type* pool = get_component_pool(type_id);
        if (!pool){
            pool = new utility::memory_pool_ex<C, utility::sync::null_mutex>(1);
            component_pool_map_[type_id] = pool;
        }
        return pool;
    }

public:
    /** 
     * @brief sub/unsubsribe the entity created event
     * @param sub - the subscriber 
     * @mode - 1, subscribe, 0 unsubscribe
     */
    virtual void subscribe_entity_create_event(event_subscriber<entity*>* sub, int32_t mode) = 0;

    /** 
     * @brief sub/unsubscribe the entify remove event
     */
    virtual void subscribe_entity_remove_event(event_subscriber<entity*>* sub, int32_t mode) = 0;

    /** 
     * @brief create the entity
     */
    virtual entity* create_entity() = 0;

    /** 
     * @brief destory the entity
     */
    virtual void destory(entity* en) = 0;
};
}

#endif