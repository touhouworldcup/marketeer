#include "thprac_utils.h"
#include <list>

namespace THPrac {
namespace TH14 {
    void THDataInit()
    {
        AnlyDataInit();

        DataRef<DATA_SCENE_ID>(U32_ARG(0x4d964c));
        DataRef<DATA_RND_SEED>(U16_ARG(0x4db510));
        DataRef<DATA_DIFFCULTY>(U8_ARG(0x4f5834));
        DataRef<DATA_SHOT_TYPE>(U8_ARG(0x4f5828));
        DataRef<DATA_SUB_SHOT_TYPE>(U8_ARG(0x4f582c));
        DataRef<DATA_STAGE>(U8_ARG(0x4f58a4));
        DataRef<DATA_STARTING_STAGE>(U8_ARG(0x4f58a8));
    }
    HOOKSET_DEFINE(THMarketeerHook)
    EHOOK_DY(mkt_skip_1, (void*)0x45a742)
    {
        DWORD skipAddress = 0x45a9f7;
        DWORD enterAddress = 0x45a752;
        if (MarketeerGetStatus() == 2) {
            auto c = GetMemContent(0x4db6a4, 0x28);
            if (c > 1 && c < 4) {
                pCtx->Eip = skipAddress;
            }
        } else if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                if (sig->GetCmd() == MARKETEER_NEW_GAME) {
                    *(uint32_t*)GetMemAddr(0x4db6a4, 0x28) = 0;
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
    EHOOK_DY(mkt_stage_start, (void*)0x455ed0)
    {
        if (MarketeerGetStatus() == 2) {
            uint32_t stage = DataRef<DATA_STAGE>();
            uint32_t startingStage = DataRef<DATA_STARTING_STAGE>();
            uint32_t player = DataRef<DATA_SHOT_TYPE>() * 2 + DataRef<DATA_SUB_SHOT_TYPE>();
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
    EHOOK_DY(mkt_game_input, (void*)0x45506c)
    {
        if (MarketeerGetStatus() == 2) {
            uint16_t inputBuf[3];
            inputBuf[0] = *(uint16_t*)0x4d6a90;
            inputBuf[1] = *(uint16_t*)0x4d6a9c;
            inputBuf[2] = *(uint16_t*)0x4d6aa0;
            MarketeerPush(inputBuf, 6);
        } else if (MarketeerGetStatus() == 1) {
            uint16_t outputBuf[3];
            auto cmd = MarketeerInputHelper(outputBuf, 6);
            if (cmd == MARKETEER_NEW_GAME) {
                U32_REF(DATA_SCENE_ID) = 10;
            } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                U32_REF(DATA_SCENE_ID) = 4;
            }
            *(uint16_t*)0x4d6a90 = outputBuf[0];
            *(uint16_t*)0x4d6a9c = outputBuf[1];
            *(uint16_t*)0x4d6aa0 = outputBuf[2];
        }
    }
    EHOOK_DY(mkt_next_stage_start, (void*)0x43361e)
    {
        if (MarketeerGetStatus() == 2) {
            auto stage = DataRef<DATA_STAGE>();
            if (stage == 6 || stage == 7) {
                MarketeerOnGameComlete();
            }
        }
    }
    EHOOK_DY(mkt_on_scene_switch, (void*)0x446945)
    {
        if (MarketeerGetStatus() == 2) {
            if (*(uint32_t*)(pCtx->Esi + 0x6ec) == 4 && *(uint32_t*)(pCtx->Esi + 0x6e8) == 7) {
                MarketeerOnQuitToMenu();
            }
        } else if (MarketeerGetStatus() == 1) {
            if (*(uint32_t*)(pCtx->Esi + 0x6ec) == 15) {
                *(uint32_t*)(pCtx->Esi + 0x6ec) = pCtx->Eax = 4;
            }
        }
    }
    EHOOK_DY(mkt_on_continue, (void*)0x44ae10)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerPushSignal(MARKETEER_CONTINUE);
        }
    }
    EHOOK_DY(mkt_on_quit, (void*)0x469e3f)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerEndLive();
        } else if (MarketeerGetStatus() == 1) {
            MarketeerTerminate();
        }
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THMarketeerViewerHook)
    EHOOK_DY(mkt_disable_input, (void*)0x4018b0)
    {
        if (MarketeerGetStatus() == 1) {
            pCtx->Eax = 0;
            pCtx->Eip = 0x401a1a;
        }
    }
    PATCH_DY(mkt_skip_2, (void*)0x45da3c, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_3, (void*)0x45e00c, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_4, (void*)0x45e571, "\x90\x90\x90\x90\x90\x90", 6);
    EHOOK_DY(mkt_enter, (void*)0x45e473)
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
            *(uint32_t*)0x4db69c = 0x4d5988 + stage * 0xcc;
            U32_REF(DATA_DIFFCULTY) = rank;
            U32_REF(DATA_SHOT_TYPE) = player;
            U32_REF(DATA_SUB_SHOT_TYPE) = subShot;
        }
    }
    EHOOK_DY(mkt_next_stage_end, (void*)0x433623)
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
    EHOOK_DY(mkt_intermission, (void*)0x43601a)
    {
        if (MarketeerGetStatus() == 1) {
            StreamSignal* sig;
            for (sig = MarketeerGetCursor(); DataRef<MARKETEER_LIMIT_LOCK>() || !sig; sig = MarketeerGetCursor()) {
                Sleep(1);
            }
        }
    }
    EHOOK_DY(mkt_on_continue_1, (void*)0x44a4f4)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_CONTINUE) {
                    MarketeerAdvCursor();
                    pCtx->Eip = 0x44a4fa;
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
        pCtx->Eip = 0x44a5e1;
    }
    EHOOK_DY(mkt_on_continue_2, (void*)0x44abb7)
    {
        pCtx->Eip = 0x44abbd;
    }
    EHOOK_DY(mkt_on_continue_3, (void*)0x44abd6)
    {
        pCtx->Eax = 90;
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THInitHook)
    static __declspec(noinline) void THGuiCreate()
    {
        // Hooks
        auto res = MarketeerInit("th14", (void*)0x4f5a18);
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
        s.th14_gui_init_1.Disable();
        s.th14_gui_init_2.Disable();
    }
    PATCH_DY(th14_disable_demo, (void*)0x45996c, "\xff\xff\xff\x7f", 4);
    EHOOK_DY(th14_disable_mutex, (void*)0x469387)
    {
        pCtx->Eip = 0x469556;
    }
    PATCH_DY(th14_startup_1, (void*)0x4595a5, "\xeb", 1);
    PATCH_DY(th14_startup_2, (void*)0x45a173, "\xeb", 1);
    EHOOK_DY(th14_gui_init_1, (void*)0x45a591)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    EHOOK_DY(th14_gui_init_2, (void*)0x46b37b)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    HOOKSET_ENDDEF()
}

void TH14Init()
{
    MarketeerMutex();
    TH14::THInitHook::singleton().EnableAllHooks();
    TryKeepUpRefreshRate((void*)0x46b4b3, (void*)0x46b2de);
}
}