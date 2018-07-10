/**
 *
 * a ecs framework implement of cpp
 * file: entity.hpp
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-08-19
 */

#ifndef __ydk_ecs_entity_hpp__
#define __ydk_ecs_entity_hpp__

#include <ecs_cpp/event.hpp>
#include <ecs_cpp/entity_manager_iface.hpp>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <typeindex>
#include <functional>
#include <unordered_map>

namespace ecs_cpp{
typedef std::vector<component_id> component_type_list;

class component;
class entity
{
public:
    friend class entity_manager;

protected:

    struct component_info_t{
        component* comp;
        int32_t    component_size;
    };

    int64_t                 entity_id_;
    std::unordered_map<component_id, component_info_t> components_map_;
    entity_manager_iface*   entity_mgr_;

    /* <entity, new added component>*/
    event_publisher<entity*, component*> component_added_event_publisher_;

    /* <entity, old component, new component> */
    event_publisher<entity*, component*, component*> component_replaced_event_publisher_;

    /* <entity, removed component */
    event_publisher<entity*, component*> component_removed_event_publisher_;

public:
    entity() = delete;
    entity(const entity& other) = delete;
    entity& operator = (const entity& other) = delete;

    entity(int64_t id, entity_manager_iface* mgr)
        : entity_id_(id)
        , entity_mgr_(mgr)
    {
    }

    virtual ~entity(){
        // remove all components
        while (!components_map_.empty()){
            component_id id = components_map_.begin()->first;
            remove_component(id);
        }
    }

public:
    bool operator == (const entity& other) const{
        return entity_id_ == other.entity_id_;
    }

    bool operator != (const entity& other) const {
        return !(*this == other);
    }

    bool operator < (const entity& other) const{
        return entity_id_ < other.entity_id_;
    }

    void destory(){
        if (entity_mgr_){
            entity_mgr_->destory(this);
        }
        else{
            delete this;
        }
    }

    int64_t id(){
        return entity_id_;
    }

    uint32_t component_count(){
        return components_map_.size();
    }

    /** sub/unsub component added event */
    void    subscribe_component_added_event(event_subscriber<entity*, component*>* sub, int32_t mode){
        if (mode == 1){
            component_added_event_publisher_.subscribe(sub);
        }
        else if (mode == 0){
            component_added_event_publisher_.unsubscribe(sub);
        }
    }

    /** sub/unsub component replace event */
    void    subscribe_component_replace_event(event_subscriber<entity*, component*, component*>* sub, int32_t mode){
        if (mode == 1){
            component_replaced_event_publisher_.subscribe(sub);
        }
        else if (mode == 0){
            component_replaced_event_publisher_.unsubscribe(sub);
        }
    }

    /** sub/unsub component remove event */
    void    subscribe_component_remove_event(event_subscriber<entity*, component*>* sub, int32_t mode){
        if (mode == 1){
            component_removed_event_publisher_.subscribe(sub);
        }
        else if (mode == 0){
            component_removed_event_publisher_.unsubscribe(sub);
        }
    }

    template<typename C, typename... Args>
    entity& add_component(Args&& ...args){
        if (has_component<C>()){
            throw std::runtime_error("add component to entity failed, the component already exists");
        }

        component* comp = add_component_no_check<C>(std::forward<Args>(args)...);

        // fire the component added event
        component_added_event_publisher_.publish_event(this, comp);

        return *this;
    }

    template<typename C>
    entity& remove_component(){
        uint32_t type_id = typeid(C).hash_code();
        remove_component(type_id);
        return *this;
    }

    template<typename C, typename... Args>
    entity& replace_component(Args&& ...args){
        if (has_component<C>()){
            replace_component_no_check<C>(std::forward<Args>(args)...);
        }
        else{
            add_component<C>(std::forward<Args>(args)...);
        }

        return *this;
    }

    template<typename C>
    bool    has_component(){
        uint32_t type_id = typeid(C).hash_code();
        return components_map_.find(type_id) != components_map_.end();
    }

    template<typename... Components>
    bool    has_components(){
        std::vector<std::function<bool()>> funs = { std::bind(&entity::has_component<Components>, this)... };
        for (auto& f : funs){
            if (!f()){
                return false;
            }
        }
        return true;
    }

    bool has_components(const component_type_list& list) const{
        for (auto type_id : list){
            if (components_map_.find(type_id) == components_map_.end()){
                return false;
            }
        }
        return true;
    }

    bool has_any_components(const component_type_list& list) const{
        for (auto type_id : list){
            if (components_map_.find(type_id) != components_map_.end()){
                return true;
            }
        }
        return false;
    }

    bool has_none_components(const component_type_list& list) const{
        for (auto type_id : list){
            if (components_map_.find(type_id) != components_map_.end()){
                return false;
            }
        }
        return true;
    }

    template<typename C>
    C*      get_component(){
        uint32_t type_id = typeid(C).hash_code();
        auto iter = components_map_.find(type_id);
        if (iter != components_map_.end()){
            return static_cast<C*>(iter->second.comp);
        }
        return nullptr;
    }

    template<typename... Components>
    std::tuple<Components* ...> get_components(){
        return std::make_tuple(get_component<Components>()...);
    }

protected:

    template<typename C, typename... Args>
    component* add_component_no_check(Args&& ...args){
        uint32_t type_id = typeid(C).hash_code();
        memory_pool_type* pool = check_or_create_component_pool<C>();
        component* comp = new (pool->allocate())C(std::forward<Args>(args)...);
        comp->set_entity(this);
        component_info_t& comp_info = components_map_[type_id];
        comp_info.comp = comp;
        comp_info.component_size = sizeof(C);
        return comp;
    }

    template<typename C, typename... Args>
    entity& replace_component_no_check(Args&& ...args){
        uint32_t type_id = typeid(C).hash_code();
        auto iter = components_map_.find(type_id);
        if (iter != components_map_.end()){
            component* old_component = iter->second.comp;
            component* new_component = add_component_no_check<C>(std::forward<Args>(args)...);

            // fire component replace event
            component_replaced_event_publisher_.publish_event(this, old_component, new_component);

            // delete the old component
            memory_pool_type* pool = check_or_create_component_pool<C>();
            pool->reclaim(old_component);
        }
        return *this;
    }

    void remove_component(component_id id){
        auto iter = components_map_.find(id);
        if (iter != components_map_.end()){
            component* comp = iter->second.comp;
            int32_t component_size = iter->second.component_size;

            components_map_.erase(iter);

            // fire the component remove event
            component_removed_event_publisher_.publish_event(this, comp);

            memory_pool_type* pool = get_component_pool(id);
            pool->reclaim(comp);
        }
    }

private:
    template<typename C>
    memory_pool_type* check_or_create_component_pool(){
        if (entity_mgr_){
            return entity_mgr_->check_or_create_component_pool<C>();
        }
        return nullptr;
    }

    memory_pool_type* get_component_pool(component_id id){
        if (entity_mgr_){
            return entity_mgr_->get_component_pool(id);
        }
        return nullptr;
    }
};

}

#endif