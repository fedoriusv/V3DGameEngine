//#include "MemoryPool.h"
//
////see link: http://www.codeproject.com/Articles/15527/C-Memory-Pool
//
//namespace v3d
//{
//namespace utils
//{
//
//#ifdef _DEBUG
//    static const c8  k_allocatedMemoryContent = 'H';
//#endif //_DEBUG
//
//MemoryPool::DefaultMemoryPoolAllocator MemoryPool::s_defaultAllocator;
//
//MemoryPool::MemoryPool(u64 initialMemoryPoolSize, u64 memoryChunkSize, u64 minimalMemorySizeToAllocate, MemoryPoolAllocator* allocator)
//    : m_allocator(allocator)
//    
//    , m_firstChunk(nullptr)
//    , m_lastChunk(nullptr)
//    , m_currentChunk(nullptr)
//
//    , m_totalMemoryPoolSize(0)
//    , m_usedMemoryPoolSize(0)
//    , m_freeMemoryPoolSize(0)
//
//    , m_memoryChunkSize(memoryChunkSize)
//    , m_memoryChunkCount(0)
//    , m_objectCount(0)
//
//    , m_minimalMemorySizeToAllocate(minimalMemorySizeToAllocate)
//{
//    ASSERT(m_allocator, "m_allocator is nullptr");
//
//    MemoryPool::allocateMemory(initialMemoryPoolSize);
//}
//
//MemoryPool::~MemoryPool()
//{
//    MemoryPool::freeAllAllocatedMemory();
//    MemoryPool::deallocateAllChunks();
//
//    ASSERT(m_objectCount == 0, "You have not free all allocated Memory");
//}
//
//void* MemoryPool::getMemory(u64 memorySize)
//{
//    u64 bestMemBlockSize = MemoryPool::calculateBestMemoryBlockSize(memorySize);
//
//    SMemoryChunk *chunk = nullptr;
//    while (!chunk)
//    {
//        // Is a Chunks available to hold the requested amount of Memory ?
//        chunk = MemoryPool::findChunkSuitableToHoldMemory(bestMemBlockSize);
//        if (!chunk)
//        {
//            // No chunk can be found
//            // => Memory-Pool is to small. We have to request 
//            //    more Memory from the Operating-System....
//            bestMemBlockSize = std::max(bestMemBlockSize, MemoryPool::calculateBestMemoryBlockSize(m_minimalMemorySizeToAllocate));
//            MemoryPool::allocateMemory(bestMemBlockSize);
//        }
//    }
//
//    // Finally, a suitable Chunk was found.
//    // Adjust the Values of the internal "TotalSize"/"UsedSize" Members and 
//    // the Values of the MemoryChunk itself.
//    m_usedMemoryPoolSize += bestMemBlockSize;
//    m_freeMemoryPoolSize -= bestMemBlockSize;
//    m_objectCount++;
//    MemoryPool::setMemoryChunkValues(chunk, bestMemBlockSize);
//
//    // eventually, return the Pointer to the User
//    return ((void *)chunk->_data);
//}
//
//void MemoryPool::freeMemory(void* memoryBlock, u64 memoryBlockSize)
//{
//    // Search all Chunks for the one holding the "ptrMemoryBlock"-Pointer
//    // ("SMemoryChunk->Data == ptrMemoryBlock"). Eventually, free that Chunks,
//    // so it beecomes available to the Memory-Pool again...
//    SMemoryChunk *chunk = MemoryPool::findChunkHoldingPointerTo(memoryBlock);
//    if (chunk)
//    {
//        MemoryPool::freeChunks(chunk);
//    }
//    else
//    {
//        ASSERT(false,"Requested Pointer not in Memory Pool");
//    }
//
//    ASSERT(m_objectCount > 0, "Request to delete more Memory then allocated.");
//    m_objectCount--;
//}
//
//bool MemoryPool::isValidPointer(void* pointer) const
//{
//    SMemoryChunk* chunk = m_firstChunk;
//    while (chunk)
//    {
//        if (chunk->_data == pointer)
//        {
//            return true;
//        }
//        chunk = chunk->_next;
//    }
//
//    return false;
//}
//
//u64 MemoryPool::getOffsetInBlock(void* pointer)
//{
//    SMemoryChunk* chunk = m_firstChunk;
//    while (chunk)
//    {
//        if (chunk->_data == pointer)
//        {
//            u64 offset = (u64)pointer - (u64)chunk->_memoryBlock;
//            return offset;
//        }
//        chunk = chunk->_next;
//    }
//
//    return 0;
//}
//
//MemoryPool::MemoryPoolAllocator* MemoryPool::getDefaultMemoryPoolAllocator()
//{
//    return &s_defaultAllocator;;
//}
//
//bool MemoryPool::allocateMemory(u64 memorySize)
//{
//    // This function will allocate *at least* "memorySize"-Bytes from the Operating-System.
//
//    // How it works :
//    // Calculate the amount of "SMemoryChunks" needed to manage the requested MemorySize.
//    // Every MemoryChunk can manage only a certain amount of Memory
//    // (set by the "m_sMemoryChunkSize"-Member of the Memory-Pool).
//    //
//    // Also, calculate the "Best" Memory-Block size to allocate from the 
//    // Operating-System, so that all allocated Memory can be assigned to a
//    // Memory Chunk.
//    // Example : 
//    // You want to Allocate 120 Bytes, but every "SMemoryChunk" can only handle
//    //    50 Bytes ("m_sMemoryChunkSize = 50").
//    //    So, "CalculateNeededChunks()" will return the Number of Chunks needed to
//    //    manage 120 Bytes. Since it is not possible to divide 120 Bytes in to
//    //    50 Byte Chunks, "CalculateNeededChunks()" returns 3.
//    //    ==> 3 Chunks can Manage 150 Bytes of data (50 * 3 = 150), so
//    //        the requested 120 Bytes will fit into this Block.
//    //    "CalculateBestMemoryBlockSize()" will return the amount of memory needed
//    //    to *perfectly* subdivide the allocated Memory into "m_sMemoryChunkSize" (= 50) Byte
//    //    pieces. -> "CalculateBestMemoryBlockSize()" returns 150.
//    //    So, 150 Bytes of memory are allocated from the Operating-System and
//    //    subdivided into 3 Memory-Chunks (each holding a Pointer to 50 Bytes of the allocated memory).
//    //    Since only 120 Bytes are requested, we have a Memory-Overhead of 
//    //    150 - 120 = 30 Bytes. 
//    //    Note, that the Memory-overhead is not a bad thing, because we can use 
//    //    that memory later (it remains in the Memory-Pool).
//    //
//
//    u32 neededChunks = MemoryPool::calculateNeededChunks(memorySize);
//    u64 bestMemBlockSize = MemoryPool::calculateBestMemoryBlockSize(memorySize);
//
//
//    void* newMemBlock = m_allocator->allocate(bestMemBlockSize); // allocate from Operating System
//    SMemoryChunk* newChunks = (SMemoryChunk*)malloc((neededChunks * sizeof(SMemoryChunk))); // allocate Chunk-Array to Manage the Memory
//    ASSERT((newMemBlock) && (newChunks), "System ran out of Memory");
//
//    // Adjust internal Values (Total/Free Memory, etc.)
//    m_totalMemoryPoolSize += bestMemBlockSize;
//    m_freeMemoryPoolSize += bestMemBlockSize;
//    m_memoryChunkCount += neededChunks;
//
//    // Associate the allocated Memory-Block with the Linked-List of MemoryChunks
//    return MemoryPool::linkChunksToData(newChunks, neededChunks, newMemBlock);
//}
//
//void MemoryPool::freeAllAllocatedMemory()
//{
//    SMemoryChunk* chunk = m_firstChunk;
//    while (chunk)
//    {
//        if (chunk->_isAllocationChunk)
//        {
//            m_allocator->deallocate(chunk->_data);
//            chunk->_data = nullptr;
//        }
//        chunk = chunk->_next;
//    }
//}
//
//u32 MemoryPool::calculateNeededChunks(u64 memorySize)
//{
//    f32 f = (f32)(((f32)memorySize) / ((f32)m_memoryChunkSize));
//    return ((u32)ceil(f));
//}
//
//u64 MemoryPool::calculateBestMemoryBlockSize(u64 requestedMemoryBlockSize)
//{
//    u32 neededChunks = MemoryPool::calculateNeededChunks(requestedMemoryBlockSize);
//    return u64((neededChunks * m_memoryChunkSize));
//}
//
//MemoryPool::SMemoryChunk* MemoryPool::findChunkSuitableToHoldMemory(u64 memorySize)
//{
//    // Find a Chunk to hold *at least* "sMemorySize" Bytes.
//    u32 chunksToSkip = 0;
//    bool continueSearch = true;
//    SMemoryChunk* chunk = m_currentChunk; // Start search at Cursor-Pos.
//    for (u32 i = 0; i < m_memoryChunkCount; i++)
//    {
//        if (chunk)
//        {
//            if (chunk == m_lastChunk) // End of List reached : Start over from the beginning
//            {
//                chunk = m_firstChunk;
//            }
//
//            if (chunk->_dataSize >= memorySize)
//            {
//                if (chunk->_usedSize == 0)
//                {
//                    m_currentChunk = chunk;
//                    return chunk;
//                }
//            }
//
//            chunksToSkip = MemoryPool::calculateNeededChunks(chunk->_usedSize);
//
//            if (chunksToSkip == 0)
//            {
//                chunksToSkip = 1;
//            }
//            chunk = MemoryPool::skipChunks(chunk, chunksToSkip);
//        }
//        else
//        {
//            continueSearch = false;
//        }
//    }
//
//    return nullptr;
//}
//
//MemoryPool::SMemoryChunk* MemoryPool::findChunkHoldingPointerTo(void* memoryBlock)
//{
//    SMemoryChunk* tempChunk = m_firstChunk;
//    while (tempChunk)
//    {
//        if (tempChunk->_data == memoryBlock)
//        {
//            break;
//        }
//        tempChunk = tempChunk->_next;
//    }
//    return tempChunk;
//}
//
//MemoryPool::SMemoryChunk* MemoryPool::skipChunks(SMemoryChunk* startChunk, u32 chunksToSkip)
//{
//    SMemoryChunk* currentChunk = startChunk;
//    for (unsigned int i = 0; i < chunksToSkip; i++)
//    {
//        if (currentChunk)
//        {
//            currentChunk = currentChunk->_next;
//        }
//        else
//        {
//            // Will occur, if you try to Skip more Chunks than actually available
//            // from your "ptrStartChunk" 
//            ASSERT(false, "Chunk is nullptr");
//            break;
//        }
//    }
//    return currentChunk;
//}
//
//MemoryPool::SMemoryChunk* MemoryPool::setChunkDefaults(SMemoryChunk* chunk)
//{
//    if (chunk)
//    {
//        chunk->_data = nullptr;
//        chunk->_dataSize = 0;
//        chunk->_usedSize = 0;
//        chunk->_isAllocationChunk = false;
//        chunk->_next = nullptr;
//    }
//
//    return chunk;
//}
//
//void MemoryPool::freeChunks(SMemoryChunk* chunk)
//{
//    // Make the Used Memory of the given Chunk available
//    // to the Memory Pool again.
//    SMemoryChunk* currentChunk = chunk;
//    u32 chunkCount = MemoryPool::calculateNeededChunks(currentChunk->_usedSize);
//    for (u32 i = 0; i < chunkCount; i++)
//    {
//        if (currentChunk)
//        {
//            // Step 1 : Set the allocated Memory to 'FREEED_MEMORY_CONTENT'
//#ifdef _DEBUG
//            memset(currentChunk->_data, k_allocatedMemoryContent, m_memoryChunkSize);
//#endif //_DEBUG
//
//            // Step 2 : Set the Used-Size to Zero
//            currentChunk->_usedSize = 0;
//
//            // Step 3 : Adjust Memory-Pool Values and goto next Chunk
//            m_usedMemoryPoolSize -= m_memoryChunkSize;
//            currentChunk = currentChunk->_next;
//        }
//    }
//}
//
//void MemoryPool::deallocateAllChunks()
//{
//    SMemoryChunk* chunk = m_firstChunk;
//    SMemoryChunk* chunkToDelete = nullptr;
//    while (chunk)
//    {
//        if (chunk->_isAllocationChunk)
//        {
//            if (chunkToDelete)
//            {
//                free(chunkToDelete);
//                chunkToDelete = nullptr;
//            }
//            chunkToDelete = chunk;
//        }
//        chunk = chunk->_next;
//    }
//}
//
//bool MemoryPool::linkChunksToData(SMemoryChunk* newChunks, u32 chunkCount, void* newMemBlock)
//{
//    SMemoryChunk* newChunk = nullptr;
//    u32 memOffset = 0;
//    bool allocationChunkAssigned = false;
//    for (u32 i = 0; i < chunkCount; i++)
//    {
//        if (!m_firstChunk)
//        {
//            m_firstChunk = MemoryPool::setChunkDefaults(&(newChunks[0]));
//            m_lastChunk = m_firstChunk;
//            m_currentChunk = m_firstChunk;
//        }
//        else
//        {
//            newChunk = MemoryPool::setChunkDefaults(&(newChunks[i]));
//            m_lastChunk->_next = newChunk;
//            m_lastChunk = newChunk;
//        }
//
//        memOffset = (i * ((u32)m_memoryChunkSize));
//        u8* charNewMemBlock = (u8*)newMemBlock;
//        m_lastChunk->_data = &(charNewMemBlock[memOffset]);
//        m_lastChunk->_memoryBlock = newMemBlock;
//
//        // The first Chunk assigned to the new Memory-Block will be 
//        // a "AllocationChunk". This means, this Chunks stores the
//        // "original" Pointer to the MemBlock and is responsible for
//        // "free()"ing the Memory later....
//        if (!allocationChunkAssigned)
//        {
//            m_lastChunk->_isAllocationChunk = true;
//            allocationChunkAssigned = true;
//        }
//    }
//
//    return MemoryPool::recalcChunkMemorySize(m_firstChunk, m_memoryChunkCount);
//}
//
//void MemoryPool::setMemoryChunkValues(SMemoryChunk* chunk, u64 memBlockSize)
//{
//    if (chunk) // && (chunk != m_lastChunk))
//    {
//        chunk->_usedSize = memBlockSize;
//    }
//    else
//    {
//        ASSERT(false, "Invalid pointer");
//    }
//}
//
//bool MemoryPool::recalcChunkMemorySize(SMemoryChunk* chunks, u32 chunkCount)
//{
//    u32 memOffset = 0;
//    for (u32 i = 0; i < chunkCount; i++)
//    {
//        if (chunks)
//        {
//            memOffset = (i * ((u32)m_memoryChunkSize));
//            chunks->_dataSize = (((u32)m_totalMemoryPoolSize) - memOffset);
//            chunks = chunks->_next;
//        }
//        else
//        {
//            ASSERT(false, "chunk is nullptr");
//            return false;
//        }
//    }
//
//    return true;
//}
//
//void* MemoryPool::DefaultMemoryPoolAllocator::allocate(u64 size, s32 aligment)
//{
//    void* newBlock = malloc(size);
//
//    ASSERT(newBlock, "Invalid allocate");
//    return newBlock;
//}
//
//void MemoryPool::DefaultMemoryPoolAllocator::deallocate(void* block, u64 size)
//{
//    ASSERT(block, "Invalid block");
//    free(block);
//}
//
//} //namespace utils
//} //namespace v3d
