#include "thprac_utils.h"

namespace THPrac {

namespace TH06 {
    HOOKSET_DEFINE(THInitHook)
    static __declspec(noinline) void THGuiCreate()
    {
    }
    static __declspec(noinline) void THInitHookDisable()
    {
        auto& s = THInitHook::singleton();
        s.th06_gui_init_1.Disable();
        s.th06_gui_init_2.Disable();
    }
    EHOOK_DY(th06_gui_init_1, (void*)0x43596f)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    EHOOK_DY(th06_gui_init_2, (void*)0x42140c)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    HOOKSET_ENDDEF()
}

void TH06Init()
{
    TH06::THInitHook::singleton().EnableAllHooks();
    TryKeepUpRefreshRate((void*)0x420f59);
}

}