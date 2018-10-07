//#pragma once
//
//#include "common.h"
//
//namespace v3d
//{
//namespace utils
//{
//    /////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    class MemoryPool
//    {
//    public:
//
//        class MemoryPoolAllocator
//        {
//        public:
//
//            MemoryPoolAllocator() {};
//            virtual ~MemoryPoolAllocator() {};
//
//            virtual void* allocate(u64 size, s32 aligment = 0) = 0;
//            virtual void  deallocate(void* block, u64 size = 0) = 0;
//        };
//
//        class DefaultMemoryPoolAllocator : public MemoryPoolAllocator
//        {
//        public:
//
//            DefaultMemoryPoolAllocator() {};
//            ~DefaultMemoryPoolAllocator() {};
//
//            void* allocate(u64 size, s32 aligment = 0) override;
//            void  deallocate(void* block, u64 size = 0) override;
//        };
//
//        static const u64 k_defaultMemoryPoolSize = 1000;
//        static const u64 k_defaultMemoryChunkSize = 128;
//        static const u64 k_defaultMemorySizeToAllocate = k_defaultMemoryChunkSize * 2;
//
//        MemoryPool(u64 initialMemoryPoolSize = k_defaultMemoryPoolSize,
//                u64 memoryChunkSize = k_defaultMemoryChunkSize,
//                u64 minimalMemorySizeToAllocate = k_defaultMemorySizeToAllocate,
//                MemoryPoolAllocator* allocator = MemoryPool::getDefaultMemoryPoolAllocator());
//
//        ~MemoryPool();
//
//        void*                   getMemory(u64 memorySize);
//        void                    freeMemory(void* memoryBlock, u64 memoryBlockSize);
//
//        bool                    isValidPointer(void* pointer) const;
//        u64                     getOffsetInBlock(void* pointer);
//
//        static MemoryPoolAllocator* getDefaultMemoryPoolAllocator();
//
//    private:
//
//        struct SMemoryChunk
//        {
//            void*               _data;
//            u64                 _dataSize;
//            u64                 _usedSize;
//            bool                _isAllocationChunk;
//            SMemoryChunk*       _next;
//
//            void*               _memoryBlock;
//        };
//
//        bool                    allocateMemory(u64 memorySize);
//        void                    freeAllAllocatedMemory();
//
//        u32                     calculateNeededChunks(u64 memorySize);
//        u64                     calculateBestMemoryBlockSize(u64 requestedMemoryBlockSize);
//
//
//        SMemoryChunk*           findChunkSuitableToHoldMemory(u64 memorySize);
//        SMemoryChunk*           findChunkHoldingPointerTo(void* memoryBlock);
//
//        SMemoryChunk*           skipChunks(SMemoryChunk* startChunk, u32 chunksToSkip);
//        SMemoryChunk*           setChunkDefaults(SMemoryChunk* chunk); 
//
//        void                    freeChunks(SMemoryChunk* chunk);
//        void                    deallocateAllChunks();
//
//        bool                    linkChunksToData(SMemoryChunk* chunk, u32 chunkCount, void* newMemBlock);
//        void                    setMemoryChunkValues(SMemoryChunk* chunk, u64 memBlockSize);
//        bool                    recalcChunkMemorySize(SMemoryChunk* chunks, u32 chunkCount);
//
//        MemoryPoolAllocator*    m_allocator;
//
//        SMemoryChunk*           m_firstChunk;
//        SMemoryChunk*           m_lastChunk;
//        SMemoryChunk*           m_currentChunk;
//
//        u64                     m_totalMemoryPoolSize;
//        u64                     m_usedMemoryPoolSize;
//        u64                     m_freeMemoryPoolSize;
//
//        u64                     m_memoryChunkSize;
//        u32                     m_memoryChunkCount;
//        u32                     m_objectCount;
//
//        u64                     m_minimalMemorySizeToAllocate;
//
//        static DefaultMemoryPoolAllocator s_defaultAllocator;
//    };
//
//    /////////////////////////////////////////////////////////////////////////////////////////////////////
//
//} //namespace utils
//} //namespace v3d
