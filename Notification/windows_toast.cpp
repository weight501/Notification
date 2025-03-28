module;
#include <string>
#include <optional>
#include <windows.h>
#include <shobjidl.h>
#include <wrl.h>
#include <wrl/client.h>
#include <windows.data.xml.dom.h>
#include <windows.ui.notifications.h>
#include <Shlobj.h>
#include <propkey.h>
#include <wrl/wrappers/corewrappers.h>

module toast.windows;

import toast;
import toast.types;
import toast.exception;
import toast.notification_support;
import toast.hstring_wrapper;

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::UI::Notifications;

namespace toast
{
    std::optional<std::unique_ptr<IToast>> WindowsToast::Create()
    {
        if (!NotificationSupport::isSupported())
        {
            return std::nullopt;
        }

        try
        {
            return std::unique_ptr<IToast>(new WindowsToast());
        }
        catch (const ToastException&)
        {
            return std::nullopt;
        }
    }

    WindowsToast::WindowsToast()
    {
    }

    WindowsToast::~WindowsToast()
    {
        if (m_isInitialized)
        {
            RoUninitialize();
            if (m_hasCoInitialized)
            {
                CoUninitialize();
            }
        }
    }

    void WindowsToast::Initialize(const std::wstring& appName, const std::wstring& aumi)
    {
        if (m_isInitialized)
        {
            throw ToastException("Already initialized");
        }

        m_appName = appName;
        m_aumi = aumi;

        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(hr))
        {
            throw ToastException("COM initialization failed", hr);
        }
        m_hasCoInitialized = true;

        hr = SetCurrentProcessExplicitAppUserModelID(m_aumi.c_str());
        if (FAILED(hr))
        {
            if (m_hasCoInitialized)
            {
                CoUninitialize();
            }
            throw ToastException("Failed to set AppUserModelID", hr);
        }

        ShortcutResult result = createShortcut();
        if (result == ShortcutResult::SHORTCUT_CREATE_FAILED)
        {
            if (m_hasCoInitialized)
            {
                CoUninitialize();
            }
            throw ToastException("Failed to create shortcut");
        }

        hr = RoInitialize(RO_INIT_SINGLETHREADED);
        if (FAILED(hr))
        {
            if (m_hasCoInitialized)
            {
                CoUninitialize();
            }
            throw ToastException("Windows Runtime initialization failed", hr);
        }

