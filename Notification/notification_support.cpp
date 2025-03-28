module;

#include <string>
#include <optional>
#include <windows.h>
#include <shobjidl.h>
#include <wrl.h>
#include <windows.ui.notifications.h>
#include <roapi.h>

using namespace Microsoft::WRL;
using namespace ABI::Windows::UI::Notifications;
using namespace ABI::Windows::Foundation;

module toast.notification_support;
import toast.types;

namespace toast
{
    bool NotificationSupport::isSupported()
    {
        return isWindows10OrGreater() && isEnabled();
    }

    bool NotificationSupport::isEnabled()
    {
        try
        {
            HKEY hKey;
            if (RegOpenKeyEx(HKEY_CURRENT_USER,
                             RegistryKeys::PushNotifications::PATH,
                             0,
                             KEY_READ,
                             &hKey) != ERROR_SUCCESS)
            {
                return true;
            }

            DWORD value = 1;
            DWORD dataSize = sizeof(DWORD);
            DWORD type = REG_DWORD;

            if (RegQueryValueEx(hKey,
                                RegistryKeys::PushNotifications::TOAST_ENABLED,
                                nullptr,
                                &type,
                                reinterpret_cast<LPBYTE>(&value),
                                &dataSize) != ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
                return true;
            }

            RegCloseKey(hKey);
            return value != 0;
        }
        catch (...)
        {
            return true; // 예외 발생시 기본값으로 true 반환
        }
    }

    bool NotificationSupport::isWindows10OrGreater()
    {
        OSVERSIONINFOEXW osvi = {sizeof(osvi)};

        auto RtlGetVersion = reinterpret_cast<LONG(WINAPI*)(PRTL_OSVERSIONINFOW)>(
            GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetVersion"));

        if (RtlGetVersion != nullptr)
        {
            RtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osvi));
            return osvi.dwMajorVersion >= 10;
        }
        return false;
    }
}
