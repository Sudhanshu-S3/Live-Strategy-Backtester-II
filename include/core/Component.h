// include/core/Component.h
#ifndef HFT_SYSTEM_COMPONENT_H
#define HFT_SYSTEM_COMPONENT_H

#include <string>
#include <memory>
#include "EventBus.h"

namespace hft_system
{

    class Component
    {
    public:
        // Every component is initialized with a reference to the central EventBus.
        explicit Component(std::shared_ptr<EventBus> event_bus, std::string name)
            : event_bus_(event_bus), name_(std::move(name)) {}

        virtual ~Component() = default;

        // Optional: Pure virtual functions to enforce that derived components
        // implement start() and stop() methods for thread management.
        virtual void start() = 0;
        virtual void stop() = 0;

        const std::string &getName() const { return name_; }

    protected:
        std::shared_ptr<EventBus> event_bus_;
        const std::string name_;
    };

} // namespace hft_system

#endif // HFT_SYSTEM_COMPONENT_H