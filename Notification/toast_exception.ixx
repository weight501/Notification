module;
#include <stdexcept>
#include <string>
#include <windows.h>

export module toast.exception;

export namespace toast
{
    class ToastException : public std::runtime_error
    {
    public:
        explicit ToastException(const char* message, HRESULT hr = 0)
            : std::runtime_error(FormatMessage(message, hr)), m_hr(hr)
        {
        }

        HRESULT GetHResult() const { return m_hr; }

    private:
        HRESULT m_hr;

        static std::string FormatMessage(const char* message, HRESULT hr)
        {
            if (hr == 0)
            {
                return message;
            }

            char buffer[256];
            sprintf_s(buffer, "%s (HRESULT: 0x%08X)", message, hr);

            return buffer;
        }
    };
}
