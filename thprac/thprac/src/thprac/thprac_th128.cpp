#include "thprac_utils.h"

namespace THPrac {
namespace TH128 {
    void THDataInit()
    {
        AnlyDataInit();

        DataRef<DATA_SCENE_ID>(U32_ARG(0x4d33c0));
        DataRef<DATA_RND_SEED>(U16_ARG(0x4d2ae0));
        DataRef<DATA_DIFFCULTY>(U8_ARG(0x4b4d0c));
        DataRef<DATA_STAGE>(U8_ARG(0x4b4d14));
        DataRef<DATA_STARTING_STAGE>(U8_ARG(0x4b4d18));
    }

    __declspec(noinline) bool Mkt128TestMsg()
    {
        auto dword_4B8950 = GetMemContent(0x4B8950);
        if (dword_4B8950) {
            auto v3 = GetMemContent(dword_4B8950 + 0x5a1c);
            if (v3) {
                uint32_t result = 0;
                uint32_t argEcx = GetMemContent(v3 + 0x6c);
                uint32_t argEdx = GetMemContent(0x4d2e50);
                _asm {
                    push argEcx;
                    mov ecx, argEcx;
                    mov edx, argEdx;
                    mov eax, 0x467420;
                    call eax;
                    mov result, eax;
                }
                if (result) {
                    return false;
                }
            }
        }
        return true;
    }
    void MktEnter()
    {
        auto sig = MarketeerGetCursor();
        if (sig && sig->GetCmd() == MARKETEER_NEW_GAME) {
            uint32_t stage = (sig->param1 & 0x0000ff00) >> 8;
            uint32_t rank = (sig->param1 & 0x00ff0000) >> 16;
            uint32_t player = ((sig->param1 & 0xff000000) >> 24) / 2;
            uint32_t subShot = ((sig->param1 & 0xff000000) >> 24) % 2;
            U32_REF(DATA_STAGE) = stage;
            U32_REF(DATA_STARTING_STAGE) = stage;
            U32_REF(DATA_SCENE_ID) = 7;
            *(uint32_t*)0x4b8a98 = 0x4b1a30 + stage * 40;
            U32_REF(DATA_DIFFCULTY) = rank;
            //U32_REF(DATA_SHOT_TYPE) = player;
            ///U32_REF(DATA_SUB_SHOT_TYPE) = subShot;
        }
    }
    HOOKSET_DEFINE(THMarketeerHook)
    EHOOK_DY(mkt_skip_1, (void*)0x44655b)
    {
        DWORD skipAddress = 0x4468f8;
        DWORD enterAddress = 0x44656b;
        if (MarketeerGetStatus() == 2) {
            auto c = GetMemContent(0x4b8a9c, 0x28);
            if (c > 1 && c < 5) {
                pCtx->Eip = skipAddress;
            }
        } else if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                if (sig->GetCmd() == MARKETEER_NEW_GAME) {
                    *(uint32_t*)GetMemAddr(0x4b8a9c, 0x28) = 0;
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
    EHOOK_DY(mkt_stage_start, (void*)0x442090)
    {
        if (MarketeerGetStatus() == 2) {
            uint32_t stage = DataRef<DATA_STAGE>();
            uint32_t startingStage = DataRef<DATA_STARTING_STAGE>();
            uint32_t player = 0;
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
    EHOOK_DY(mkt_game_input, (void*)0x441209)
    {
        if (MarketeerGetStatus() == 2) {
            uint16_t inputBuf[3];
            inputBuf[0] = *(uint16_t*)0x4db210;
            inputBuf[1] = *(uint16_t*)0x4db21c;
            inputBuf[2] = *(uint16_t*)0x4db220;
            if (Mkt128TestMsg()) {
                MarketeerPush(inputBuf, 6);
            }
        } else if (MarketeerGetStatus() == 1) {
            uint16_t outputBuf[3] {};
            if (Mkt128TestMsg()) {
                auto cmd = MarketeerInputHelper(outputBuf, 6);
                if (cmd == MARKETEER_NEW_GAME) {
                    U32_REF(DATA_SCENE_ID) = 10;
                } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                    U32_REF(DATA_SCENE_ID) = 4;
                }
            }
            *(uint16_t*)0x4db210 = outputBuf[0];
            *(uint16_t*)0x4db21c = outputBuf[1];
            *(uint16_t*)0x4db220 = outputBuf[2];
        }
    }
    EHOOK_DY(mkt_next_stage_start, (void*)0x42454e)
    {
        if (MarketeerGetStatus() == 2) {
            auto stage = DataRef<DATA_STAGE>();
            if (stage == 3 || stage == 5 ||
                stage == 8 || stage == 10 ||
                stage == 13 || stage == 15 ||
                stage == 16) {
                MarketeerOnGameComlete();
            }
        } else if (MarketeerGetStatus() == 1) {
            THMarketeerHook::singleton().mkt_next_stage_end.Enable();
        }
    }
    EHOOK_ST(mkt_next_stage_end, (void*)0x4248be)
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
    EHOOK_DY(mkt_on_scene_switch, (void*)0x4358f6)
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
    EHOOK_DY(mkt_on_continue, (void*)0x439043)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerPushSignal(MARKETEER_CONTINUE);
        }
    }
    EHOOK_DY(mkt_on_quit, (void*)0x454359)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerEndLive();
        } else if (MarketeerGetStatus() == 1) {
            MarketeerTerminate();
        }
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THMarketeerViewerHook)
    EHOOK_DY(mkt_disable_input, (void*)0x4689e0)
    {
        if (MarketeerGetStatus() == 1) {
            pCtx->Eax = 0;
            pCtx->Eip = 0x468b6e;
        }
    }
    PATCH_DY(mkt_skip_2, (void*)0x449edd, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_3, (void*)0x44a244, "\x90\x90\x90\x90\x90\x90", 6);
    EHOOK_DY(mkt_enter_1, (void*)0x44a3a1)
    {
        MktEnter();
    }
    EHOOK_DY(mkt_intermission, (void*)0x4263a7)
    {
        if (MarketeerGetStatus() == 1) {
            StreamSignal* sig;
            for (sig = MarketeerGetCursor(); DataRef<MARKETEER_LIMIT_LOCK>() || !sig; sig = MarketeerGetCursor()) {
                Sleep(1);
            }
        }
    }
    EHOOK_DY(mkt_on_continue_1, (void*)0x4386b1)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_CONTINUE) {
                    MarketeerAdvCursor();
                    pCtx->Eip = 0x4386c1;
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
        pCtx->Eip = 0x4387ce;
    }
    EHOOK_DY(mkt_on_continue_2, (void*)0x438e25)
    {
        pCtx->Eip = 0x438e30;
    }
    EHOOK_DY(mkt_on_continue_3, (void*)0x438e49)
    {
        pCtx->Eax = 90;
    }
    EHOOK_DY(mkt_th128_path_select_1, (void*)0x423593)
    {
        if (MarketeerGetStatus() == 1) {
            if (*(uint32_t*)(pCtx->Esi + 0x74) != 2) {
                pCtx->Eip = 0x42359c;
            } else {
                pCtx->Eip = 0x4235c9;
            }
        }
    }
    EHOOK_DY(mkt_th128_path_select_2, (void*)0x4235fd)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_NEW_STAGE) {
                    pCtx->Eip = 0x423609;
                    MarketeerSetCaption(MKT_CAPTION_PLAYING);
                    return;
                } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                    U32_REF(DATA_SCENE_ID) = 4;
                } else if (cmd == MARKETEER_NEW_GAME) {
                    U32_REF(DATA_SCENE_ID) = 10;
                } else if (cmd == MARKETEER_GAME_COMPLETE) {
                    U32_REF(DATA_SCENE_ID) = 4;
                }
            }
        }
        MarketeerSetCaption(MKT_CAPTION_WAITING_SIGNAL);
        pCtx->Eip = 0x42366d;
    }
    EHOOK_DY(mkt_th128_stage_calc, (void*)0x427400)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_NEW_STAGE) {
                    uint32_t stage = (sig->param1 & 0x0000ff00) >> 8;
                    pCtx->Eax = stage;
                } 
            }
        }
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THInitHook)
    static __declspec(noinline) void THGuiCreate()
    {
        // Hooks
        auto res = MarketeerInit("th128", (void*)0x4d3970);
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
        s.th128_gui_init_1.Disable();
        s.th128_gui_init_2.Disable();
    }
    PATCH_DY(th128_disable_demo, (void*)0x4457ae, "\xff\xff\xff\x7f", 4);
    EHOOK_DY(th128_disable_mutex, (void*)0x453a89)
    {
        pCtx->Eip = 0x453aa2;
    }
    PATCH_DY(th128_startup_1, (void*)0x4452b7, "\xeb", 1);
    PATCH_DY(th128_startup_2, (void*)0x445eb9, "\xeb", 1);
    EHOOK_DY(th128_gui_init_1, (void*)0x446273)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    EHOOK_DY(th128_gui_init_2, (void*)0x455a63)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    HOOKSET_ENDDEF()
}

void TH128Init()
{
    TH128::THInitHook::singleton().EnableAllHooks();
    TryKeepUpRefreshRate((void*)0x455669);
}
}