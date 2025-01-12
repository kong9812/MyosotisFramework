#pragma once
#include <memory>

#define TYPEDEF_SHARED_PTR(T)                                               \
    typedef std::shared_ptr<T> T##_ptr;                                     \
    inline T##_ptr Create##T##Pointer() { return std::make_shared<T>(); }   

#define TYPEDEF_SHARED_PTR_ARGS(T)                                          \
    typedef std::shared_ptr<T> T##_ptr;                                     \
    template<typename... Args>                                              \
    inline T##_ptr Create##T##Pointer(Args&&... args) { return std::make_shared<T>(std::forward<Args>(args)...); }

#define TYPEDEF_UNIQUE_PTR(T)                                               \
    typedef std::unique_ptr<T> T##_ptr;                                     \
    inline T##_ptr Create##T##Pointer() { return std::make_unique<T>(); }   

#define TYPEDEF_UNIQUE_PTR_ARGS(T)                                          \
    typedef std::unique_ptr<T> T##_ptr;                                     \
    template<typename... Args>                                              \
    inline T##_ptr Create##T##Pointer(Args&&... args) { return std::make_unique<T>(std::forward<Args>(args)...); }
