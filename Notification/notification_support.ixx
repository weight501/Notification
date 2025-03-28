module;

export module toast.notification_support;
import toast.types;

export namespace toast
{
    class NotificationSupport
    {
    public:
        static bool isSupported();
        static bool isEnabled();
        static bool isWindows10OrGreater();
    };
}
