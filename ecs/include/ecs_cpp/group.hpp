/**
 *
 * a ecs framework implement of cpp
 * file: group.hpp
 * 
 * a entity group that has specific tuple component
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-08-20
 */

#ifndef __ydk_ecs_group_hpp__
#define __ydk_ecs_group_hpp__

#include <ecs_cpp/matcher.hpp>
#include <ecs_cpp/entity.hpp>
#include <ecs_cpp/event.hpp>
#include <ecs_cpp/entity_manager_iface.hpp>
#include <unordered_set>

namespace ecs_cpp
{
class group
{
public:
    typedef std::shared_ptr<group> ptr;
protected:
    matcher::ptr                mather_;
    std::unordered_set<entity*> entitis_;
    entity_manager_iface*       entity_mgr_;

    /** <entity, new added component*> */
    event_subscriber<entity*, component*> component_added_event_subscriber_;

    /** <entity, old component, new component*> */
    event_subscriber<entity*, component*, component*> component_replace_event_subsciber_;

    /** <entity, removed component*> */
    event_subscriber<entity*, component*> component_removed_subscriber_;

    /** <entity> */
    event_subscriber<entity*> entity_added_subscriber_;

    /** <entity> */
    event_subscriber<entity*> entity_removed_subscriber_;

public:
    static group::ptr create(matcher::ptr mther, entity_manager_iface* entity_mgr){
        return std::make_shared<group>(mther, entity_mgr);
    }

public:
    group(matcher::ptr mather, entity_manager_iface* entity_mgr)
        : mather_(mather)
        , entity_mgr_(entity_mgr)
    {
        initailize_event_subscriber();

        // subscribe entity create/remove events
        subscriber_entity_events(1);
    }

    ~group(){

        // unsubscribe all the entitys of the group
        while (!entitis_.empty()){
            entity* en = *entitis_.begin();
            remove_entity(en);
        }

        // unsubscribe entity create/remove events
        subscriber_entity_events(0);
    }

    uint32_t entity_count(){
        return entitis_.size();
    }

    const std::unordered_set<entity*>& entities(){
        return entitis_;
    }

    matcher::ptr matcher(){
        return mather_;
    }

    void handle_entity_match(entity* en){
        if (!mather_ || !en ){
            return;
        }

        if (mather_->matches(en)){
            add_entity(en);
        }
        else{
            remove_entity(en);
        }
    }

protected:
    void    add_entity(entity* en){
        entitis_.insert(en);

        // subscribe the entity events
        subscribe_entity_component_events(en, 1);
    }

    void    remove_entity(entity* en){
        entitis_.erase(en);

        // unsubscribe the entity envents
        subscribe_entity_component_events(en, 0);
    }

    void    subscribe_entity_component_events(entity* en, int32_t mode){
        en->subscribe_component_added_event(&component_added_event_subscriber_, mode);
        en->subscribe_component_replace_event(&component_replace_event_subsciber_, mode);
        en->subscribe_component_remove_event(&component_removed_subscriber_, mode);
    }

    void    subscriber_entity_events(int32_t mode){
        if (entity_mgr_){
            entity_mgr_->subscribe_entity_create_event(&entity_added_subscriber_, mode);
            entity_mgr_->subscribe_entity_remove_event(&entity_removed_subscriber_, mode);
        }
    }

    void    initailize_event_subscriber(){
        // entity component events
        component_added_event_subscriber_.register_event_handler(
            std::bind(&group::event_entity_component_added, this, std::placeholders::_1, std::placeholders::_2));
        component_replace_event_subsciber_.register_event_handler(
            std::bind(&group::event_entity_component_replaced, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        component_removed_subscriber_.register_event_handler(
            std::bind(&group::event_entity_component_removed, this, std::placeholders::_1, std::placeholders::_2));

        // entity events
        entity_added_subscriber_.register_event_handler(
            std::bind(&group::event_entity_added, this, std::placeholders::_1));
        entity_removed_subscriber_.register_event_handler(
            std::bind(&group::event_entity_removed, this, std::placeholders::_1));
    }

    void    event_entity_component_added(entity* en, component* added_comp){
        handle_entity_match(en);
    }

    void    event_entity_component_replaced(entity* en, component* old_comp, component* new_comp){
        handle_entity_match(en);
    }

    void    event_entity_component_removed(entity* en, component* removed_comp){
        handle_entity_match(en);
    }

    void    event_entity_added(entity* en){
        handle_entity_match(en);
    }

    void    event_entity_removed(entity* en){
        remove_entity(en);
    }
};
}

#endif