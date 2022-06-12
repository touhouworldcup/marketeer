#include "thprac_utils.h"

namespace THPrac {
namespace TH17 {
    void THDataInit()
    {
        AnlyDataInit();

        DataRef<DATA_SCENE_ID>(U32_ARG(0x4b61d4));
        DataRef<DATA_RND_SEED>(U16_ARG(0x4b7668));
        DataRef<DATA_DIFFCULTY>(U8_ARG(0x4b5a00));
        DataRef<DATA_SHOT_TYPE>(U8_ARG(0x4b59f4));
        DataRef<DATA_SHOT_GOAST>(U8_ARG(0x4b59f8));
        DataRef<DATA_STAGE>(U8_ARG(0x4b59dc));
        DataRef<DATA_STARTING_STAGE>(U8_ARG(0x4b59e0));
    }

    
    HOOKSET_DEFINE(THMarketeerHook)
    EHOOK_DY(mkt_skip_1, (void*)0x4529db)
    {
        DWORD skipAddress = 0x452cc6;
        DWORD enterAddress = 0x4529eb;
        if (MarketeerGetStatus() == 2) {
            auto c = GetMemContent(0x4b77f0, 0x24);
            if (c > 1 && c < 5) {
                pCtx->Eip = skipAddress;
            }
        } else if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                if (sig->GetCmd() == MARKETEER_NEW_GAME) {
                    *(uint32_t*)GetMemAddr(0x4b77f0, 0x24) = 0;
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
    EHOOK_DY(mkt_stage_start, (void*)0x44f5a0)
    {
        if (MarketeerGetStatus() == 2) {
            uint32_t stage = DataRef<DATA_STAGE>();
            uint32_t startingStage = DataRef<DATA_STARTING_STAGE>();
            uint32_t player = DataRef<DATA_SHOT_TYPE>() * 3 + DataRef<DATA_SHOT_GOAST>();
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
    EHOOK_DY(mkt_game_input, (void*)0x44f1e3)
    {
        if (MarketeerGetStatus() == 2) {
            uint16_t inputBuf[3];
            inputBuf[0] = *(uint16_t*)0x4b3448;
            inputBuf[1] = *(uint16_t*)0x4b3454;
            inputBuf[2] = *(uint16_t*)0x4b3458;
            MarketeerPush(inputBuf, 6);
        } else if (MarketeerGetStatus() == 1) {
            uint16_t outputBuf[3];
            auto cmd = MarketeerInputHelper(outputBuf, 6);
            if (cmd == MARKETEER_NEW_GAME) {
                U32_REF(DATA_SCENE_ID) = 10;
            } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                U32_REF(DATA_SCENE_ID) = 4;
            }
            *(uint16_t*)0x4b3448 = outputBuf[0];
            *(uint16_t*)0x4b3454 = outputBuf[1];
            *(uint16_t*)0x4b3458 = outputBuf[2];
        }
    }
    EHOOK_DY(mkt_next_stage_start, (void*)0x42ec5e)
    {
        if (MarketeerGetStatus() == 2) {
            auto stage = DataRef<DATA_STAGE>();
            if (stage == 6 || stage == 7) {
                MarketeerOnGameComlete();
            }
        }
    }
    EHOOK_DY(mkt_on_scene_switch, (void*)0x4427e7)
    {
        if (MarketeerGetStatus() == 2) {
            if (*(uint32_t*)(pCtx->Esi + 0x6f4) == 4 && *(uint32_t*)(pCtx->Esi + 0x6f0) == 7) {
                MarketeerOnQuitToMenu();
            }
        } else if (MarketeerGetStatus() == 1) {
            if (*(uint32_t*)(pCtx->Esi + 0x6f4) == 15) {
                *(uint32_t*)(pCtx->Esi + 0x6f4) = pCtx->Eax = 4;
            }
        }
    }
    EHOOK_DY(mkt_on_continue, (void*)0x445f50)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerPushSignal(MARKETEER_CONTINUE);
        }
    }
    EHOOK_DY(mkt_on_quit, (void*)0x4617be)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerEndLive();
        } else if (MarketeerGetStatus() == 1) {
            MarketeerTerminate();
        }
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THMarketeerViewerHook)
    EHOOK_DY(mkt_disable_input, (void*)0x401690)
    {
        if (MarketeerGetStatus() == 1) {
            pCtx->Eax = 0;
            pCtx->Eip = 0x4019a7;
        }
    }
    PATCH_DY(mkt_skip_2, (void*)0x45580f, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_3, (void*)0x456022, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_4, (void*)0x45664d, "\x90\x90\x90\x90\x90\x90", 6);
    EHOOK_DY(mkt_enter, (void*)0x456877)
    {
        auto sig = MarketeerGetCursor();
        if (sig && sig->GetCmd() == MARKETEER_NEW_GAME) {
            uint32_t stage = (sig->param1 & 0x0000ff00) >> 8;
            uint32_t rank = (sig->param1 & 0x00ff0000) >> 16;
            uint32_t player = ((sig->param1 & 0xff000000) >> 24) / 3;
            uint32_t goast = ((sig->param1 & 0xff000000) >> 24) % 3;
            U32_REF(DATA_STAGE) = stage;
            U32_REF(DATA_STARTING_STAGE) = stage;
            U32_REF(DATA_SCENE_ID) = 7;
            *(uint32_t*)0x4b77e8 = 0x4b23e8 + stage * 0xe8;
            U32_REF(DATA_DIFFCULTY) = rank;
            U32_REF(DATA_SHOT_TYPE) = player;
            U32_REF(DATA_SHOT_GOAST) = goast;
        }
    }
    EHOOK_DY(mkt_next_stage_end, (void*)0x42ec63)
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
    EHOOK_DY(mkt_intermission, (void*)0x430b8a)
    {
        if (MarketeerGetStatus() == 1) {
            StreamSignal* sig;
            for (sig = MarketeerGetCursor(); DataRef<MARKETEER_LIMIT_LOCK>() || !sig; sig = MarketeerGetCursor()) {
                Sleep(1);
            }
        }
    }
    EHOOK_DY(mkt_on_continue_1, (void*)0x445002)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_CONTINUE) {
                    MarketeerAdvCursor();
                    pCtx->Eip = 0x445008;
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
        pCtx->Eip = 0x4451cb;
    }
    EHOOK_DY(mkt_on_continue_2, (void*)0x4456da)
    {
        pCtx->Eip = 0x4456e5;
    }
    EHOOK_DY(mkt_on_continue_3, (void*)0x4456e7)
    {
        pCtx->Eax = 90;
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THInitHook)
    static __declspec(noinline) void THGuiCreate()
    {
        // Hooks
        auto res = MarketeerInit("th17", (void*)0x5226c0);
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
        s.th17_gui_init_1.Disable();
        s.th17_gui_init_2.Disable();
    }
    PATCH_DY(th17_disable_demo, (void*)0x45167c, "\xff\xff\xff\x7f", 4);
    EHOOK_DY(th17_disable_mutex, (void*)0x46055c)
    {
        pCtx->Eip = 0x46083e;
    }
    PATCH_DY(th17_startup_1, (void*)0x4511af, "\x90\x90", 2);
    PATCH_DY(th17_startup_2, (void*)0x451f32, "\xeb", 1);
    EHOOK_DY(th17_gui_init_1, (void*)0x452502)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    EHOOK_DY(th17_gui_init_2, (void*)0x462e17)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    HOOKSET_ENDDEF()
}

void TH17Init()
{
    MarketeerMutex();
    TH17::THInitHook::singleton().EnableAllHooks();
    TryKeepUpRefreshRate((void*)0x462f7a, (void*)0x462d3d);
}
}