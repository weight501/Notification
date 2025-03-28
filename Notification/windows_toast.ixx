module;
#pragma comment(lib, "runtimeobject.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "windowsapp.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Propsys.lib")

#include <memory>
#include <optional>
#include <string>
#include <windows.data.xml.dom.h>
#include <windows.ui.notifications.h>
#include <wrl/client.h>

export module toast.windows;

import toast;
import toast.types;
import toast.exception;
import toast.notification_support;

using namespace ABI::Windows::Data::Xml::Dom;
using namespace ABI::Windows::UI::Notifications;
using namespace Microsoft::WRL;

export namespace toast
{
    class WindowsToast : public IToast
    {
    public:
        static std::optional<std::unique_ptr<IToast>> Create();
        void Show(ToastTemplateType type, const ToastContentOptions& options) override;
        void Initialize(const std::wstring& appName, const std::wstring& aumi) override;
        ~WindowsToast() override;

    private:
        WindowsToast();

        std::wstring m_aumi{Config::DEFAULT_AUMI};
        std::wstring m_appName{Config::DEFAULT_APP_NAME};

        bool m_isInitialized{false};
        bool m_hasCoInitialized{false};

        ComPtr<IToastNotificationManagerStatics> CreateToastManager();
        ComPtr<IXmlDocument> GetTemplateContent(IToastNotificationManagerStatics* toastManager, ToastTemplateType type);
        ComPtr<IToastNotification> CreateNotification(IXmlDocument* xmlDoc);
        ComPtr<IToastNotifier> CreateNotifier(IToastNotificationManagerStatics* toastManager);

        ShortcutResult createShortcut();

        bool SetTextForNode(IXmlNodeList* nodeList, UINT index, const wchar_t* text);
        void SetNodeStringValue(IXmlNode* node, const wchar_t* value);
        void DebugXmlContent(IXmlDocument* xmlDoc);

        void SetToastContent(IXmlDocument* xml, const ToastContentOptions& options, ToastTemplateType type);

        void SetSingleLineText(IXmlNodeList* nodeList, const wchar_t* text);
        void SetTitleAndMessage(IXmlNodeList* nodeList, const wchar_t* title, const wchar_t* message);
        void SetTitleAndTwoLineMessage(IXmlNodeList* nodeList, const wchar_t* title, const wchar_t* message1,
                                       const wchar_t* message2);

        void SetToastImage(IXmlNodeList* nodeList, const wchar_t* imagePath);
    };
}
