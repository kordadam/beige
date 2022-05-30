#pragma once

#include "../Defines.hpp"

#include <map>

namespace beige {
namespace core {

enum class Key : uint32_t {
    Backspace = 0x08,
    Enter = 0x0D,
    Tab = 0x09,
    Shift = 0x10,
    Control = 0x11,

    Pause = 0x13,
    Capital = 0x14,

    Escape = 0x1B,

    Convert = 0x1C,
    Nonconvert = 0x1D,
    Accept = 0x1E,
    Modechange = 0x1F,

    Space = 0x20,
    Prior = 0x21,
    Next = 0x22,
    End = 0x23,
    Home = 0x24,
    Left = 0x25,
    Up = 0x26,
    Right = 0x27,
    Down = 0x28,
    Select = 0x29,
    Print = 0x2A,
    Execute = 0x2B,
    Snapshot = 0x2C,
    Insert = 0x2D,
    Delete = 0x2E,
    Help = 0x2F,

    A = 0x41,
    B = 0x42,
    C = 0x43,
    D = 0x44,
    E = 0x45,
    F = 0x46,
    G = 0x47,
    H = 0x48,
    I = 0x49,
    J = 0x4A,
    K = 0x4B,
    L = 0x4C,
    M = 0x4D,
    N = 0x4E,
    O = 0x4F,
    P = 0x50,
    Q = 0x51,
    R = 0x52,
    S = 0x53,
    T = 0x54,
    U = 0x55,
    V = 0x56,
    W = 0x57,
    X = 0x58,
    Y = 0x59,
    Z = 0x5A,

    LWin = 0x5B,
    RWin = 0x5C,
    Apps = 0x5D,

    Sleep = 0x5F,

    Numpad0 = 0x60,
    Numpad1 = 0x61,
    Numpad2 = 0x62,
    Numpad3 = 0x63,
    Numpad4 = 0x64,
    Numpad5 = 0x65,
    Numpad6 = 0x66,
    Numpad7 = 0x67,
    Numpad8 = 0x68,
    Numpad9 = 0x69,
    Multiply = 0x6A,
    Add = 0x6B,
    Separator = 0x6C,
    Subtract = 0x6D,
    Decimal = 0x6E,
    Divide = 0x6F,

    F1 = 0x70,
    F2 = 0x71,
    F3 = 0x72,
    F4 = 0x73,
    F5 = 0x74,
    F6 = 0x75,
    F7 = 0x76,
    F8 = 0x77,
    F9 = 0x78,
    F10 = 0x79,
    F11 = 0x7A,
    F12 = 0x7B,
    F13 = 0x7C,
    F14 = 0x7D,
    F15 = 0x7E,
    F16 = 0x7F,
    F17 = 0x80,
    F18 = 0x81,
    F19 = 0x82,
    F20 = 0x83,
    F21 = 0x84,
    F22 = 0x85,
    F23 = 0x86,
    F24 = 0x87,

    Numlock = 0x90,
    Scroll = 0x91,

    NumpadEqual = 0x92,

    LShift = 0xA0,
    RShift = 0xA1,
    LControl = 0xA2,
    RControl = 0xA3,
    LMenu = 0xA4,
    RMenu = 0xA5,

    Semicolon = 0xBA,
    Plus = 0xBB,
    Coma = 0xBC,
    Minus = 0xBD,
    Period = 0xBE,
    Slash = 0xBF,
    Grave = 0xC0
};

enum class Button : uint32_t {
    Invalid,
    Left,
    Middle,
    Right
};

using KeyboardState = std::map<Key, bool>;

struct MouseState {
    int32_t xPos;
    int32_t yPos;
    std::map<Button, bool> buttons;
};

enum class KeyEventCode : uint32_t {
    Pressed,
    Released
};

enum class MouseEventCode : uint32_t {
    Pressed,
    Released,
    Moved,
    Wheel
};

} // namespace core
} // namespace beige
