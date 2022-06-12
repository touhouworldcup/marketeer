#include "thprac_utils.h"

namespace THPrac {
namespace TH10 {
    void THDataInit()
    {
        AnlyDataInit();

        DataRef<DATA_SCENE_ID>(U32_ARG(0x491fb8));
        DataRef<DATA_RND_SEED>(U16_ARG(0x4918b0));
        DataRef<DATA_DIFFCULTY>(U8_ARG(0x474c74));
        DataRef<DATA_SHOT_TYPE>(U8_ARG(0x474c68));
        DataRef<DATA_SUB_SHOT_TYPE>(U8_ARG(0x474c6c));
        DataRef<DATA_STAGE>(U8_ARG(0x474c7c));
        DataRef<DATA_STARTING_STAGE>(U8_ARG(0x474c80));
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
            *(uint32_t*)0x477848 = 0x474788 + stage * 48;
            U32_REF(DATA_DIFFCULTY) = rank;
            U32_REF(DATA_SHOT_TYPE) = player;
            U32_REF(DATA_SUB_SHOT_TYPE) = subShot;
        }
    }
    HOOKSET_DEFINE(THMarketeerHook)
    EHOOK_DY(mkt_skip_1, (void*)0x42d5ff)
    {
        DWORD skipAddress = 0x42d8c2;
        DWORD enterAddress = 0x42d60f;
        if (MarketeerGetStatus() == 2) {
            auto c = GetMemContent(0x47784C, 0x24);
            if (c > 1 && c < 4) {
                pCtx->Eip = skipAddress;
            }
        } else if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                if (sig->GetCmd() == MARKETEER_NEW_GAME) {
                    *(uint32_t*)GetMemAddr(0x47784C, 0x24) = 0;
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
    EHOOK_DY(mkt_stage_start, (void*)0x42a450)
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
    EHOOK_DY(mkt_game_input, (void*)0x42984e)
    {
        if (MarketeerGetStatus() == 2) {
            uint16_t inputBuf[3];
            inputBuf[0] = *(uint16_t*)0x474e5c;
            inputBuf[1] = *(uint16_t*)0x474e62;
            inputBuf[2] = *(uint16_t*)0x474e64;
            MarketeerPush(inputBuf, 6);
        } else if (MarketeerGetStatus() == 1) {
            uint16_t outputBuf[3];
            auto cmd = MarketeerInputHelper(outputBuf, 6);
            if (cmd == MARKETEER_NEW_GAME) {
                U32_REF(DATA_SCENE_ID) = 10;
            } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                U32_REF(DATA_SCENE_ID) = 4;
            }
            *(uint16_t*)0x474e5c = outputBuf[0];
            *(uint16_t*)0x474e62 = outputBuf[1];
            *(uint16_t*)0x474e64 = outputBuf[2];
        }
    }
    EHOOK_DY(mkt_next_stage_start, (void*)0x416bd3)
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
    EHOOK_ST(mkt_next_stage_end, (void*)0x416f09)
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
    EHOOK_DY(mkt_on_scene_switch, (void*)0x4218e8)
    {
        if (MarketeerGetStatus() == 2) {
            if (*(uint32_t*)(pCtx->Edi + 0x390) == 4 && *(uint32_t*)(pCtx->Edi + 0x38c) == 7) {
                MarketeerOnQuitToMenu();
            }
        } else if (MarketeerGetStatus() == 1) {
            if (*(uint32_t*)(pCtx->Edi + 0x390) == 14) {
                *(uint32_t*)(pCtx->Edi + 0x390) = pCtx->Eax = 4;
            }
        }
    }
    EHOOK_DY(mkt_on_continue, (void*)0x424360)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerPushSignal(MARKETEER_CONTINUE);
        }
    }
    EHOOK_DY(mkt_on_quit, (void*)0x43903a)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerEndLive();
        } else if (MarketeerGetStatus() == 1) {
            MarketeerTerminate();
        }
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THMarketeerViewerHook)
    EHOOK_DY(mkt_disable_input, (void*)0x44a190)
    {
        if (MarketeerGetStatus() == 1) {
            pCtx->Eax = 0;
            pCtx->Eip = 0x44a32b;
        }
    }
    PATCH_DY(mkt_skip_2, (void*)0x4304dd, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_3, (void*)0x4308ac, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_4, (void*)0x430e24, "\x90\x90\x90\x90\x90\x90", 6);
    EHOOK_DY(mkt_enter_1, (void*)0x430f92)
    {
        MktEnter();
    }
    EHOOK_DY(mkt_enter_2, (void*)0x430f6e)
    {
        MktEnter();
    }
    EHOOK_DY(mkt_intermission, (void*)0x417bfa)
    {
        if (MarketeerGetStatus() == 1) {
            StreamSignal* sig;
            for (sig = MarketeerGetCursor(); DataRef<MARKETEER_LIMIT_LOCK>() || !sig; sig = MarketeerGetCursor()) {
                Sleep(1);
            }
        }
    }
    EHOOK_DY(mkt_on_continue_1, (void*)0x423910)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_CONTINUE) {
                    MarketeerAdvCursor();
                    pCtx->Eip = 0x423916;
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
        pCtx->Eip = 0x423a0a;
    }
    EHOOK_DY(mkt_on_continue_2, (void*)0x423736)
    {
        pCtx->Eip = 0x42373c;
    }
    EHOOK_DY(mkt_on_continue_3, (void*)0x4238a4)
    {
        if (*(uint32_t*)(pCtx->Ebp + 0x24) == 2) {
            pCtx->Esp -= 4;
            *(uint32_t*)pCtx->Esp = 1;
            pCtx->Eip = 0x4238d3;
        } else {
            pCtx->Eip = 0x4238da;
        }
    }
    EHOOK_DY(mkt_on_continue_4, (void*)0x42406f)
    {
        pCtx->Eip = 0x42407f;
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THInitHook)
    static __declspec(noinline) void THGuiCreate()
    {
        // Hooks
        auto res = MarketeerInit("th10", (void*)0x4924f0);
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
        s.th10_gui_init_1.Disable();
        s.th10_gui_init_2.Disable();
    }
    PATCH_DY(th10_disable_demo, (void*)0x42ce44, "\xff\xff\xff\x7f", 4);
    EHOOK_DY(th10_disable_mutex, (void*)0x43a024)
    {
        pCtx->Eip = 0x43a051;
    }
    PATCH_DY(th10_startup_1, (void*)0x42ca31, "\xeb", 1);
    PATCH_DY(th10_startup_2, (void*)0x42d04f, "\xeb\x23", 2);
    EHOOK_DY(th10_gui_init_1, (void*)0x42d516)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    EHOOK_DY(th10_gui_init_2, (void*)0x439d16)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    HOOKSET_ENDDEF()
}

void TH10Init()
{
    MarketeerMutex();
    TH10::THInitHook::singleton().EnableAllHooks();
    TryKeepUpRefreshRate((void*)0x439950);
    if (GetModuleHandleA("vpatch_th10.dll")) {
        TryKeepUpRefreshRate((void*)((DWORD)GetModuleHandleA("vpatch_th10.dll") + 0x553b));
    }
}
}