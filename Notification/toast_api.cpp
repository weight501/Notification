#include "toast_api.h"

#include <memory>
#include <optional>
#include <Windows.UI.Notifications.h>

using namespace ABI::Windows::UI::Notifications;

import toast;

namespace
{
    std::optional<std::unique_ptr<toast::IToast>> g_toast;
}

bool Initialize(const wchar_t* appName, const wchar_t* aumi)
{
    try
    {
        if (!g_toast)
        {
            g_toast = toast::IToast::Create();
            if (!g_toast)
            {
                return false;
            }
        }

        (*g_toast)->Initialize(appName, aumi);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool Show(const wchar_t* message, const wchar_t* title, const wchar_t* message2, const wchar_t* imagePath)
{
    try
    {
        if (!g_toast || !message)
        {
            return false;
        }

        toast::ToastContentOptions options;
        options.message = message;

        ToastTemplateType templateType;

        if (imagePath)
        {
            options.imagePath = imagePath;
            if (title != nullptr && message2 != nullptr)
            {
                options.title = title;
                options.message2 = message2;
                templateType = ToastTemplateType_ToastImageAndText04;
            }
            else if (title != nullptr)
            {
                options.title = title;
                templateType = ToastTemplateType_ToastImageAndText02;
            }
            else
            {
                templateType = ToastTemplateType_ToastImageAndText01;
            }
        }
        else
        {
            if (title != nullptr && message2 != nullptr)
            {
                options.title = title;
                options.message2 = message2;
                templateType = ToastTemplateType_ToastText04;
            }
            else if (title != nullptr)
            {
                options.title = title;
                templateType = ToastTemplateType_ToastText02;
            }
            else
            {
                templateType = ToastTemplateType_ToastText01;
            }
        }

        (*g_toast)->Show(templateType, options);

        return true;
    }
    catch (...)
    {
        return false;
    }
}
