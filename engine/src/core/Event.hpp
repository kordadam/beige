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

    virtual auto subscribe(const Callback& callback) -> Subscription final;
    virtual auto unsubscribe(const Subscription subscription) -> void final;

protected:
    virtual auto notifyListeners(const Arguments&... arguments) -> void final;

private:
    std::vector<Callback> m_listeners;
};

template<typename... Arguments>
auto Event<Arguments...>::subscribe(const Callback& callback) -> Subscription {
    const Subscription subscription { static_cast<Subscription>(m_listeners.size()) };
    m_listeners.push_back(callback);
    return subscription;
}

template<typename... Arguments>
auto Event<Arguments...>::unsubscribe(const Subscription subscription) -> void {
    m_listeners.erase(m_listeners.begin() + subscription);
}

template<typename... Arguments>
auto Event<Arguments...>::notifyListeners(const Arguments&... arguments) -> void {
    for (const Callback& listener : m_listeners) {
        listener(arguments...);
    }
}

} // namespace core
} // namespace beige
