/**
 *
 * a ecs framework implement of cpp
 * file: component.hpp
 *
 * the component only has data, has no function
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-08-19
 */

#ifndef __ydk_ecs_component_hpp__
#define __ydk_ecs_component_hpp__

#include <ecs_cpp/entity.hpp>

namespace ecs_cpp
{
class component
{
public:
    friend class entity;

    component() : entity_(nullptr){}
    virtual ~component(){}

public:
    template<typename C>
    C*      sibling();

protected:
    void    set_entity(entity* en){
        entity_ = en;
    }

protected:
    entity*  entity_;
};

template<typename C>
C*      component::sibling()
{
    if (entity_){
        return entity_->get_component<C>();
    }
    return nullptr;
}

}

#endif