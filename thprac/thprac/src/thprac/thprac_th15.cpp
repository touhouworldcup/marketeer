#include "thprac_utils.h"

namespace THPrac {
namespace TH15 {
    void THDataInit()
    {
        AnlyDataInit();

        DataRef<DATA_SCENE_ID>(U32_ARG(0x4e7ecc));
        DataRef<DATA_RND_SEED>(U16_ARG(0x4e9a48));
        DataRef<DATA_DIFFCULTY>(U8_ARG(0x4e7410));
        DataRef<DATA_SHOT_TYPE>(U8_ARG(0x4e7404));
        //DataRef<DATA_SUB_SHOT_TYPE>(U8_ARG(0x4a57ac));
        DataRef<DATA_STAGE>(U8_ARG(0x4e73f0));
        DataRef<DATA_STARTING_STAGE>(U8_ARG(0x4e73f4));
    }


    HOOKSET_DEFINE(THMarketeerHook)
    EHOOK_DY(mkt_skip_1, (void*)0x461d02)
    {
        DWORD skipAddress = 0x461fa6;
        DWORD enterAddress = 0x461d12;
        if (MarketeerGetStatus() == 2) {
            auto c = GetMemContent(0x4e9be0, 0x24);
            if (c > 1 && c < 4) {
                pCtx->Eip = skipAddress;
            }
        } else if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                if (sig->GetCmd() == MARKETEER_NEW_GAME) {
                    *(uint32_t*)GetMemAddr(0x4e9be0, 0x24) = 0;
                    DataRef<MARKETEER_LIMIT_LOCK>() = 0;
                    MarketeerSetCaption(MKT_CAPTION_ACCEPTING);
                    pCtx->Eip = enterAddress;
                } else {
                    MarketeerAdvCursor();
                    pCtx->Eip = skipAddress;
                }
            } else {
                MarketeerSetCaption(MKT_CAPTION_WAITING_SIGNAL);
                pCtx->Eip = skipAddress;
            }
        }
    }
    EHOOK_DY(mkt_stage_start, (void*)0x45cf20)
    {
        if (MarketeerGetStatus() == 2) {
            uint32_t stage = DataRef<DATA_STAGE>();
            uint32_t startingStage = DataRef<DATA_STARTING_STAGE>();
            uint32_t player = DataRef<DATA_SHOT_TYPE>();
            uint32_t rank = DataRef<DATA_DIFFCULTY>();
            uint16_t seed = U16_REF(DATA_RND_SEED);
            uint32_t playerX = 0;
            uint32_t playerY = 0;
            MarketeerOnStageStart(stage == startingStage, stage, rank, player, seed, playerX, playerY);

        } else if (MarketeerGetStatus() == 1) {
            StreamSignal* sig;
            uint16_t outputBuf[3];
            for (sig = MarketeerGetCursor(); DataRef<MARKETEER_LIMIT_LOCK>() || !sig; sig = MarketeerGetCursor()) {
                MarketeerFreezeGame();
            }
            auto cmd = sig->GetCmd();
            if (cmd == MARKETEER_NEW_GAME || cmd == MARKETEER_NEW_STAGE) {
                U16_REF(DATA_RND_SEED) = (uint16_t)sig->param3;
                DataRef<MARKETEER_FRAME_REC_1>() = 0;
                MarketeerAlignStream(sig);
                while (!MarketeerReadStream(0, 6, outputBuf)) {
                    MarketeerFreezeGame();
                }
                MarketeerAdvCursor();
            }
        }
    }
    EHOOK_DY(mkt_game_input, (void*)0x45c066)
    {
        if (MarketeerGetStatus() == 2) {
            uint16_t inputBuf[3];
            inputBuf[0] = *(uint16_t*)0x4e6f28;
            inputBuf[1] = *(uint16_t*)0x4e6f34;
            inputBuf[2] = *(uint16_t*)0x4e6f38;
            MarketeerPush(inputBuf, 6);
        } else if (MarketeerGetStatus() == 1) {
            uint16_t outputBuf[3];
            auto cmd = MarketeerInputHelper(outputBuf, 6);
            if (cmd == MARKETEER_NEW_GAME) {
                U32_REF(DATA_SCENE_ID) = 10;
            } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                U32_REF(DATA_SCENE_ID) = 4;
            }
            *(uint16_t*)0x4e6f28 = outputBuf[0];
            *(uint16_t*)0x4e6f34 = outputBuf[1];
            *(uint16_t*)0x4e6f38 = outputBuf[2];
        }
    }
    EHOOK_DY(mkt_next_stage_start, (void*)0x439151)
    {
        if (MarketeerGetStatus() == 2) {
            auto stage = DataRef<DATA_STAGE>();
            if (stage == 6 || stage == 7) {
                MarketeerOnGameComlete();
            }
        }
    }
    EHOOK_DY(mkt_on_scene_switch, (void*)0x44e615)
    {
        if (MarketeerGetStatus() == 2) {
            if (*(uint32_t*)(pCtx->Esi + 0x6fc) == 4 && *(uint32_t*)(pCtx->Esi + 0x6f8) == 7) {
                MarketeerOnQuitToMenu();
            }
        } else if (MarketeerGetStatus() == 1) {
            if (*(uint32_t*)(pCtx->Esi + 0x6fc) == 15) {
                *(uint32_t*)(pCtx->Esi + 0x6fc) = pCtx->Eax = 4;
            }
        }
    }
    EHOOK_DY(mkt_on_continue, (void*)0x45283f)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerPushSignal(MARKETEER_CONTINUE);
        }
    }
    EHOOK_DY(mkt_on_quit, (void*)0x471eaa)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerEndLive();
        } else if (MarketeerGetStatus() == 1) {
            MarketeerTerminate();
        }
    }
    EHOOK_DY(mkt_force_legacy, (void*)0x466258)
    {
        if (MarketeerGetStatus()) {
            auto c = GetMemContent(0x4e9be0, 0x24);
            if (c == 0) {
                pCtx->Eip = 0x46628a;
            } else {
                pCtx->Eip = 0x466294;
            }
        } 
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THMarketeerViewerHook)
    EHOOK_DY(mkt_disable_input, (void*)0x401b20)
    {
        if (MarketeerGetStatus() == 1) {
            pCtx->Eax = 0;
            pCtx->Eip = 0x401cae;
        }
    }
    PATCH_DY(mkt_skip_2, (void*)0x4662fc, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_3, (void*)0x466953, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_4, (void*)0x467202, "\x90\x90\x90\x90\x90\x90", 6);
    EHOOK_DY(mkt_enter, (void*)0x46706b)
    {
        auto sig = MarketeerGetCursor();
        if (sig && sig->GetCmd() == MARKETEER_NEW_GAME) {
            uint32_t stage = (sig->param1 & 0x0000ff00) >> 8;
            uint32_t rank = (sig->param1 & 0x00ff0000) >> 16;
            uint32_t player = ((sig->param1 & 0xff000000) >> 24);
            U32_REF(DATA_STAGE) = stage;
            U32_REF(DATA_STARTING_STAGE) = stage;
            U32_REF(DATA_SCENE_ID) = 7;
            *(uint32_t*)0x4e9bd8 = 0x4e3f50+ stage * 0xd4;
            U32_REF(DATA_DIFFCULTY) = rank;
            U32_REF(DATA_SHOT_TYPE) = player;
        }
    }
    EHOOK_DY(mkt_next_stage_end, (void*)0x439156)
    {
        if (MarketeerGetStatus() == 1) {
            StreamSignal* sig;
            for (sig = MarketeerGetCursor(); !sig; sig = MarketeerGetCursor()) {
                MarketeerSetCaption(MKT_CAPTION_FETCHING);
                MarketeerFreezeGame();
            }
            auto cmd = sig->GetCmd();
            if (cmd == MARKETEER_NEW_STAGE) {
                MarketeerSetStreamLimit(sig);
                DataRef<MARKETEER_LIMIT_LOCK>() = 1;
            } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                MarketeerSetStreamLimit(nullptr);
            } else if (cmd == MARKETEER_GAME_COMPLETE) {
                U32_REF(DATA_SCENE_ID) = 4;
            }
        }
    }
    EHOOK_DY(mkt_intermission, (void*)0x43c5ad)
    {
        if (MarketeerGetStatus() == 1) {
            StreamSignal* sig;
            for (sig = MarketeerGetCursor(); DataRef<MARKETEER_LIMIT_LOCK>() || !sig; sig = MarketeerGetCursor()) {
                Sleep(1);
            }
        }
    }
    EHOOK_DY(mkt_on_continue_1, (void*)0x4519c0)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_CONTINUE) {
                    MarketeerAdvCursor();
                    pCtx->Eip = 0x4519c6;
                    MarketeerSetCaption(MKT_CAPTION_PLAYING);
                    return;
                } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                    MarketeerAdvCursor();
                    U32_REF(DATA_SCENE_ID) = 4;
                } else {
                    MarketeerAdvCursor();
                }
            }
        }
        MarketeerSetCaption(MKT_CAPTION_WAITING_SIGNAL);
        pCtx->Eip = 0x451c1e;
    }
    EHOOK_DY(mkt_on_continue_2, (void*)0x45206a)
    {
        pCtx->Eip = 0x452075;
    }
    EHOOK_DY(mkt_on_continue_3, (void*)0x452077)
    {
        pCtx->Eax = 90;
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THInitHook)
    static __declspec(noinline) void THGuiCreate()
    {
        // Hooks
        auto res = MarketeerInit("th15", (void*)0x519bb0);
        if (res) {
            if (res == 1) {
                THMarketeerViewerHook::singleton().EnableAllHooks();
            }
            THMarketeerHook::singleton().EnableAllHooks();
        }
        THDataInit();
    }
    static __declspec(noinline) void THInitHookDisable()
    {
        auto& s = THInitHook::singleton();
        s.th15_gui_init_1.Disable();
        s.th15_gui_init_2.Disable();
    }
    PATCH_DY(th15_disable_demo, (void*)0x460900, "\xff\xff\xff\x7f", 4);
    EHOOK_DY(th15_disable_mutex, (void*)0x4713ec)
    {
        pCtx->Eip = 0x4715a9;
    }
    PATCH_DY(th15_startup_1, (void*)0x46055f, "\x90\x90", 2);
    PATCH_DY(th15_startup_2, (void*)0x4610c3, "\xeb", 1);
    EHOOK_DY(th15_gui_init_1, (void*)0x4617ff)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    EHOOK_DY(th15_gui_init_2, (void*)0x47341b)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    HOOKSET_ENDDEF()
}

void TH15Init()
{
    MarketeerMutex();
    TH15::THInitHook::singleton().EnableAllHooks();
    TryKeepUpRefreshRate((void*)0x47356c, (void*)0x47333d);
    if (GetModuleHandleA("vpatch_th15.dll")) {
        TryKeepUpRefreshRate((void*)((DWORD)GetModuleHandleA("vpatch_th15.dll") + 0x6bd9));
    }
}
}