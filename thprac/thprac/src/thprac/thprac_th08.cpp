#include "thprac_utils.h"

namespace THPrac {
namespace TH08 {
    void THDataInit()
    {
        AnlyDataInit();

        DataRef<DATA_SCENE_ID>(U32_ARG(0x17ce8b4));
        DataRef<DATA_RND_SEED>(U16_ARG(0x164d520));
        DataRef<DATA_DIFFCULTY>(U8_ARG(0x160f538));
        DataRef<DATA_SHOT_TYPE>(U8_ARG(0x164d0b1));
        //DataRef<DATA_SUB_SHOT_TYPE>(U8_ARG(0x474c6c));
        DataRef<DATA_STAGE>(U8_ARG(0x164d2cc));
    }

    HOOKSET_DEFINE(THMarketeerHook)
    EHOOK_DY(mkt_skip_1, (void*)0x467cc6)
    {
        DWORD skipAddress = 0x46817f;
        DWORD enterAddress = 0x467cef;
        if (MarketeerGetStatus() == 2) {
            auto c = GetMemContent(0x18bde08, 0x0);
            if (c > 1 && c < 5) {
                pCtx->Eip = skipAddress;
            }
        } else if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                if (sig->GetCmd() == MARKETEER_NEW_GAME) {
                    *(uint32_t*)GetMemAddr(0x18bde08, 0x0) = 0;
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
    EHOOK_DY(mkt_init_start, (void*)0x43abd7)
    {
        if (MarketeerGetStatus() == 2) {
            DataRef<MARKETEER_OLD_TH_SEED>() = U16_REF(DATA_RND_SEED);
        } else if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                if (sig->GetCmd() == MARKETEER_NEW_GAME || sig->GetCmd() == MARKETEER_NEW_STAGE) {
                    U16_REF(DATA_RND_SEED) = (uint16_t)sig->param4;
                } 
            }
        }
    }
    EHOOK_DY(mkt_init_end, (void*)0x43b8a8)
    {
        if (MarketeerGetStatus() == 2) {
            uint32_t stage = DataRef<DATA_STAGE>();
            uint32_t startingStage = DataRef<MARKETEER_OLD_TH_STARTING>();
            uint32_t player = DataRef<DATA_SHOT_TYPE>();
            uint32_t rank = DataRef<DATA_DIFFCULTY>();
            uint16_t seed = U16_REF(DATA_RND_SEED);
            uint32_t seedPreInit = DataRef<MARKETEER_OLD_TH_SEED>();
            MarketeerOnStageStart(stage == startingStage, stage, rank, player, seed, seedPreInit, 0);
            DataRef<MARKETEER_OLD_TH_LOCK>() = 1;
        } else if (MarketeerGetStatus() == 1) {
            StreamSignal* sig;
            uint16_t outputBuf[3];
            for (sig = MarketeerGetCursor(); !sig; sig = MarketeerGetCursor()) {
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
            DataRef<MARKETEER_OLD_TH_LOCK>() = 1;
        }
    }
    EHOOK_DY(mkt_game_input_player_main, (void*)0x44c390)
    {
        if (MarketeerGetStatus() == 2) {
            uint16_t inputBuf[3] {};
            inputBuf[0] = *(uint16_t*)0x164d528;
            inputBuf[1] = *(uint16_t*)0x164d52c;
            inputBuf[2] = *(uint16_t*)0x164d534;
            //inputBuf[3] = U16_REF(DATA_RND_SEED);
            if (DataRef<MARKETEER_OLD_TH_LOCK>()) {
                MarketeerPush(inputBuf, 6);
            }
        } else if (MarketeerGetStatus() == 1) {
            uint16_t outputBuf[3] {};
            if (DataRef<MARKETEER_OLD_TH_LOCK>()) {
                auto cmd = MarketeerInputHelper(outputBuf, 6);
                if (cmd == MARKETEER_NEW_GAME) {
                    U32_REF(DATA_SCENE_ID) = 10;
                } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                    U32_REF(DATA_SCENE_ID) = 1;
                }
            } 
            *(uint16_t*)0x164d528 = outputBuf[0];
            *(uint16_t*)0x164d52c = outputBuf[1];
            *(uint16_t*)0x164d534 = outputBuf[2];
        }
    }
    EHOOK_DY(mkt_next_stage_start, (void*)0x4351df)
    {
        if (MarketeerGetStatus() == 2) {
            if (DataRef<MARKETEER_OLD_TH_LOCK>()) {
                DataRef<MARKETEER_OLD_TH_LOCK>() = 0;
                auto stage = DataRef<DATA_STAGE>();
                if (stage == 6 || stage == 7 || stage == 8) {
                    MarketeerOnGameComlete();
                }
            }
        } else if (MarketeerGetStatus() == 1) {
            if (DataRef<MARKETEER_OLD_TH_LOCK>()) {
                DataRef<MARKETEER_OLD_TH_LOCK>() = 0;
                StreamSignal* sig;
                for (sig = MarketeerGetCursor(); !sig; sig = MarketeerGetCursor()) {
                    MarketeerSetCaption(MKT_CAPTION_FETCHING);
                    MarketeerFreezeGame();
                }
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_NEW_STAGE) {
                    MarketeerSetStreamLimit(sig);
                } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                    MarketeerSetStreamLimit(nullptr);
                } else if (cmd == MARKETEER_GAME_COMPLETE) {
                    U32_REF(DATA_SCENE_ID) = 1;
                }
            }
        }
    }
    EHOOK_DY(mkt_on_scene_switch, (void*)0x4456b1)
    {
        auto now = *(uint32_t*)(pCtx->Eax + 0x15c);
        auto prev = *(uint32_t*)(pCtx->Eax + 0x158);
        if (MarketeerGetStatus() == 2) {
            if ((now == 1 && prev == 2) || (now == 6 && prev == 2)) {
                MarketeerOnQuitToMenu();
                DataRef<MARKETEER_OLD_TH_LOCK>() = 0;
            } else if (now == 2 && prev == 1) {
                DataRef<MARKETEER_OLD_TH_STARTING>() = U8_REF(DATA_STAGE);
            } 
        } else if (MarketeerGetStatus() == 1) {
            if (now == 9 && prev == 2) {
                *(uint32_t*)(pCtx->Eax + 0x15c) = 1;
            }
        }
    }
    EHOOK_DY(mkt_on_continue, (void*)0x404fa3)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerPushSignal(MARKETEER_CONTINUE);
        }
    }
    EHOOK_DY(mkt_on_quit, (void*)0x441e07)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerEndLive();
        } else if (MarketeerGetStatus() == 1) {
            MarketeerTerminate();
        }
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THMarketeerViewerHook)
    EHOOK_DY(mkt_disable_input, (void*)0x43e15c)
    {
        if (MarketeerGetStatus() == 1) {
            pCtx->Eax = 0;
        }
    }
    EHOOK_DY(mkt_skip_2, (void*)0x46a447)
    {
        pCtx->Eip = 0x46a470;
    }
    EHOOK_DY(mkt_skip_3, (void*)0x46ab43)
    {
        pCtx->Eip = 0x46ab6c;
    }
    EHOOK_DY(mkt_enter, (void*)0x46ac15)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig && sig->GetCmd() == MARKETEER_NEW_GAME) {
                uint32_t stage = (sig->param1 & 0x0000ff00) >> 8;
                uint32_t rank = (sig->param1 & 0x00ff0000) >> 16;
                uint32_t player = ((sig->param1 & 0xff000000) >> 24);
                // uint32_t subShot = ((sig->param1 & 0xff000000) >> 24) % 3;
                U8_REF(DATA_STAGE) = stage;
                U8_REF(DATA_DIFFCULTY) = rank;
                U8_REF(DATA_SHOT_TYPE) = player;
                // U32_REF(DATA_SUB_SHOT_TYPE) = subShot;
            }
        }
    }
    EHOOK_DY(mkt_intermission, (void*)0x439f3c)
    {
        pCtx->Eip = 0x439f93;
    }
    EHOOK_DY(mkt_on_continue_continue, (void*)0x404cbd)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_CONTINUE) {
                    MarketeerAdvCursor();
                    pCtx->Eip = 0x404cf3;
                    MarketeerSetCaption(MKT_CAPTION_PLAYING);
                    return;
                } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                    *(uint32_t*)(GetMemContent(pCtx->Ebp - 0x38)) = 2;
                } else {
                    MarketeerAdvCursor();
                }
            }
        }
        MarketeerSetCaption(MKT_CAPTION_WAITING_SIGNAL);
        pCtx->Eip = 0x4051d4;
    }
    EHOOK_DY(mkt_on_continue_quit, (void*)0x404e9a)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_CONTINUE) {
                    *(uint32_t*)(GetMemContent(pCtx->Ebp - 0x38)) = 1;
                    MarketeerSetCaption(MKT_CAPTION_PLAYING);
                } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                    pCtx->Eip = 0x404ec1;
                    MarketeerAdvCursor();
                    return;
                } else {
                    MarketeerAdvCursor();
                }
            }
        }
        MarketeerSetCaption(MKT_CAPTION_WAITING_SIGNAL);
        pCtx->Eip = 0x404f12;
    }
    PATCH_DY(mkt_on_continue_giveup, (void*)0x404f40, "\x01\x00\x00\x00", 4);
    PATCH_DY(mkt_on_continue_exhaust, (void*)0x4049b6, "\x01\x00\x00\x00", 4);
    HOOKSET_ENDDEF()

    HOOKSET_DEFINE(THInitHook)
    static __declspec(noinline) void THGuiCreate()
    {
        // Hooks
        auto res = MarketeerInit("th08", (void*)0x17ce700);
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
        s.th08_gui_init_1.Disable();
        s.th08_gui_init_2.Disable();
    }
    PATCH_DY(th08_disable_dataver, (void*)0x40bb80, "\x33\xc0\xc3", 3);
    PATCH_DY(th08_disable_demo, (void*)0x467aca, "\xff\xff\xff\x7f", 4);
    EHOOK_DY(th08_disable_mutex, (void*)0x44344f)
    {
        pCtx->Eip = 0x44346b;
    }
    EHOOK_DY(th08_gui_init_1, (void*)0x467960)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    EHOOK_DY(th08_gui_init_2, (void*)0x442a7a)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    HOOKSET_ENDDEF()
}

void TH08Init()
{
    MarketeerMutex();
    TH08::THInitHook::singleton().EnableAllHooks();
    TryKeepUpRefreshRate((void*)0x442591);
}
}
