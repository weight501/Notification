module;

import toast;
import toast.windows;

module toast;
#include <memory>
#include <optional>

namespace toast
{
    std::optional<std::unique_ptr<IToast>> IToast::Create()
    {
        return WindowsToast::Create();
    }
}
