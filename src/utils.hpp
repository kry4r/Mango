//
// Created by gxy on 24-9-11.
//

#ifndef MANGO_SRC_UTILS_HPP
#define MANGO_SRC_UTILS_HPP
#include <memory>

namespace mango::core
{
    template<typename T>
    class Singleton {
    public:
        static T* current() {
            static T instance{};
            return &instance;
        }

        // 禁止拷贝和赋值操作
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

    protected:
        Singleton() = default;
        ~Singleton() = default;
    };
}

#endif //MANGO_SRC_UTILS_HPP