        m_isInitialized = true;
    }

    ComPtr<IToastNotificationManagerStatics> WindowsToast::CreateToastManager()
    {
        ComPtr<IToastNotificationManagerStatics> toastManager;

        HStringWrapper className(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager);

        HRESULT hr = RoGetActivationFactory(
            className,
            __uuidof(IToastNotificationManagerStatics),
            reinterpret_cast<void**>(toastManager.GetAddressOf()));

        if (FAILED(hr))
        {
            throw ToastException("Failed to create toast manager string", hr);
        }

        hr = RoGetActivationFactory(
            className,
            __uuidof(IToastNotificationManagerStatics),
            reinterpret_cast<void**>(toastManager.GetAddressOf()));
        WindowsDeleteString(className);
        if (FAILED(hr))
        {
            throw ToastException("Failed to create toast manager", hr);
        }

        return toastManager;
    }

    ComPtr<IToastNotification> WindowsToast::CreateNotification(IXmlDocument* xmlDoc)
    {
        ComPtr<IToastNotificationFactory> factory;
        HStringWrapper toastNotificationStr(RuntimeClass_Windows_UI_Notifications_ToastNotification);

        HRESULT hr = RoGetActivationFactory(
            toastNotificationStr,
            __uuidof(IToastNotificationFactory),
            reinterpret_cast<void**>(factory.GetAddressOf()));

        if (FAILED(hr))
        {
            throw ToastException("Failed to create notification factory", hr);
        }

        ComPtr<IToastNotification> notification;
        hr = factory->CreateToastNotification(xmlDoc, &notification);
        if (FAILED(hr))
        {
            throw ToastException("Failed to create notification", hr);
        }

        return notification;
    }

    ComPtr<IToastNotifier> WindowsToast::CreateNotifier(IToastNotificationManagerStatics* toastManager)
    {
        ComPtr<IToastNotifier> notifier;
        HStringWrapper appIdStr(m_aumi.c_str());

        HRESULT hr = toastManager->CreateToastNotifierWithId(appIdStr, &notifier);
        if (FAILED(hr))
        {
            throw ToastException("Failed to create notifier", hr);
        }

        return notifier;
    }

    ComPtr<IXmlDocument> WindowsToast::GetTemplateContent(
        IToastNotificationManagerStatics* toastManager, ToastTemplateType type)
    {
        ComPtr<IXmlDocument> xmlDoc;
        HRESULT hr = toastManager->GetTemplateContent(type, &xmlDoc);
        if (FAILED(hr))
        {
            throw ToastException("Failed to get template content", hr);
        }
        return xmlDoc;
    }

    void WindowsToast::Show(ToastTemplateType type, const ToastContentOptions& options)
    {
        if (!m_isInitialized)
        {
            throw ToastException("Not initialized");
        }

        auto toastManager = CreateToastManager();
        auto xmlDoc = GetTemplateContent(toastManager.Get(), type);
        SetToastContent(xmlDoc.Get(), options, type);

        auto notification = CreateNotification(xmlDoc.Get());
        auto notifier = CreateNotifier(toastManager.Get());

        HRESULT hr = notifier->Show(notification.Get());
        if (FAILED(hr))
        {
            throw ToastException("Failed to show toast", hr);
        }
    }

    ShortcutResult WindowsToast::createShortcut()
    {
        if (!m_aumi.c_str() || !m_appName.c_str())
        {
            return ShortcutResult::SHORTCUT_MISSING_PARAMETERS;
        }

        if (!m_hasCoInitialized)
        {
            HRESULT initHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            if (initHr != RPC_E_CHANGED_MODE)
            {
                if (FAILED(initHr) && initHr != S_FALSE)
                {
                    return ShortcutResult::SHORTCUT_COM_INIT_FAILURE;
                }
                m_hasCoInitialized = true;
            }
        }

        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);

        PWSTR programsPath = nullptr;
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_Programs, 0, nullptr, &programsPath);
        if (FAILED(hr)) return ShortcutResult::SHORTCUT_CREATE_FAILED;

        auto shortcutPath = std::wstring(programsPath);
        shortcutPath.append(L"\\").append(m_appName).append(L".lnk");
        CoTaskMemFree(programsPath);

        ComPtr<IShellLinkW> shellLink;
        hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&shellLink));
        if (FAILED(hr)) return ShortcutResult::SHORTCUT_CREATE_FAILED;

        hr = shellLink->SetPath(exePath);
        if (FAILED(hr)) return ShortcutResult::SHORTCUT_CREATE_FAILED;

        auto workingDir = std::wstring(exePath);
        size_t lastSlash = workingDir.find_last_of(L"\\");
        if (lastSlash != std::wstring::npos)
        {
            workingDir = workingDir.substr(0, lastSlash);
            hr = shellLink->SetWorkingDirectory(workingDir.c_str());
            if (FAILED(hr)) return ShortcutResult::SHORTCUT_CREATE_FAILED;
        }

        ComPtr<IPropertyStore> propertyStore;
        hr = shellLink.As(&propertyStore);
        if (FAILED(hr)) return ShortcutResult::SHORTCUT_CREATE_FAILED;

        PROPVARIANT propVar;
        PropVariantInit(&propVar);

        try
        {
            propVar.vt = VT_LPWSTR;
            propVar.pwszVal = const_cast<LPWSTR>(m_aumi.c_str());

            hr = propertyStore->SetValue(PKEY_AppUserModel_ID, propVar);
            if (SUCCEEDED(hr))
            {
                hr = propertyStore->Commit();
            }
        }
        catch (...)
        {
            PropVariantClear(&propVar);
            throw;
        }

        if (SUCCEEDED(hr))
        {
            ComPtr<IPersistFile> persistFile;
            hr = shellLink.As(&persistFile);
            if (SUCCEEDED(hr))
            {
                hr = persistFile->Save(shortcutPath.c_str(), TRUE);
            }
        }

        return SUCCEEDED(hr)
                   ? ShortcutResult::SHORTCUT_WAS_CREATED
                   : ShortcutResult::SHORTCUT_CREATE_FAILED;
    }

    void WindowsToast::SetNodeStringValue(IXmlNode* node, const wchar_t* value)
    {
        if (node == nullptr)
        {
            throw ToastException("Null node pointer", E_INVALIDARG);
        }

        HStringWrapper valueString(value);

        ComPtr<IXmlNode> stringNode;
        ComPtr<IXmlDocument> xmlDocument;
        ComPtr<IXmlText> textNode;

        HRESULT hr = node->get_OwnerDocument(&xmlDocument);
        if (FAILED(hr))
        {
            throw ToastException("Failed to get owner document", hr);
        }

        hr = xmlDocument->CreateTextNode(valueString, &textNode);
        if (SUCCEEDED(hr))
        {
            hr = textNode.As(&stringNode);
        }

        if (FAILED(hr))
        {
            throw ToastException("Failed to create text node", hr);
        }

        ComPtr<IXmlNode> childNode;
        while (SUCCEEDED(node->get_FirstChild(&childNode)) && childNode != nullptr)
        {
            ComPtr<IXmlNode> tempNode;
            node->RemoveChild(childNode.Get(), &tempNode);
            childNode = nullptr;
        }

        ComPtr<IXmlNode> appendedNode;
        hr = node->AppendChild(stringNode.Get(), &appendedNode);

        if (FAILED(hr))
        {
            throw ToastException("Failed to append child", hr);
        }
    }

    void WindowsToast::SetToastContent(IXmlDocument* xml, const ToastContentOptions& options, ToastTemplateType type)
    {
        HStringWrapper textNodeName(L"text");
        HStringWrapper imageNodeName(L"image");

        ComPtr<IXmlNodeList> textNodeList;
        ComPtr<IXmlNodeList> imageNodeList;

        HRESULT hr = xml->GetElementsByTagName(textNodeName, &textNodeList);
        if (FAILED(hr))
        {
            WindowsDeleteString(textNodeName);
            throw ToastException("Failed to get text elements", hr);
        }

        if (type <= ToastTemplateType_ToastImageAndText04)
        {
            hr = xml->GetElementsByTagName(imageNodeName, &imageNodeList);
            if (FAILED(hr))
            {
                throw ToastException("Failed to get image elements", hr);
            }

            SetToastImage(imageNodeList.Get(), options.imagePath);
        }

        switch (type)
        {
        case ToastTemplateType_ToastImageAndText01:
        case ToastTemplateType_ToastText01:
            SetSingleLineText(textNodeList.Get(), options.message);
            break;

        case ToastTemplateType_ToastImageAndText02:
        case ToastTemplateType_ToastText02:
            SetTitleAndMessage(textNodeList.Get(), options.title, options.message);
            break;

        case ToastTemplateType_ToastImageAndText03:
        case ToastTemplateType_ToastText03:
            SetTitleAndMessage(textNodeList.Get(), options.title, options.message);
            break;

        case ToastTemplateType_ToastImageAndText04:
        case ToastTemplateType_ToastText04:
            SetTitleAndTwoLineMessage(textNodeList.Get(), options.title, options.message, options.message2);
            break;
        }

        WindowsDeleteString(textNodeName);
        WindowsDeleteString(imageNodeName);
    }

    bool WindowsToast::SetTextForNode(IXmlNodeList* nodeList, UINT index, const wchar_t* text)
    {
        ComPtr<IXmlNode> node;
        HRESULT hr = nodeList->Item(index, &node);
        if (SUCCEEDED(hr) && node != nullptr)
        {
            SetNodeStringValue(node.Get(), text);
            return true;
        }
        return false;
    }

    void WindowsToast::SetSingleLineText(IXmlNodeList* nodeList, const wchar_t* text)
    {
        SetTextForNode(nodeList, 0, text);
    }

    void WindowsToast::SetTitleAndMessage(IXmlNodeList* nodeList,
                                          const wchar_t* title, const wchar_t* message)
    {
        SetTextForNode(nodeList, 0, title);
        SetTextForNode(nodeList, 1, message);
    }

    void WindowsToast::SetTitleAndTwoLineMessage(IXmlNodeList* nodeList, const wchar_t* title, const wchar_t* message1,
                                                 const wchar_t* message2)
    {
        SetTextForNode(nodeList, 0, title);
        SetTextForNode(nodeList, 1, message1);
        SetTextForNode(nodeList, 2, message2);
    }

    void WindowsToast::SetToastImage(IXmlNodeList* nodeList, const wchar_t* imagePath)
    {
        ComPtr<IXmlNode> imageNode;
        HRESULT hr = nodeList->Item(0, &imageNode);
        if (SUCCEEDED(hr))
        {
            ComPtr<IXmlNamedNodeMap> attributes;
            hr = imageNode->get_Attributes(&attributes);
            if (SUCCEEDED(hr))
            {
                ComPtr<IXmlNode> srcAttribute;
                HStringWrapper srcName(L"src");

                if (SUCCEEDED(hr))
                {
                    hr = attributes->GetNamedItem(srcName, &srcAttribute);
                    if (SUCCEEDED(hr))
                    {
                        SetNodeStringValue(srcAttribute.Get(), imagePath);
                    }
                }
            }
        }
    }

    void WindowsToast::DebugXmlContent(IXmlDocument* xmlDoc)
    {
        ComPtr<IXmlNodeSerializer> serializer;
        HRESULT hr = xmlDoc->QueryInterface(IID_PPV_ARGS(&serializer));
        if (SUCCEEDED(hr))
        {
            HSTRING xmlString;
            hr = serializer->GetXml(&xmlString);
            if (SUCCEEDED(hr))
            {
                UINT32 length;
                const wchar_t* xmlContent = WindowsGetStringRawBuffer(xmlString, &length);
                MessageBoxW(nullptr, xmlContent, L"Toast XML Content", MB_OK);
                WindowsDeleteString(xmlString);
            }
        }
    }
}
