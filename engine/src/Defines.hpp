#pragma once

#pragma warning(disable:4251)

#define BEIGE_PLATFORM_WIN32

#ifdef BEIGE_EXPORT
#define BEIGE_API __declspec(dllexport)
#else
#define BEIGE_API __declspec(dllimport)
#endif // BEIGE_EXPORT

#define debugBreak() __debugbreak()

#define STATIC_ASSERT static_assert