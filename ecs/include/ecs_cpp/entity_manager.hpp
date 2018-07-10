/**
 *
 * a ecs framework implement of cpp
 * file: entity_manager.hpp
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-08-22
 */

#ifndef __ydk_ecs_entity_manager_hpp__
#define __ydk_ecs_entity_manager_hpp__

#include <ecs_cpp/entity.hpp>
#include <ecs_cpp/group.hpp>
#include <ecs_cpp/matcher.hpp>
#include <ecs_cpp/entity_manager_iface.hpp>

namespace ecs_cpp
{
class entity_manager : public entity_manager_iface
{
protected:
    std::unordered_map<int64_t, entity*> entity_map_;
    int64_t                              next_entity_id_;

    typedef uint32_t matcher_hash_type;
    typedef std::unordered_map<matcher_hash_type, group*> mather_group_map_type;
    mather_group_map_type                mather_group_map_;

    /** event publisher */
    event_publisher<entity*>             entity_create_event_publisher_;
    event_publisher<entity*>             entity_remove_event_publisher_;

    /** entity pool */
    utility::memory_pool_ex<entity, utility::sync::null_mutex> entity_memory_pool_;
public:
    /** implements of the interface from entity_manager_iface */
    /**
    * @brief sub/unsubsribe the entity created event
    * @param sub - the subscriber
    * @mode - 1, subscribe, 0 unsubscribe
    */
    virtual void subscribe_entity_create_event(event_subscriber<entity*>* sub, int32_t mode) override{
        if (mode == 1){
            entity_create_event_publisher_.subscribe(sub);
        }
        else if (mode == 0){
            entity_create_event_publisher_.unsubscribe(sub);
        }
    }

    /**
    * @brief sub/unsubscribe the entify remove event
    */
    virtual void subscribe_entity_remove_event(event_subscriber<entity*>* sub, int32_t mode) override{
        if (mode == 1){
            entity_remove_event_publisher_.subscribe(sub);
        }
        else if (mode == 0){
            entity_remove_event_publisher_.unsubscribe(sub);
        }
    }

    /**
    * @brief create the entity
    */
    virtual entity* create_entity() override {
        entity* en = new (entity_memory_pool_.allocate()) entity(generate_next_entity_id(), this);
        entity_map_[en->id()] = en;

        // fire entity create event 
        entity_create_event_publisher_.publish_event(en);

        return en;
    }

    /**
    * @brief destory the entity
    */
    virtual void destory(entity* en) override{
        if (!en)
            return;

        // fire the en destory event
        entity_remove_event_publisher_.publish_event(en);

        // remove from the map
        entity_map_.erase(en->id());

        // return to the pool
        entity_memory_pool_.reclaim(en);
    }

public:
    entity_manager() : next_entity_id_(0), entity_memory_pool_(1){}
    virtual ~entity_manager(){
        for (auto& gp_kv : mather_group_map_){
            delete gp_kv.second;
            gp_kv.second = nullptr;
        }

        // remove all entitys
        while (!entity_map_.empty()){
            entity* en = entity_map_.begin()->second;
            en->destory();
        }
    }

public:
    entity* get_entity(int64_t id){
        auto iter = entity_map_.find(id);
        if (iter != entity_map_.end()){
            return iter->second;
        }
        return nullptr;
    }

    bool    has_entity(int64_t id){
        auto iter = entity_map_.find(id);
        return iter != entity_map_.end();
    }

    uint32_t entity_count(){
        return entity_map_.size();
    }

    group* get_group(matcher::ptr mther){
        group* gp = nullptr;
        if (!mther){
            return nullptr;
        }

        matcher_hash_type mather_hash_code = mther->hash_code();
        auto iter = mather_group_map_.find(mather_hash_code);
        if (iter == mather_group_map_.end()){
            // new create a group
            gp = new group(mther, this);
            for (auto& en_kv : entity_map_){
                gp->handle_entity_match(en_kv.second);
            }
            mather_group_map_.insert(std::make_pair(mather_hash_code, gp));

            // todo, fire group create event
        }
        else{
            gp = iter->second;
        }
        return gp;
    }

protected:
    int64_t generate_next_entity_id(){
        return ++next_entity_id_;
    }
};
}

#endif