#pragma once

#include <assert.h>
#include <vector>
#include <array>
#include <list>
#include <map>

#define DEBUG_MEMORY 0
#define ENABLE_STATISTIC 0

namespace v3d
{
namespace utils
{
    typedef unsigned short      u16;
    typedef signed int          s32;
    typedef unsigned int        u32;
    typedef signed long long    s64;
    typedef unsigned long long  u64;
    typedef double              f64;
    typedef void*               address_ptr;

    /*
    * class MemoryPool
    */
    class MemoryPool final
    {
    public:

        MemoryPool(const MemoryPool&) = delete;
        MemoryPool& operator=(const MemoryPool&) = delete;

        /*
        * class MemoryAllocator. Allocator for pool
        */
        class MemoryAllocator
        {
        public:

            explicit MemoryAllocator() noexcept = default;
            virtual ~MemoryAllocator() = default;

            virtual address_ptr allocate(u64 size, u32 aligment = 0, void* user = nullptr) = 0;
            virtual void        deallocate(address_ptr memory, u64 size = 0, void* user = nullptr) = 0;
        };

        static constexpr u64 k_mixSizePageSize = 65'536;

        /*
        * MemoryPool constuctor
        * param maxAllocSize : block size for small allocation (best size 65KB, but no more)
        * param allocator: allocator
        * param user: user data
        */
        explicit MemoryPool(u64 maxAllocSize, MemoryAllocator* allocator = MemoryPool::getDefaultMemoryAllocator(), bool deleteUnusedPools = true, void* user = nullptr) noexcept;

        /*
        * ~MemoryPool destuctor
        */
        ~MemoryPool();

        /*
        * Request free memory from pool
        * param size: count bytes will be requested
        * param aligment: aligment
        */
        address_ptr allocMemory(u64 size, u32 aligment = 0);

        template<class T>
        T* allocElement()
        {
            return reinterpret_cast<T*>(allocMemory(sizeof(T)));
        }

        template<class T>
        T* allocArray(u64 count)
        {
            return reinterpret_cast<T*>(allocMemory(sizeof(T) * count));
        }

        /*
        * Return memory to pool
        * param address_ptr: address of memory
        */
        void freeMemory(address_ptr memory);

        /*
        * Prepare small table pools
        * call it if need more speed, but it creates a log of empty pools
        */
        void preAllocatePools();

        /*
        * Reset pools, Return all requested allocation
        */
        void reset();

        /*
        * Free pool, delete all pools
        */
        void clear();

        /*
        * default allocator
        */
        static MemoryAllocator* getDefaultMemoryAllocator();

        void collectStatistic();

    private:

        MemoryAllocator*    m_allocator;
        void*               m_userData;

        struct Pool;
        struct PoolTable;

        template<class T>
        struct Node
        {
            Node<T>() noexcept
                : _prev(nullptr)
                , _next(nullptr)
            {
            }

            Node<T>(const Node<T>&) = default;

            void reset()
            {
                _next = nullptr;
                _prev = nullptr;
            }

            T* _prev;
            T* _next;
        };

        template<class T>
        class List final
        {
        public:

            List() noexcept
            {
                List<T>::clear();
            }

            List(const List<T>&) = default;
            ~List() = default;

            T* begin()
            {
                return _end._next;
            }

            T* end()
            {
                return &_end;
            }

            bool empty() const
            {
                return &_end == _end._next;
            }

            void clear()
            {
                link(&_end, &_end);
#if DEBUG_MEMORY
                _size = 0;
#endif
            }

            void priorityInsert(T* node)
            {
                //TODO use binary search
                T* current = begin();
                while (current != end())
                {
                    if (node < current)
                    {
                        link(current->_prev, node);
                        link(node, current);
#if DEBUG_MEMORY
                        ++_size;
#endif
                        return;
                    }
                    current = current->_next;
                }

                link(_end._prev, node);
                link(node, &_end);
#if DEBUG_MEMORY
                ++_size;
#endif
            }

            void insert(T* node)
            {
                link(_end._prev, node);
                link(node, &_end);
#if DEBUG_MEMORY
                ++_size;
#endif
            }

