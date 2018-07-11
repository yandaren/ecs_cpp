/**
 *
 * null_mutex.hpp
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-04-19
 */

#ifndef __ydk_utility_sync_null_mutex_hpp__
#define __ydk_utility_sync_null_mutex_hpp__

namespace utility
{
namespace sync
{
struct null_mutex
{
    void lock() {}
    void unlock() {}
    bool try_lock()
    {
        return true;
    }
};

}
}

#endif