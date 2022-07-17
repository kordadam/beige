#include "Input.hpp"

namespace beige {
namespace core {

std::shared_ptr<Input> Input::m_instance { new Input };
std::vector<bool> Input::m_currentKeyboardState(256, false);
std::vector<bool> Input::m_previousKeyboardState(256, false);

MouseState Input::m_currentMouseState {
    0,
    0,
    {
        { Button::Left, false },
        { Button::Middle, false },
        { Button::Right, false }
    }
};

MouseState Input::m_previousMouseState{
    m_currentMouseState
};

auto Input::getInstance() -> std::shared_ptr<Input> {
    return m_instance;
}

auto Input::update(const double deltaTime) -> void {
    m_previousKeyboardState = m_currentKeyboardState;
    m_previousMouseState = m_currentMouseState;
}

auto Input::isKeyDown(const Key key) -> bool {
    return m_currentKeyboardState.at(static_cast<uint64_t>(key));
}
auto Input::isKeyUp(const Key key) -> bool {
    return !m_currentKeyboardState.at(static_cast<uint64_t>(key));
}

auto Input::wasKeyDown(const Key key) -> bool {
    return m_previousKeyboardState.at(static_cast<uint64_t>(key));
}

auto Input::wasKeyUp(const Key key) -> bool {
    return !m_previousKeyboardState.at(static_cast<uint64_t>(key));
}

auto Input::processKey(const Key key, const bool isPressed) -> void {
    if (m_currentKeyboardState.at(static_cast<uint64_t>(key)) != isPressed) {
        m_currentKeyboardState.at(static_cast<uint64_t>(key)) = isPressed;
        KeyEvent::notifyListeners(isPressed ? KeyEventCode::Pressed : KeyEventCode::Released, key);
    }
}

auto Input::processButton(const Button button, const bool isPressed) -> void {
    if (m_currentMouseState.buttons.at(button) != isPressed) {
        m_currentMouseState.buttons.at(button) = isPressed;
        MouseEvent::notifyListeners(isPressed ? MouseEventCode::Pressed : MouseEventCode::Released, m_currentMouseState);
    }
}

auto Input::processMouseMove(const int32_t xPos, const int32_t yPos) -> void {
    if (m_currentMouseState.xPos != xPos || m_currentMouseState.yPos != yPos) {
        m_currentMouseState.xPos = xPos;
        m_currentMouseState.yPos = yPos;
        MouseEvent::notifyListeners(MouseEventCode::Moved, m_currentMouseState);
    }
}

auto Input::processMouseWheel(const int32_t deltaZ) -> void {

}

} // namespace core
} // namespace beige
