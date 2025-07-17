// #include "FreeRTOS.h"
#ifndef BSP_ALLOCATE_HPP
#define BSP_ALLOCATE_HPP
#include <cstddef>
#include <utility>    

// Assuming pvPortMalloc and vPortFree are defined in FreeRTOS
extern "C" void* pvPortMalloc(size_t xWantedSize);
extern "C" void vPortFree(void* pv);

template <class _TypeT>
class OSallocator
{
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _TypeT value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    OSallocator() noexcept {}

    OSallocator(const OSallocator&) noexcept {}

    template <class _TypeU>
    struct rebind {
        typedef OSallocator<_TypeU> other;
    };

    template <class _TypeU>
    OSallocator(const OSallocator<_TypeU>&) noexcept {}

    template <class _TypeU>
    OSallocator& operator=(const OSallocator<_TypeU>&) noexcept {
        return *this;
    }

    pointer address(reference __x) const {
        return &__x;
    }

    const_pointer address(const_reference __x) const {
        return &__x;
    }

    pointer allocate(size_type __n, const void* = 0) {
        if (__n == 0) return nullptr;
        pointer __p = static_cast<pointer>(pvPortMalloc(__n * sizeof(value_type)));
        // if (!__p) throw std::bad_alloc();
        // return __p;
    }

    void deallocate(pointer __p, size_type) {
        vPortFree(__p);
    }

    size_type max_size() const noexcept {
        return size_type(-1) / sizeof(value_type);
    }

    template <class _Up, class... _Args>
    void construct(_Up* __p, _Args&&... __args) {
        ::new((void *)__p) _Up(std::forward<_Args>(__args)...);
    }

    template <class _Up>
    void destroy(_Up* __p) noexcept {
        __p->~_Up();
    }
};

// Specialization for void
template <>
class OSallocator<void>
{
public:
    typedef void* pointer;
    typedef const void* const_pointer;
    typedef void value_type;

    template <class _Up>
    struct rebind {
        typedef OSallocator<_Up> other;
    };
};

// Specialization for const void
template <>
class OSallocator<const void>
{
public:
    typedef const void* pointer;
    typedef const void* const_pointer;
    typedef const void value_type;

    template <class _Up>
    struct rebind {
        typedef OSallocator<_Up> other;
    };
};
#endif