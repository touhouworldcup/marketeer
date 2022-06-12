#include "thprac_utils.h"

namespace THPrac {
namespace TH11 {
    void THDataInit()
    {
        AnlyDataInit();

        DataRef<DATA_SCENE_ID>(U32_ARG(0x4c37d8));
        DataRef<DATA_RND_SEED>(U16_ARG(0x4c2f00));
        DataRef<DATA_DIFFCULTY>(U8_ARG(0x4a5720));
        DataRef<DATA_SHOT_TYPE>(U8_ARG(0x4a5710));
        DataRef<DATA_SUB_SHOT_TYPE>(U8_ARG(0x4a5714));
        DataRef<DATA_STAGE>(U8_ARG(0x4a5728));
        DataRef<DATA_STARTING_STAGE>(U8_ARG(0x4a572c));
    }


    void MktEnter()
    {
        auto sig = MarketeerGetCursor();
        if (sig && sig->GetCmd() == MARKETEER_NEW_GAME) {
            uint32_t stage = (sig->param1 & 0x0000ff00) >> 8;
            uint32_t rank = (sig->param1 & 0x00ff0000) >> 16;
            uint32_t player = ((sig->param1 & 0xff000000) >> 24) / 3;
            uint32_t subShot = ((sig->param1 & 0xff000000) >> 24) % 3;
            U32_REF(DATA_STAGE) = stage;
            U32_REF(DATA_STARTING_STAGE) = stage;
            U32_REF(DATA_SCENE_ID) = 7;
            *(uint32_t*)0x4a8ec8 = 0x4a3828 + stage * 64;
            U32_REF(DATA_DIFFCULTY) = rank;
            U32_REF(DATA_SHOT_TYPE) = player;
            U32_REF(DATA_SUB_SHOT_TYPE) = subShot;
        }
    }
    HOOKSET_DEFINE(THMarketeerHook)
    EHOOK_DY(mkt_skip_1, (void*)0x43a3af)
    {
        DWORD skipAddress = 0x43a684;
        DWORD enterAddress = 0x43a3bf;
        if (MarketeerGetStatus() == 2) {
            auto c = GetMemContent(0x4a8ecc, 0x24);
            if (c > 1 && c < 4) {
                pCtx->Eip = skipAddress;
            }
        } else if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                if (sig->GetCmd() == MARKETEER_NEW_GAME) {
                    *(uint32_t*)GetMemAddr(0x4a8ecc, 0x24) = 0;
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
    EHOOK_DY(mkt_stage_start, (void*)0x436da0)
    {
        if (MarketeerGetStatus() == 2) {
            uint32_t stage = DataRef<DATA_STAGE>();
            uint32_t startingStage = DataRef<DATA_STARTING_STAGE>();
            uint32_t player = DataRef<DATA_SHOT_TYPE>() * 3 + DataRef<DATA_SUB_SHOT_TYPE>();
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
    EHOOK_DY(mkt_game_input, (void*)0x436085)
    {
        if (MarketeerGetStatus() == 2) {
            uint16_t inputBuf[3];
            inputBuf[0] = *(uint16_t*)0x4c93c0;
            inputBuf[1] = *(uint16_t*)0x4c93cc;
            inputBuf[2] = *(uint16_t*)0x4c93d0;
            MarketeerPush(inputBuf, 6);
        } else if (MarketeerGetStatus() == 1) {
            uint16_t outputBuf[3];
            auto cmd = MarketeerInputHelper(outputBuf, 6);
            if (cmd == MARKETEER_NEW_GAME) {
                U32_REF(DATA_SCENE_ID) = 10;
            } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                U32_REF(DATA_SCENE_ID) = 4;
            }
            *(uint16_t*)0x4c93c0 = outputBuf[0];
            *(uint16_t*)0x4c93cc = outputBuf[1];
            *(uint16_t*)0x4c93d0 = outputBuf[2];
        }
    }
    EHOOK_DY(mkt_next_stage_start, (void*)0x41eb17)
    {
        if (MarketeerGetStatus() == 2) {
            auto stage = DataRef<DATA_STAGE>();
            if (stage == 6 || stage == 7) {
                MarketeerOnGameComlete();
            }
        } else if (MarketeerGetStatus() == 1) {
            THMarketeerHook::singleton().mkt_next_stage_end.Enable();
        }
    }
    EHOOK_ST(mkt_next_stage_end, (void*)0x41d43e)
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
        THMarketeerHook::singleton().mkt_next_stage_end.Disable();
    }
    EHOOK_DY(mkt_on_scene_switch, (void*)0x42b0fc)
    {
        if (MarketeerGetStatus() == 2) {
            if (*(uint32_t*)(pCtx->Edi + 0x558) == 4 && *(uint32_t*)(pCtx->Edi + 0x554) == 7) {
                MarketeerOnQuitToMenu();
            }
        } else if (MarketeerGetStatus() == 1) {
            if (*(uint32_t*)(pCtx->Edi + 0x558) == 15) {
                *(uint32_t*)(pCtx->Edi + 0x558) = pCtx->Eax = 4;
            }
        }
    }
    EHOOK_DY(mkt_on_continue, (void*)0x42e5a5)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerPushSignal(MARKETEER_CONTINUE);
        }
    }
    EHOOK_DY(mkt_on_quit, (void*)0x445d9e)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerEndLive();
        } else if (MarketeerGetStatus() == 1) {
            MarketeerTerminate();
        }
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THMarketeerViewerHook)
    EHOOK_DY(mkt_disable_input, (void*)0x457270)
    {
        if (MarketeerGetStatus() == 1) {
            pCtx->Eax = 0;
            pCtx->Eip = 0x4573fe;
        }
    }
    PATCH_DY(mkt_skip_2, (void*)0x43c98b, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_3, (void*)0x43cdf1, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_4, (void*)0x43d47b, "\x90\x90\x90\x90\x90\x90", 6);
    EHOOK_DY(mkt_enter_1, (void*)0x43d60f)
    {
        MktEnter();
    }
    EHOOK_DY(mkt_intermission, (void*)0x41fd87)
    {
        if (MarketeerGetStatus() == 1) {
            StreamSignal* sig;
            for (sig = MarketeerGetCursor(); DataRef<MARKETEER_LIMIT_LOCK>() || !sig; sig = MarketeerGetCursor()) {
                Sleep(1);
            }
        }
    }
    EHOOK_DY(mkt_on_continue_1, (void*)0x42dc1c)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_CONTINUE) {
                    MarketeerAdvCursor();
                    pCtx->Eip = 0x42dc1e;
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
        pCtx->Eip = 0x42dc83;
    }
    EHOOK_DY(mkt_on_continue_2, (void*)0x42e389)
    {
        pCtx->Eip = 0x42e38f;
    }
    EHOOK_DY(mkt_on_continue_3, (void*)0x42e3a2)
    {
        pCtx->Eax = 90;
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THInitHook)
    static __declspec(noinline) void THGuiCreate()
    {
        // Hooks
        auto res = MarketeerInit("th11", (void*)0x4c3d88);
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
        s.th11_gui_init_1.Disable();
        s.th11_gui_init_2.Disable();
    }
    PATCH_DY(th11_disable_demo, (void*)0x439a7b, "\xff\xff\xff\x7f", 4);
    EHOOK_DY(th11_disable_mutex, (void*)0x4455b3)
    {
        pCtx->Eip = 0x4455cc;
    }
    PATCH_DY(th11_startup_1, (void*)0x439527, "\xeb", 1);
    PATCH_DY(th11_startup_2, (void*)0x43a12a, "\xeb", 1);
    EHOOK_DY(th11_gui_init_1, (void*)0x43a299)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    EHOOK_DY(th11_gui_init_2, (void*)0x447269)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    HOOKSET_ENDDEF()
}

void TH11Init()
{
    MarketeerMutex();
    TH11::THInitHook::singleton().EnableAllHooks();
    TryKeepUpRefreshRate((void*)0x446e3c);
}
}