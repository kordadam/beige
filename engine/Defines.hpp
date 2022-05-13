#pragma once

#ifdef BEIGE_EXPORT
#define BEIGE_API __declspec(dllexport)
#else
#define BEIGE_API __declspec(dllimport)
#endif // BEIGE_EXPORT
