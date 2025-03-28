#pragma once

#ifdef TOAST_EXPORTS
    #define TOAST_API __declspec(dllexport)
#else
#define TOAST_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

TOAST_API bool Initialize(const wchar_t* appName, const wchar_t* aumi);
TOAST_API bool Show(const wchar_t* message, const wchar_t* title = nullptr,
                    const wchar_t* message2 = nullptr, const wchar_t* imagePath = nullptr);


#ifdef __cplusplus
}
#endif
