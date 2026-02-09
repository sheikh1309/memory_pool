#ifndef MEMORY_POOL_POOLALLOCATOR_H
#define MEMORY_POOL_POOLALLOCATOR_H

class PoolAllocator {
public:
    PoolAllocator(size_t block_size, size_t block_count);
    ~PoolAllocator();

    void* allocate();
    void deallocate(void* ptr);

private:
    struct Node {
        Node* next;
    };

    size_t m_block_size;
    size_t m_block_count;
    char* m_memory_pool;
    Node* m_free_list;
};

#endif
