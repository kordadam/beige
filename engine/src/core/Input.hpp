#pragma once

#include "../Defines.hpp"
#include "InputTypes.hpp"
#include "Event.hpp"

#include <cstdint>
#include <vector>

namespace beige {
namespace core {

class BEIGE_API Input final :
public Event<KeyEventCode, Key>,
public Event<MouseEventCode, MouseState> {
public:
    using KeyEvent = Event<KeyEventCode, Key>;
    using MouseEvent = Event<MouseEventCode, MouseState>;

    ~Input() = default;

    static auto getInstance() -> std::shared_ptr<Input>;

    static auto update(const double deltaTime) -> void;

    auto isKeyDown(const Key key) -> bool;
    auto isKeyUp(const Key key) -> bool;
    auto wasKeyDown(const Key key) -> bool;
    auto wasKeyUp(const Key key) -> bool;

    auto processKey(const Key key, const bool isPressed) -> void;
    auto processButton(const Button button, const bool isPressed) -> void;
    auto processMouseMove(const int32_t xPos, const int32_t yPos) -> void;
    auto processMouseWheel(const int32_t deltaZ) -> void;

private:
    Input() = default;

    static std::shared_ptr<Input> m_instance;

    static std::vector<bool> m_currentKeyboardState;
    static std::vector<bool> m_previousKeyboardState;
    static MouseState m_currentMouseState;
    static MouseState m_previousMouseState;
};

} // namespace core
} // namespace beige
