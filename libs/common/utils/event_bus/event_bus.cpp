#include "event_bus.hpp"

EventBus::EventBus(EventBus&& other) noexcept :
    _subscriptionMap(std::move(other._subscriptionMap)),
    _idCounter(other._idCounter)
{};

EventBus& EventBus::operator = (EventBus&& other) noexcept {
    if (this != &other) {
        this->_subscriptionMap = std::move(other._subscriptionMap);
        this->_idCounter = std::move(other._idCounter);
    }
    return *this;
}