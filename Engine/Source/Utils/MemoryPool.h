#pragma once

#include "Common.h"
#include "Utils/NonCopyable.h"

namespace v3d
{
namespace utils
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class DefaultMemoryPoolAllocator;

    class MemoryPool final : utils::NonCopyable
    {
    public:

        class MemoryPoolAllocator
        {
        public:

            MemoryPoolAllocator() {};
            virtual ~MemoryPoolAllocator() {};

            virtual void* allocate(u64 size, s32 aligment = 0) = 0;
            virtual void  deallocate(void* block, u64 size = 0) = 0;
        };

        static const u64 k_defaultMemoryPoolSize = 1024;
        static const u64 k_defaultMemoryChunkSize = 64;
        static const u64 k_defaultMemorySizeToAllocate = k_defaultMemoryPoolSize * 2;

        MemoryPool(u64 initialMemoryPoolSize = k_defaultMemoryPoolSize,
                u64 memoryChunkSize = k_defaultMemoryChunkSize,
                u64 minimalMemorySizeToAllocate = k_defaultMemorySizeToAllocate,
                MemoryPoolAllocator* allocator = MemoryPool::getDefaultMemoryPoolAllocator());

        ~MemoryPool();

        void*                   getMemory(u64 memorySize);
        void                    freeMemory(void* memoryBlock, u64 memoryBlockSize = 0);

        bool                    isValidPointer(void* pointer) const;
        u64                     getOffsetInBlock(void* pointer);

        void                    clearPools();

        static MemoryPoolAllocator* getDefaultMemoryPoolAllocator();

    private:

        struct MemoryChunk
        {
            void*               _data;
            u64                 _dataSize;
            u64                 _usedSize;
            bool                _isAllocationChunk;
            MemoryChunk*       _next;

            void*               _memoryBlock;
        };

        bool                    allocateMemory(u64 memorySize);
        void                    freeAllAllocatedMemory();

        u32                     calculateNeededChunks(u64 memorySize);
        u64                     calculateBestMemoryBlockSize(u64 requestedMemoryBlockSize);


        MemoryChunk*            findChunkSuitableToHoldMemory(u64 memorySize);
        MemoryChunk*            findChunkHoldingPointerTo(void* memoryBlock);

        MemoryChunk*            skipChunks(MemoryChunk* startChunk, u32 chunksToSkip);
        MemoryChunk*            setChunkDefaults(MemoryChunk* chunk); 

        void                    freeChunks(MemoryChunk* chunk);
        void                    deallocateAllChunks();

        bool                    linkChunksToData(MemoryChunk* chunk, u32 chunkCount, void* newMemBlock);
        void                    setMemoryChunkValues(MemoryChunk* chunk, u64 memBlockSize);
        bool                    recalcChunkMemorySize(MemoryChunk* chunks, u32 chunkCount);

        MemoryPoolAllocator*    m_allocator;

        MemoryChunk*            m_firstChunk;
        MemoryChunk*            m_lastChunk;
        MemoryChunk*            m_currentChunk;

        u64                     m_totalMemoryPoolSize;
        u64                     m_usedMemoryPoolSize;
        u64                     m_freeMemoryPoolSize;

        u64                     m_memoryChunkSize;
        u32                     m_memoryChunkCount;
        u32                     m_objectCount;

        u64                     m_minimalMemorySizeToAllocate;

        static MemoryPoolAllocator* s_defaultMemoryPoolAllocator;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    class DefaultMemoryPoolAllocator : public MemoryPool::MemoryPoolAllocator
    {
    public:

        DefaultMemoryPoolAllocator() {};
        ~DefaultMemoryPoolAllocator() {};

        void* allocate(u64 size, s32 aligment = 0) override;
        void  deallocate(void* block, u64 size = 0) override;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////

} //namespace utils
} //namespace v3d