            void merge()
            {
                if (List<T>::empty())
                {
                    return;
                }

                T* node = List<T>::begin();
                T* nodeNext = List<T>::begin()->_next;
                while (nodeNext != List<T>::end())
                {
                    address_ptr ptrNext = (address_ptr)(reinterpret_cast<u64>(node->ptr()) + node->_size);
                    if (nodeNext == ptrNext)
                    {
                        node->_size += nodeNext->_size;
                        nodeNext = List<T>::erase(nodeNext);

                        continue;
                    }

                    node = node->_next;
                    nodeNext = nodeNext->_next;
                }
            }

            T* erase(T* node)
            {
                link(node->_prev, node->_next);
#if DEBUG_MEMORY
                --_size;
                assert(_size >= 0);
#endif

                return node->_next;
            }

        private:

            void link(T* l, T* r)
            {
                l->_next = r;
                r->_prev = l;
            }

            T   _end;
#if DEBUG_MEMORY
            s64 _size;
#endif
        };

        struct Block : Node<Block>
        {
            Block()
                : _pool(nullptr)
                , _size(0)
            {
            }
            
            Block(const Block&) = default;

            Block(Pool* pool, u64 size) noexcept
                : _pool(pool)
                , _size(size)
            {
            }

            Pool*       _pool;
            u64         _size;
#if DEBUG_MEMORY
            address_ptr _ptr;
#endif //DEBUG_MEMORY
            address_ptr ptr()
            {
#if DEBUG_MEMORY
                u64 offset = sizeof(Block);
                address_ptr ptr = reinterpret_cast<address_ptr>(reinterpret_cast<u64>(this) + offset);
                assert(ptr);
                return ptr;
#else
                return reinterpret_cast<address_ptr>(reinterpret_cast<u64>(this) + sizeof(Block));
#endif
            }

            bool compare(Block* block)
            {
                address_ptr ptrNext = reinterpret_cast<address_ptr>(reinterpret_cast<u64>(block->ptr()) + block->_size);
                if (ptrNext == block)
                {
                    return true;
                }

                return false;
            }
        };

        struct Pool : Node<Pool>
        {
            Pool()
                : _table(nullptr)
                , _blockSize(0)
                , _poolSize(0)
            {
                _used.clear();
                _free.clear();
            }
            
            Pool(const Pool&) = default;

            Pool(PoolTable const* table, u64 blockSize, u64 poolSize) noexcept
                : _table(table)
                , _blockSize(blockSize)
                , _poolSize(poolSize)
            {
                _used.clear();
                _free.clear();
            }

            address_ptr ptr() const
            {
#if DEBUG_MEMORY
                u64 offset = sizeof(Pool);
                address_ptr ptr = reinterpret_cast<address_ptr>(reinterpret_cast<u64>(this) + offset);
                assert(ptr);
                return ptr;
#else
                return reinterpret_cast<address_ptr>(reinterpret_cast<u64>(this) + sizeof(Pool));
#endif
            }

            void reset()
            {
                Block* block = _used.begin();
                while (block != _used.end())
                {
                    Block* newBlock = block->_next;
                    _free.insert(block);

                    block = newBlock;
                }
                _used.clear();
            }

            PoolTable const*    _table;
            u64                 _blockSize;
            const u64           _poolSize;
            List<Block>         _used;
            List<Block>         _free;
        };

        struct PoolTable
        {
            enum Type : u32
            {
                Default = 0,
                SmallTable,
            };

            PoolTable() noexcept
                : _size(0)
                , _type(Type::Default)
            {
            }

            PoolTable(const PoolTable& table)
                : _size(0)
                , _type(Type::Default)
            {
                //wrong runtime logic, but need for compile
                assert(false);
            }

            explicit PoolTable(u16 size, PoolTable::Type type) noexcept
                : _size(size)
                , _type(Type::Default)
            {
            }

            List<Pool>  _activePools;
            List<Pool>  _fullPools;
            u64         _size;
            Type        _type;
        };

        static const u64 k_countPagesPerAllocation = 16;

