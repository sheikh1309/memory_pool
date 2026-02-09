#ifndef MEMORY_POOL_SHAREDPTR_H
#define MEMORY_POOL_SHAREDPTR_H

#include <cstddef>
#include <utility>

struct ControlBlock {
    size_t shared_count = 1;
    size_t weak_count = 0;

    virtual void destroy_object() = 0;
    virtual ~ControlBlock() = default;
};

template <typename T>
struct DefaultControlBlock : ControlBlock {
    T* ptr;

    explicit DefaultControlBlock(T* p) : ptr(p) {}

    void destroy_object() override {
        delete ptr;
    }
};

template <typename T, typename Deleter>
struct DeleterControlBlock : ControlBlock {
    T* ptr;
    Deleter deleter;

    DeleterControlBlock(T* p, Deleter d) : ptr(p), deleter(std::move(d)) {}

    void destroy_object() override {
        deleter(ptr);
    }
};

template <typename T>
class shared_ptr {
public:
    shared_ptr(T* ptr = nullptr)
        : m_ptr(ptr), m_control_block(nullptr) {
        if (m_ptr) {
            m_control_block = new DefaultControlBlock<T>(m_ptr);
        }
    }

    template <typename Deleter>
    shared_ptr(T* ptr, Deleter deleter)
        : m_ptr(ptr), m_control_block(nullptr) {
        if (m_ptr) {
            m_control_block = new DeleterControlBlock<T, Deleter>(m_ptr, std::move(deleter));
        }
    }

    ~shared_ptr() {
        decrement_ref_count();
    }

    shared_ptr(const shared_ptr& other)
        : m_ptr(other.m_ptr), m_control_block(other.m_control_block) {
        increment_ref_count();
    }

    shared_ptr& operator=(const shared_ptr& other) {
        if (this != &other) {
            decrement_ref_count();
            m_ptr = other.m_ptr;
            m_control_block = other.m_control_block;
            increment_ref_count();
        }
        return *this;
    }

    shared_ptr(shared_ptr&& other) noexcept
        : m_ptr(other.m_ptr), m_control_block(other.m_control_block) {
        other.m_ptr = nullptr;
        other.m_control_block = nullptr;
    }

    shared_ptr& operator=(shared_ptr&& other) noexcept {
        if (this != &other) {
            decrement_ref_count();
            m_ptr = other.m_ptr;
            m_control_block = other.m_control_block;
            other.m_ptr = nullptr;
            other.m_control_block = nullptr;
        }
        return *this;
    }

    T* get() const noexcept {
        return m_ptr;
    }

    T& operator*() const {
        return *m_ptr;
    }

    T* operator->() const noexcept {
        return m_ptr;
    }

    size_t use_count() const noexcept {
        if (m_control_block) {
            return m_control_block->shared_count;
        }
        return 0;
    }

    explicit operator bool() const noexcept {
        return m_ptr != nullptr;
    }

    void reset(T* ptr = nullptr) {
        decrement_ref_count();
        m_ptr = ptr;
        if (m_ptr) {
            m_control_block = new DefaultControlBlock<T>(m_ptr);
        } else {
            m_control_block = nullptr;
        }
    }

    void swap(shared_ptr& other) noexcept {
        T* tmp_ptr = m_ptr;
        m_ptr = other.m_ptr;
        other.m_ptr = tmp_ptr;

        ControlBlock* tmp_cb = m_control_block;
        m_control_block = other.m_control_block;
        other.m_control_block = tmp_cb;
    }

private:
    void increment_ref_count() {
        if (m_control_block) {
            ++m_control_block->shared_count;
        }
    }

    void decrement_ref_count() {
        if (m_control_block) {
            --m_control_block->shared_count;
            if (m_control_block->shared_count == 0) {
                m_control_block->destroy_object();
                if (m_control_block->weak_count == 0) {
                    delete m_control_block;
                }
                m_ptr = nullptr;
                m_control_block = nullptr;
            }
        }
    }

    T* m_ptr;
    ControlBlock* m_control_block;
};

#endif
