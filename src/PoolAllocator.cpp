#include <memory_pool/PoolAllocator.h>

PoolAllocator::PoolAllocator(size_t block_size, size_t block_count) {
    m_block_size = block_size;
    m_block_count = block_count;
    m_memory_pool = new char[block_size * block_count];
    m_free_list = nullptr;

    for (size_t i = 0; i < block_count; ++i) {
        char* block = m_memory_pool + (i * block_size);
        Node* node = reinterpret_cast<Node*>(block);
        node->next = m_free_list;
        m_free_list = node;
    }
}

PoolAllocator::~PoolAllocator() {
    delete[] m_memory_pool;
}

void* PoolAllocator::allocate() {
    if (m_free_list == nullptr) {
        return nullptr;
    }

    Node* block = m_free_list;
    m_free_list = m_free_list->next;
    return block;
}

void PoolAllocator::deallocate(void* ptr) {
    if (ptr == nullptr) return;

    Node* node = reinterpret_cast<Node*>(ptr);
    node->next = m_free_list;
    m_free_list = node;
}
