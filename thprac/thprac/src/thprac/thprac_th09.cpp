#include "thprac_utils.h"

namespace THPrac {
namespace TH09 {
    HOOKSET_DEFINE(THInitHook)
    static __declspec(noinline) void THGuiCreate()
    {
    }
    static __declspec(noinline) void THInitHookDisable()
    {
        auto& s = THInitHook::singleton();
        s.th09_gui_init_1.Disable();
        s.th09_gui_init_2.Disable();
    }
    EHOOK_DY(th09_gui_init_1, (void*)0x42a0c4)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    EHOOK_DY(th09_gui_init_2, (void*)0x42e3a8)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    HOOKSET_ENDDEF()
}

void TH09Init()
{
    TH09::THInitHook::singleton().EnableAllHooks();
    TryKeepUpRefreshRate((void*)0x42de70);
}
}