/**
 *
 * memory_pool.hpp
 *
 * a simple implement of memory pool
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2017-06-20
 */

#ifndef __ydk_utility_pool_memory_pool_hpp__
#define __ydk_utility_pool_memory_pool_hpp__

#include <cstdint>
#include <list>
#include <vector>
#include <mutex>

namespace utility
{
    /** 
     * @brief 
     * memory pool, try to decrease the cost that allocate memory and free memory
     * try to reuse the object memory as far as possible
     */
    template<class Mutex>
    class memory_pool
    {
    public:

        typedef uint32_t            size_type;
        typedef void*               pointer;
        typedef std::list<pointer>  cell_queue_type;

        /** 
         * @brief
         * @param cell_size:            each cell's meory size
         * @param initial_cell_count:   initial cell count
         * @param grow_cell_count:      the memory pool infate speed
         */
        memory_pool(size_type cell_size, size_type initial_cell_count, size_type grow_cell_count = 1)
            :m_cell_size(cell_size)
            ,m_grow_cell_count(grow_cell_count)
        {
            inflate(initial_cell_count);
        }

        virtual ~memory_pool()
        {
            clear();
        }

        /** 
         * free the memory
         */
        void    clear()
        {
            std::lock_guard<Mutex> locker(m_mtx);

            auto iter = m_total_cells.begin();
            for( iter; iter != m_total_cells.end(); ++ iter )
            {
                free(*iter);
            }
            m_total_cells.clear();
            m_free_cells.clear();
        }

        /** 
         * allocate a cell 
         */
        virtual pointer  allocate()
        {
            std::lock_guard<Mutex> locker(m_mtx);

            if( m_free_cells.empty())
            {
                inflate(m_grow_cell_count);
            }

            pointer ret = m_free_cells.front();
            m_free_cells.pop_front();

            return ret;
        }

        /** 
         * reclaim the cell
         */
        virtual void    reclaim(pointer p)
        {
            std::lock_guard<Mutex> locker(m_mtx);

            m_free_cells.push_back(p);
        }

        /** 
         * @brief get current free cell count
         */
        inline  size_type free_cell_count()
        {
            std::lock_guard<Mutex> locker(m_mtx);

            return m_free_cells.size();
        }

        /** 
         * @brief get current free memory size
         */
        inline  size_type  free_memory_size()
        {
            std::lock_guard<Mutex> locker(m_mtx);

            return m_free_cells.size() * m_cell_size;
        }

        /** 
         * @brief get current total cell count
         */
        inline  size_type   total_cell_count()
        {
            std::lock_guard<Mutex> locker(m_mtx);

            return m_total_cells.size();
        }

        /** 
         * @brief get current total memory size
         */
        inline  size_type   total_memory_size()
        {
            std::lock_guard<Mutex> locker(m_mtx);

            return m_total_cells.size() * m_cell_size;
        }

    private:

        /** 
         * inflate the pool size
         */
        void    inflate(size_type count)
        {
            for( size_type i = 0; i < count; ++ i )
            {
                pointer p = malloc(m_cell_size);
                m_free_cells.push_back(p);
                m_total_cells.push_back(p);
            }
        }

    private:

        size_type       m_cell_size;        // 每个单元的内存大小
        
        size_type       m_grow_cell_count;  // 内存池内存膨胀的速度（增加的单元数)

        cell_queue_type m_free_cells;       // 空闲的单元列表

        cell_queue_type m_total_cells;      // 总单元列表

        Mutex           m_mtx;              // 互斥量 
    };

    template<class T, class Mutex>
    class memory_pool_ex : public memory_pool<Mutex>
    {
    public:
        memory_pool_ex(size_type initial_cell_count, size_type grow_cell_count = 1)
            : memory_pool(sizeof(T), initial_cell_count, grow_cell_count)
        {
        }
        virtual ~memory_pool_ex(){
        }

    public:
        virtual void    reclaim(pointer p) override
        {
            T* p_t = static_cast<T*>(p);
            if (p_t){
                p_t->~T();
            }
            memory_pool<Mutex>::reclaim(p);
        }
    };


    /** 
     * memory pool fixed pool size, the pool size will not inflate when there is no free cell
     * the dvantage is that the memory buffer is continuous
     */
     template<class Mutex>
     class memory_pool_fixsize
     {
public:
        typedef uint32_t                size_type;
        typedef void*                   pointer;
        typedef uint32_t                cell_index_type;
        typedef std::vector<pointer>    cell_array_type;

        /** 
         * @brief
         * @param cell_size:            each cell's meory size
         * @param total_cell_count:    total cell count
         */
        memory_pool_fixsize(size_type cell_size, size_type total_cell_count)
            :m_cell_size(cell_size)
        {
            m_total_cells.resize(total_cell_count, 0);

            m_buffer = malloc( m_cell_size * total_cell_count );

            void* p = m_buffer;
            for(uint32_t i = 0; i < total_cell_count; ++ i )
            {
                m_total_cells[i] = p;
                p = (void*)((char*)p + m_cell_size);
            }
            m_free_cell_index = total_cell_count;
        }

        ~memory_pool_fixsize()
        {
            clear();
        }

        /** 
         * free the memory
         */
        void    clear()
        {
            std::lock_guard<Mutex> locker(m_mtx);

            if( m_buffer )
            {
                free(m_buffer);
                m_buffer = NULL;
            }
              
            m_total_cells.clear();
        }

        /** 
         * allocate a cell 
         */
        pointer  allocate()
        {
            std::lock_guard<Mutex> locker(m_mtx);

            pointer ret = NULL;
            if( m_free_cell_index > 0 )
            {
                m_free_cell_index --;
                ret = m_total_cells[m_free_cell_index];
            }else
            {
                ret = malloc(m_cell_size);
            }
            return ret;
        }

        /** 
         * reclaim the cell
         */
        bool    reclaim(pointer p)
        {
            std::lock_guard<Mutex> locker(m_mtx);

            if( p >= m_buffer && p < ((char*)m_buffer + m_cell_size * m_total_cells.size()) )
            {
                if( m_free_cell_index < m_total_cells.size() )
                {
                    m_total_cells[m_free_cell_index] = p;
                    m_free_cell_index ++;
                }else
                {
                    return false;
                }
            }else
            {
                free(p);
            }

            return true;
        }

        /** 
         * @brief get current free cell count
         */
        inline  size_type free_cell_count()
        {
            std::lock_guard<Mutex> locker(m_mtx);

            return m_free_cell_index;
        }

        /** 
         * @brief get current free memory size
         */
        inline  size_type  free_memory_size()
        {
            std::lock_guard<Mutex> locker(m_mtx);

            return m_free_cell_index * m_cell_size;
        }

        /** 
         * @brief get current total cell count
         */
        inline  size_type   total_cell_count()
        {
            std::lock_guard<Mutex> locker(m_mtx);

            return m_total_cells.size();
        }

        /** 
         * @brief get current total memory size
         */
        inline  size_type   total_memory_size()
        {
            std::lock_guard<Mutex> locker(m_mtx);

            return m_total_cells.size() * m_cell_size;
        }

    private:

        /** 
         * try get the cell index
         */

    private:

        cell_index_type     m_free_cell_index;          // 当前空闲的cell下标

        size_type           m_cell_size;                // 每个单元的内存大小

        cell_array_type     m_total_cells;              // 总单元列表

        pointer             m_buffer;                   // buffer的起始地址

        Mutex               m_mtx;
     };
}

#endif