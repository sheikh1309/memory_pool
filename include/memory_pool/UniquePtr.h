#ifndef MEMORY_POOL_UNIQUEPTR_H
#define MEMORY_POOL_UNIQUEPTR_H
#include <utility>

template <typename T>
struct default_delete {
    void operator()(T* ptr) const {
        delete ptr;
    }
};

template <typename T, typename Deleter = default_delete<T>>
class unique_ptr {
public:
    explicit unique_ptr(T* ptr = nullptr)
        : m_ptr(ptr), m_deleter(Deleter{}) {
    }

    unique_ptr(T* ptr, Deleter deleter) {
        m_ptr = ptr;
        m_deleter = deleter;
    }

    ~unique_ptr() {
        if (m_ptr) {
            m_deleter(m_ptr);
        }
    }

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    unique_ptr(unique_ptr&& other) noexcept {
        m_ptr = other.m_ptr;
        m_deleter = std::move(other.m_deleter);
        other.m_ptr = nullptr;
    }

    unique_ptr& operator=(unique_ptr&& other) noexcept {
        if (this != &other) {
            if (m_ptr) {
                m_deleter(m_ptr);
            }
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
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

    explicit operator bool() const noexcept {
        return m_ptr != nullptr;
    }

    Deleter& get_deleter() noexcept {
        return m_deleter;
    }

    const Deleter& get_deleter() const noexcept {
        return m_deleter;
    }

    T* release() noexcept {
        T* temp = m_ptr;
        m_ptr = nullptr;
        return temp;
    }

    void reset(T* ptr = nullptr) noexcept {
        if (m_ptr) {
            m_deleter(m_ptr);
        }
        m_ptr = ptr;
    }

    void swap(unique_ptr& other) noexcept {
        T* temp = m_ptr;
        m_ptr = other.m_ptr;
        other.m_ptr = temp;

        Deleter temp_deleter = std::move(other.m_deleter);
        m_deleter = std::move(temp_deleter);
        other.m_deleter = std::move(temp_deleter);
    }

private:
    T* m_ptr;
    Deleter m_deleter;
};

#endif
