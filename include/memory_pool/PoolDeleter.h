#ifndef MEMORY_POOL_POOLDELETER_H
#define MEMORY_POOL_POOLDELETER_H

#include <memory_pool/PoolAllocator.h>

#include "UniquePtr.h"
#include "SharedPtr.h"

template <typename T>
class PoolDeleter {
public:
    explicit PoolDeleter(PoolAllocator& pool) {
        m_pool = &pool;
    }

    PoolDeleter() {
        m_pool = nullptr;
    }

    void operator()(T* ptr) const {
        if (ptr && m_pool) {
            ptr->~T();
            m_pool->deallocate(ptr);
        }
    }

private:
    PoolAllocator* m_pool;
};

template <typename T, typename... Args>
unique_ptr<T, PoolDeleter<T>> make_unique_from_pool(PoolAllocator& pool, Args&&... args) {
    void* memory = pool.allocate();
    if (!memory) {
        return unique_ptr<T, PoolDeleter<T>>(nullptr, PoolDeleter<T>(pool));
    }

    T* obj = new (memory) T(std::forward<Args>(args)...);
    return unique_ptr<T, PoolDeleter<T>>(obj, PoolDeleter<T>(pool));
}

template <typename T, typename... Args>
shared_ptr<T> make_shared_from_pool(PoolAllocator& pool, Args&&... args) {
    void* memory = pool.allocate();
    if (!memory) {
        return shared_ptr<T>(nullptr);
    }

    T* obj = new (memory) T(std::forward<Args>(args)...);
    return shared_ptr<T>(obj, PoolDeleter<T>(pool));
}

#endif
