/**
 *
 * a ecs framework implement of cpp
 * file: matcher.hpp
 *
 * a matcher to match a entity group with specific componet tuple
 * 
 * @author  :   yandaren1220@126.com
 * @date    :   2017-08-20
 */

#ifndef __ydk_ecs_mather_hpp__
#define __ydk_ecs_mather_hpp__

#include <ecs_cpp/entity.hpp>
#include <cstdint>
#include <vector>
#include <typeindex>
#include <functional>
#include <memory>

namespace ecs_cpp
{
class entity;
class matcher
{
public:
    typedef std::shared_ptr<matcher> ptr;

protected:
    component_type_list all_of_component_type_list_;
    component_type_list any_of_component_type_list_;
    component_type_list none_of_component_type_list_;
    uint32_t            hash_code_;

public:
    template<typename ... Components>
    static matcher::ptr all_of(){
        matcher::ptr mther = std::make_shared<matcher>();
        std::vector<std::function<void()>> funs = { std::bind(&matcher::add_to_allof_list<Components>, mther.get())... };
        for (auto& f : funs){
            f();
        }
        mther->calc_hash_code();
        return mther;
    }

    template<typename ... Components>
    static matcher::ptr any_of(){
        matcher::ptr mther = std::make_shared<matcher>();
        std::vector<std::function<void()>> funs = { std::bind(&matcher::add_to_anyof_list<Components>, mther.get())... };
        for (auto& f : funs){
            f();
        }
        mther->calc_hash_code();
        return mther;
    }

    template<typename ... Components>
    static matcher::ptr none_of(){
        matcher::ptr mther = std::make_shared<matcher>();
        std::vector<std::function<void()>> funs = { std::bind(&matcher::add_to_noneof_list<Components>, mther.get())... };
        for (auto& f : funs){
            f();
        }
        mther->calc_hash_code();
        return mther;
    }

public:
    matcher() : hash_code_(0){}
    ~matcher(){}

public:
    bool matches(entity* en){
        return ((all_of_component_type_list_.empty() || en->has_components(all_of_component_type_list_)) &&
                (any_of_component_type_list_.empty() || en->has_any_components(any_of_component_type_list_)) &&
                (none_of_component_type_list_.empty() || en->has_none_components(none_of_component_type_list_)));
    }

    uint32_t hash_code() const { 
        return hash_code_; 
    }

    bool operator ==(const matcher& that) const{
        return (hash_code() == that.hash_code() && 
                check_component_vector_equal(all_of_component_type_list_, that.all_of_component_type_list_) && 
                check_component_vector_equal(any_of_component_type_list_, that.all_of_component_type_list_) && 
                check_component_vector_equal(none_of_component_type_list_, that.none_of_component_type_list_));
    }

protected:
    static bool check_component_vector_equal(const component_type_list& list1, const component_type_list& list2){
        if (list1.size() != list2.size()){
            return false;
        }

        for (std::size_t i = 0; i < list1.size(); ++i){
            if (list1[i] != list2[i]){
                return false;
            }
        }
        return true;
    }

    void calc_hash_code(){
        uint32_t hash = typeid(matcher).hash_code();
        hash = apply_hash(hash, all_of_component_type_list_, 3, 53);
        hash = apply_hash(hash, any_of_component_type_list_, 307, 367);
        hash = apply_hash(hash, none_of_component_type_list_, 647, 683);
        hash_code_ = hash;
    }

    template<typename C>
    void add_to_allof_list(){
        uint32_t type_id = typeid(C).hash_code();
        auto iter = std::find(all_of_component_type_list_.begin(), all_of_component_type_list_.end(), type_id);
        if (iter == all_of_component_type_list_.end()){
            all_of_component_type_list_.push_back(type_id);
        }
    }

    template<typename C>
    void add_to_anyof_list(){
        uint32_t type_id = typeid(C).hash_code();
        auto iter = std::find(any_of_component_type_list_.begin(), any_of_component_type_list_.end(), type_id);
        if (iter == any_of_component_type_list_.end()){
            any_of_component_type_list_.push_back(type_id);
        }
    }

    template<typename C>
    void add_to_noneof_list(){
        uint32_t type_id = typeid(C).hash_code();
        auto iter = std::find(none_of_component_type_list_.begin(), none_of_component_type_list_.end(), type_id);
        if (iter == none_of_component_type_list_.end()){
            none_of_component_type_list_.push_back(type_id);
        }
    }

private:
    uint32_t apply_hash(uint32_t hash, const component_type_list& list, int32_t i1, int32_t i2){
        if (list.size() > 0){
            for (size_t i = 0; i < list.size(); ++i){
                hash ^= list[i] * i1;
            }
            hash ^= list.size() * i2;
        }
        return hash;
    }
};
}

#endif