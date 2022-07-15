#include "Input.hpp"

namespace beige {
namespace core {

std::shared_ptr<Input> Input::m_instance { new Input };

std::vector<bool> Input::m_currentKeyboardState(256, false);
//    { Key::Backspace, false },
//    { Key::Enter, false },
//    { Key::Tab, false },
//    { Key::Shift, false },
//    { Key::Control, false },
//    { Key::Pause, false },
//    { Key::Capital, false },
//    { Key::Escape, false },
//    { Key::Convert, false },
//    { Key::Nonconvert, false },
//    { Key::Accept, false },
//    { Key::Modechange, false },
//    { Key::Space, false },
//    { Key::Prior, false },
//    { Key::Next, false },
//    { Key::End, false },
//    { Key::Home, false },
//    { Key::Left, false },
//    { Key::Up, false },
//    { Key::Right, false },
//    { Key::Down, false },
//    { Key::Select, false },
//    { Key::Print, false },
//    { Key::Execute, false },
//    { Key::Snapshot, false },
//    { Key::Insert, false },
//    { Key::Delete, false },
//    { Key::Help, false },
//    { Key::A, false },
//    { Key::B, false },
//    { Key::C, false },
//    { Key::D, false },
//    { Key::E, false },
//    { Key::F, false },
//    { Key::G, false },
//    { Key::H, false },
//    { Key::I, false },
//    { Key::J, false },
//    { Key::K, false },
//    { Key::L, false },
//    { Key::M, false },
//    { Key::N, false },
//    { Key::O, false },
//    { Key::P, false },
//    { Key::Q, false },
//    { Key::R, false },
//    { Key::S, false },
//    { Key::T, false },
//    { Key::U, false },
//    { Key::V, false },
//    { Key::W, false },
//    { Key::X, false },
//    { Key::Y, false },
//    { Key::Z, false },
//    { Key::LWin, false },
//    { Key::RWin, false },
//    { Key::Apps, false },
//    { Key::Sleep, false },
//    { Key::Numpad0, false },
//    { Key::Numpad1, false },
//    { Key::Numpad2, false },
//    { Key::Numpad3, false },
//    { Key::Numpad4, false },
//    { Key::Numpad5, false },
//    { Key::Numpad6, false },
//    { Key::Numpad7, false },
//    { Key::Numpad8, false },
//    { Key::Numpad9, false },
//    { Key::Multiply, false },
//    { Key::Add, false },
//    { Key::Separator, false },
//    { Key::Subtract, false },
//    { Key::Decimal, false },
//    { Key::Divide, false },
//    { Key::F1, false },
//    { Key::F2, false },
//    { Key::F3, false },
//    { Key::F4, false },
//    { Key::F5, false },
//    { Key::F6, false },
//    { Key::F7, false },
//    { Key::F8, false },
//    { Key::F9, false },
//    { Key::F10, false },
//    { Key::F11, false },
//    { Key::F12, false },
//    { Key::F13, false },
//    { Key::F14, false },
//    { Key::F15, false },
//    { Key::F16, false },
//    { Key::F17, false },
//    { Key::F18, false },
//    { Key::F19, false },
//    { Key::F20, false },
//    { Key::F21, false },
//    { Key::F22, false },
//    { Key::F23, false },
//    { Key::F24, false },
//    { Key::Numlock, false },
//    { Key::Scroll, false },
//    { Key::NumpadEqual, false },
//    { Key::LShift, false },
//    { Key::RShift, false },
//    { Key::LControl, false },
//    { Key::RControl, false },
//    { Key::LMenu, false },
//    { Key::RMenu, false },
//    { Key::Semicolon, false },
//    { Key::Plus, false },
//    { Key::Coma, false },
//    { Key::Minus, false },
//    { Key::Period, false },
//    { Key::Slash, false },
//    { Key::Grave, false }
//};

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
    m_currentKeyboardState = m_previousKeyboardState;
    m_currentMouseState = m_previousMouseState;
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
