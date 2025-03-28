module;

#include <memory>
#include <optional>
#include <string>
#include <Windows.UI.Notifications.h>

using namespace ABI::Windows::UI::Notifications;

export module toast;

export namespace toast
{
    struct ToastContentOptions
    {
        const wchar_t* title = nullptr;
        const wchar_t* message = nullptr;
        const wchar_t* message2 = nullptr;
        const wchar_t* imagePath = nullptr;
    };

    class IToast
    {
    public:
        static std::optional<std::unique_ptr<IToast>> Create();

        virtual void Initialize(const std::wstring& appName, const std::wstring& aumi) = 0;
        virtual void Show(ToastTemplateType type, const ToastContentOptions& options) = 0;

        virtual ~IToast() = default;
    };
}
