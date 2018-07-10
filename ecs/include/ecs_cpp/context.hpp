/**
 *
 * a ecs framework implement of cpp
 * file: context.hpp
 * 
 * the ecs framework context(also the manager of the entits and events
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-08-19
 */

#ifndef __ydk_ecs_context_hpp__
#define __ydk_ecs_context_hpp__

#include <ecs_cpp/entity_manager.hpp>

namespace ecs_cpp{

class context
{
public:
    entity_manager entity_admin;

public:
    context(){}
    virtual ~context(){}
};
}

#endif