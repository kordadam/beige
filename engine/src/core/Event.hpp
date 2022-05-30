#pragma once

#include <functional>
#include <vector>

namespace beige {
namespace core {

template<typename... Arguments>
class Event {
public:
    using Callback = std::function<void(const Arguments&...)>;
    using Subscription = uint32_t;

    Event() = default;
    virtual ~Event() = default;

    virtual auto subscribe(const Callback& callback) -> Subscription;
    virtual auto unsubscribe(const Subscription subscription) -> void;

protected:
    std::vector<Callback> m_listeners;

    virtual auto notifyListeners(const Arguments&... arguments) -> void;

private:
    static inline Subscription m_subscriptionCount { 0u };
};

template<typename... Arguments>
auto Event<Arguments...>::subscribe(const Callback& callback) -> Subscription {
    m_listeners.push_back(callback);
    return m_subscriptionCount++;
}

template<typename... Arguments>
auto Event<Arguments...>::unsubscribe(const Subscription subscription) -> void {
    //m_listeners.erase(subscription);
}

template<typename... Arguments>
auto Event<Arguments...>::notifyListeners(const Arguments&... arguments) -> void {
    for (const Callback& listener : m_listeners) {
        listener(arguments...);
    }
}

} // namespace core
} // namespace beige
