#include <ecs_cpp.hpp>
#include <iostream>

using namespace std;

namespace ecs_cpp{

class position : public component
{
public:
    int32_t x;
    int32_t y;
    int32_t z;
public:
    position(int32_t _x, int32_t _y, int32_t _z)
        : x(_x), y(_y), z(_z){
    }
};

class direction : public component
{
public:
    int32_t x;
    int32_t y;
    int32_t z;
public:
    direction(int32_t _x, int32_t _y, int32_t _z)
        : x(_x), y(_y), z(_z){
    }
};

class speed : public component
{
public:
    int32_t s;
public:
    speed(int32_t _s)
        : s(_s){
    }
};

void entity_comp_test()
{
    context ecs_ctx;

    entity* en = ecs_ctx.entity_admin.create_entity();
    printf("en id: %d\n", en->id());
    bool has_entity = en->has_component<position>();
    printf("has position component %d\n", has_entity);
    en->add_component<position>(1, 2, 3)
        .add_component<direction>(100, 200, 300);

    has_entity = en->has_component<position>();
    printf("has position component %d after added\n", has_entity);
    position* pos = en->get_component<position>();
    if (pos){
        printf("position { %d, %d, %d}\n", pos->x, pos->y, pos->z);
    }
    else{
        printf("get componet of position failed\n");
    }

    en->replace_component<position>(4, 5, 6);
    has_entity = en->has_component<position>();
    printf("has position component %d after replace\n", has_entity);
    pos = en->get_component<position>();
    if (pos){
        printf("position { %d, %d, %d}\n", pos->x, pos->y, pos->z);
    }
    else{
        printf("get componet of position failed\n");
    }
    direction* direct = en->get_component<direction>();
    printf("direct{%d, %d, %d}\n", direct->x, direct->y, direct->z);

    std::tuple<position*, direction*> comp_tuple = en->get_components<position, direction>();
    {
        pos = std::get<0>(comp_tuple);
        direct = std::get<1>(comp_tuple);
        printf("get by tuple\n");
        printf("\tposition { %d, %d, %d}\n", pos->x, pos->y, pos->z);
        printf("\tdirect {%d, %d, %d}\n", direct->x, direct->y, direct->z);
    }

    bool has_pos_dir_cmp_tuple = en->has_components<position, direction>();
    printf("has pos, direction component tuple %d\n", has_pos_dir_cmp_tuple);
    bool has_pos_dir_speed_camp_tuple = en->has_components<position, direction, speed>();
    printf("has pos, direction, speed componet tuple %d\n", has_pos_dir_speed_camp_tuple);
    en->add_component<speed>(1000);
    {
        speed* spd = en->get_component<speed>();
        pos = spd->sibling<position>();
        direct = spd->sibling<direction>();
        printf("component sibling test\n");
        printf("\tposition { %d, %d, %d}\n", pos->x, pos->y, pos->z);
        printf("\tdirect {%d, %d, %d}\n", direct->x, direct->y, direct->z);
        printf("\tspeed {%d}\n", spd->s);
    }

    en->remove_component<position>();
    has_entity = en->has_component<position>();
    printf("has position component %d after remove\n", has_entity);
}

void mather_test(){
    matcher::ptr mt_all = matcher::all_of<position, direction, speed>();
    printf("mather all hash_code: %u\n", mt_all->hash_code());

    matcher::ptr mt_any = matcher::any_of<position, direction, speed>();
    printf("mather any hash_code: %u\n", mt_any->hash_code());

    matcher::ptr mt_none = matcher::none_of<position, direction, speed>();
    printf("mather none hash_code: %u\n", mt_none->hash_code());

    context ecs_ctx;
    entity* en = ecs_ctx.entity_admin.create_entity();
    bool ret = mt_all->matches(en);
    printf("match all %d\n", ret);

    printf("add position, direction, speed components\n");
    en->add_component<position>(1, 2, 3)
        .add_component<direction>(4, 5, 6)
        .add_component<speed>(1000);

    ret = mt_all->matches(en);
    printf("match all %d after add component{position, direction, speed}\n", ret);

    ret = mt_any->matches(en);
    printf("match any %d after add component{position, direction, speed}\n", ret);

    ret = mt_none->matches(en);
    printf("match none %d after add component{position, direction, speed}\n", ret);

    group* all_group = ecs_ctx.entity_admin.get_group(mt_all);
    printf("group of mather all, entity count: %d\n", all_group->entity_count());

    group* any_grop = ecs_ctx.entity_admin.get_group(mt_any);
    printf("group of mather any, entity count: %d\n", any_grop->entity_count());

    group* none_group = ecs_ctx.entity_admin.get_group(mt_none);
    printf("group of mather none, entity count: %d\n", none_group->entity_count());

    printf("remove components{position, direction, speed}\n");
    en->remove_component<position>()
        .remove_component<direction>()
        .remove_component<speed>();

    ret = mt_none->matches(en);
    printf("match none %d after remove component{position, direction, speed}\n", ret);

    en->destory();

    //::system("pause");
}

}



int main()
{
    printf("ecs_cpp test\n");

    // ecs_cpp::entity_comp_test();

    ecs_cpp::mather_test();

    system("pause");
    return 0;
}