        static const u64 k_maxSizeSmallTableAllocation = 32'768;
        std::array<u16, (k_maxSizeSmallTableAllocation >> 2)> m_smallTableIndex;
        std::vector<PoolTable>  m_smallPoolTables;

        PoolTable               m_poolTable;

        const u64               k_maxSizePoolAllocation;

        List<Block>             m_largeAllocations;

        static MemoryAllocator* s_defaultMemoryAllocator;


        Pool*   allocateFixedBlocksPool(PoolTable* table, u32 align);
        Pool*   allocatePool(PoolTable* table, u32 align);
        void    deallocatePool(Pool* pool);

        Block* initBlock(address_ptr ptr, Pool* pool, u64 size);
        void freeBlock(Block* block);

        Block* allocateFromSmallTables(u64 size);
        Block* allocateFromTable(u64 size);

        void collectEmptyPools(List<Pool>& pools, std::vector<Pool*>& markedToDelete);
        std::vector<Pool*> m_markedToDelete;

        const bool k_deleteUnusedPools;

#if ENABLE_STATISTIC
        struct Statistic
        {
            Statistic()
            {
                memset(this, 0, sizeof(Statistic));
            }

            void reset()
            {
                _allocateTime = 0;
                _dealocateTime = 0;

                _generalAllocationCount = 0;
                _generalAllocationSize = 0;

                for (u32 i = 0; i < 3; ++i)
                {
                    _tableAllocationCount[i] = 0;
                    _tableAllocationSizes[i] = 0;

                    _poolsAllocationCount[i] = 0;
                    _poolAllocationSizes[i] = 0;
                }
                _globalAllocationSize = 0;
            }

            template<u32 type>
            void registerAllocation(u64 size)
            {
                static_assert(type < 3);
                ++_tableAllocationCount[type];
                _tableAllocationSizes[type] += size;
                assert((u64)_tableAllocationCount[type] < (u64)std::numeric_limits<s64>().max());

                ++_generalAllocationCount;
                assert((u64)_generalAllocationCount < (u64)std::numeric_limits<s64>().max());
                _generalAllocationSize += size;
            }

            template<u32 type>
            void registerDeallocation(u64 size)
            {
                static_assert(type < 3);
                --_tableAllocationCount[type];
                _tableAllocationSizes[type] -= size;
                assert(_tableAllocationCount[type] >= 0 && _tableAllocationSizes[type] >= 0);

                --_generalAllocationCount;
                _generalAllocationSize -= size;
                assert(_generalAllocationCount >= 0 && _generalAllocationSize >= 0);
            }

            template<u32 type>
            void registerPoolAllocation(u64 size)
            {
                static_assert(type < 3);
                ++_poolsAllocationCount[type];
                _poolAllocationSizes[type] += size;
                assert((u64)_poolsAllocationCount[type] < (u64)std::numeric_limits<s64>().max() && (u64)_poolAllocationSizes[type] < (u64)std::numeric_limits<s64>().max());
            }

            template<u32 type>
            void registerPoolDeallocation(u64 size)
            {
                static_assert(type < 3);
                --_poolsAllocationCount[type];
                _poolAllocationSizes[type] -= size;
                assert(_poolsAllocationCount[type] >= 0 && _poolAllocationSizes[type] >= 0);
            }

            u64 _allocateTime;
            u64 _dealocateTime;

            s64 _tableAllocationCount[3];
            s64 _tableAllocationSizes[3];
            s64 _generalAllocationCount;
            u64 _generalAllocationSize;

            s64 _poolsAllocationCount[3];
            s64 _poolAllocationSizes[3];
            u64 _globalAllocationSize;

        };

        Statistic   m_statistic;
#endif //ENABLE_STATISTIC
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class DefaultMemoryAllocator : public MemoryPool::MemoryAllocator
    {
    public:

        explicit DefaultMemoryAllocator() noexcept = default;
        ~DefaultMemoryAllocator() = default;

        address_ptr allocate(u64 size, u32 aligment = 0, void* user = nullptr) override;
        void        deallocate(address_ptr memory, u64 size = 0, void* user = nullptr) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
