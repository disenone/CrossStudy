#pragma once

#if defined(_WIN32)
#ifdef cimageWin_EXPORTS
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif
#else
#define EXPORT
#endif