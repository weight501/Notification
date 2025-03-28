module;
#include <windows.h>

export module toast.types;

export namespace toast
{
    struct Config
    {
        static constexpr auto DEFAULT_APP_NAME = L"test_notification";
        static constexpr auto DEFAULT_AUMI = L"test_notification";
    };

    struct RegistryKeys
    {
        struct PushNotifications
        {
            static constexpr auto PATH = L"Software\\Microsoft\\Windows\\CurrentVersion\\PushNotifications";
            static constexpr auto TOAST_ENABLED = L"ToastEnabled";
        };
    };

    enum class Result
    {
        Success = 0,
        Created = 1,
        Changed = 2,

        Error_MissingParameters = -1,
        Error_IncompatibleOS = -2,
        Error_ComInitFailed = -3,
        Error_CreateFailed = -4
    };

    enum class ShortcutResult
    {
        SHORTCUT_UNCHANGED = 0,
        SHORTCUT_WAS_CHANGED = 1,
        SHORTCUT_WAS_CREATED = 2,

        SHORTCUT_MISSING_PARAMETERS = -1,
        SHORTCUT_INCOMPATIBLE_OS = -2,
        SHORTCUT_COM_INIT_FAILURE = -3,
        SHORTCUT_CREATE_FAILED = -4
    };
}
