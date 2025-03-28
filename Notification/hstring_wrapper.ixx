// hstring_wrapper.ixx
module;
#include <windows.h>
#include <roapi.h>
#include <winstring.h>

export module toast.hstring_wrapper;
import toast.exception;

export namespace toast
{
    class HStringWrapper
    {
    public:
        explicit HStringWrapper(const wchar_t* str)
        {
            auto length = str ? static_cast<UINT32>(wcslen(str)) : 0;
            auto hr = WindowsCreateString(str, length, &m_hstring);
            if (FAILED(hr))
            {
                throw ToastException("Failed to create HSTRING", hr);
            }
        }

        ~HStringWrapper()
        {
            if (m_hstring)
            {
                WindowsDeleteString(m_hstring);
            }
        }

        HSTRING Get() const { return m_hstring; }

        operator HSTRING() const { return m_hstring; }

    private:
        HSTRING m_hstring = nullptr;

        HStringWrapper(const HStringWrapper&) = delete;
        HStringWrapper& operator=(const HStringWrapper&) = delete;
    };
}
