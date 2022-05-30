#pragma once

#include "../Defines.hpp"
#include "InputTypes.hpp"
#include "Event.hpp"

#include <cstdint>

namespace beige {
namespace core {

class BEIGE_API Input final :
    public Event<KeyEventCode, Key>,
    public Event<MouseEventCode, MouseState> {
public:
    using Event<KeyEventCode, Key>::subscribe;
    using Event<KeyEventCode, Key>::unsubscribe;
    using Event<KeyEventCode, Key>::notifyListeners;
    using Event<MouseEventCode, MouseState>::subscribe;
    using Event<MouseEventCode, MouseState>::unsubscribe;
    using Event<MouseEventCode, MouseState>::notifyListeners;

    Input();
    ~Input();

    auto update(const float deltaTime) -> void;

    auto isKeyDown(const Key key) -> bool;
    auto isKeyUp(const Key key) -> bool;
    auto wasKeyDown(const Key key) -> bool;
    auto wasKeyUp(const Key key) -> bool;

    auto processKey(const Key key, const bool isPressed) -> void;
    auto processButton(const Button button, const bool isPressed) -> void;
    auto processMouseMove(const int32_t xPos, const int32_t yPos) -> void;
    auto processMouseWheel(const int32_t deltaZ) -> void;

private:
    KeyboardState m_currentKeyboardState;
    KeyboardState m_previousKeyboardState;
    MouseState m_currentMouseState;
    MouseState m_previousMouseState;
};

} // namespace core
} // namespace beige
