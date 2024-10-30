//
// Created by gxy on 24-9-11.
//

#ifndef MANGO_SRC_UTILS_HPP
#define MANGO_SRC_UTILS_HPP
#include <memory>

namespace mango::core
{
    struct Pinned {
        using Mobility = Pinned;
        using Core_Marker_Pinned = Pinned;

        Pinned() = default;
        Pinned(const Pinned&) = delete;          // Disable copy constructor
        Pinned(Pinned&&) = delete;               // Disable move constructor
        Pinned& operator=(const Pinned&) = delete; // Disable copy assignment
        Pinned& operator=(Pinned&&) = delete;      // Disable move assignment
    };

    template <class Instance>
    struct Singleton : core::Pinned {
    private:
        // Hold the actual instance
        static std::unique_ptr<Instance> instance_ptr;

    public:
        Singleton() = default;

        // Get reference to the instance
        static Instance& instance() {
            if (!instance_ptr) {
                instance_ptr = std::unique_ptr<Instance>(new Instance());
            }
            return *instance_ptr;
        }

        // Get a pointer to the current instance (may be nullptr)
        static Instance* current() {
            return instance_ptr.get();
        }

        // Get the unique_ptr itself to manage instance externally
        static std::unique_ptr<Instance>* controllable_instance() {
            return &instance_ptr;
        }

        // Drop the instance, resetting the unique_ptr
        static void drop_instance() {
            instance_ptr.reset();
        }
    };

    template <class Instance>
    std::unique_ptr<Instance> Singleton<Instance>::instance_ptr = nullptr;
}

#endif //MANGO_SRC_UTILS_HPP
