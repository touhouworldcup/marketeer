#include "thprac_utils.h"

namespace THPrac {
namespace TH13 {
    void THDataInit()
    {
        AnlyDataInit();

        SwitchHackSet<uint8_t, uint8_t>(InsHookCallback, (void*)0x4772ea, 30, (void*)0x4772fb, 0x10c, (void*)0x4772f4, 0x5e);
        InsHookInit(100, (void*)0x4772e4, [](PCONTEXT pCtx) { return (pCtx->Eax + 4); });

        InsHookRegister(1, 0, 0x5410, 10101);
        InsHookRegister(1, 0, 0x5444, 10102);
        InsHookRegister(1, 0, 0x54ac, 10103);
        InsHookRegister(1, 0, 0x8c54, TH13_ST1_MID1);
        InsHookRegister(1, 0, 0x4c44, 10104);
        InsHookRegister(1, 0, 0x5544, 10105);
        InsHookRegister(1, 2, 0x868, TH13_ST1_BOSS1);
        InsHookRegister(1, 2, 0x1030, TH13_ST1_BOSS3);
        InsHookRegister(1, 2, 0x167c, TH13_ST1_BOSS5);
        InsHookRegister(1, 2, 0x1a34, TH13_ST1_BOSS6);
        InsHookRegister(1, 2, 0x1f4c, TH13_ST1_BOSS2);
        InsHookRegister(1, 2, 0x3124, TH13_ST1_BOSS4);
        InsHookRegister(1, 2, 0x4114, TH13_ST1_BOSS5);
        InsHookRegister(1, 2, 0x4a18, TH13_ST1_BOSS6);

        DataRef<DATA_SCENE_ID>(U32_ARG(0x4dcc24));
        DataRef<DATA_RND_SEED>(U16_ARG(0x4dc324));
        DataRef<DATA_DIFFCULTY>(U8_ARG(0x4be7c4));
        DataRef<DATA_SHOT_TYPE>(U8_ARG(0x4be7b8));
        DataRef<DATA_STAGE>(U8_ARG(0x4be81c));
        DataRef<DATA_STARTING_STAGE>(U8_ARG(0x4be820));
        DataRef<DATA_FRAME>(U32_ARG(0x4be828));
        DataRef<DATA_ECL_BASE>(U32_ARG(0x4c2188, 0xac, 0xC));
        DataRef<DATA_GAME_INPUT>(U16_ARG(0x4E4C08));

        DataRef<DATA_SCORE>(U32_ARG(0x4be7c0));
        DataRef<DATA_LIFE>(U32_ARG(0x4be7f4));
        DataRef<DATA_EXTEND>(U32_ARG(0x4be7fc));
        DataRef<DATA_LIFE_FRAG>(U32_ARG(0x4be7f8));
        DataRef<DATA_BOMB>(U32_ARG(0x4be800));
        DataRef<DATA_BOMB_FRAG>(U32_ARG(0x4be804));
        DataRef<DATA_POWER>(U32_ARG(0x4be7e8));
        DataRef<DATA_VALUE>(U32_ARG(0x4be7dc));
        DataRef<DATA_GRAZE>(U32_ARG(0x4be7d0));
        DataRef<DATA_TRANCE_METER>(U32_ARG(0x4be808));

        DataRef<DATA_SPIRIT_BLUE>(0x438ba6);
        DataRef<DATA_SPIRIT_GREY>(0x438c3d);
        DataRef<DATA_SPIRIT_LIFE>(0x438b5d);
        DataRef<DATA_SPIRIT_BOMB>(0x438c19);
        DataRef<DATA_SPIRIT_BLUE_TRANCE>(0x438d51);
        DataRef<DATA_SPIRIT_GREY_TRANCE>(0x438da5);
        DataRef<DATA_SPIRIT_LIFE_TRANCE>(0x438d0e);
        DataRef<DATA_SPIRIT_BOMB_TRANCE>(0x438d84);
        DataRef<DATA_ITEM_POWER>(0x42e94d);
        DataRef<DATA_ITEM_POINT>(0x42e9e5);
        DataRef<DATA_ITEM_CANCEL>(0x42eb46);

        DataBatch<DATA_SCORE, DATA_LIFE, DATA_EXTEND, DATA_LIFE_FRAG, DATA_BOMB, DATA_BOMB_FRAG,
            DATA_POWER, DATA_VALUE, DATA_GRAZE, DATA_TRANCE_METER,
            DATA_SPIRIT_BLUE, DATA_SPIRIT_GREY, DATA_SPIRIT_LIFE, DATA_SPIRIT_BOMB,
            DATA_SPIRIT_BLUE_TRANCE, DATA_SPIRIT_GREY_TRANCE, DATA_SPIRIT_LIFE_TRANCE, DATA_SPIRIT_BOMB_TRANCE,
            DATA_ITEM_POWER, DATA_ITEM_POINT, DATA_ITEM_CANCEL,
            EVENT_MISS, EVENT_BOMB, EVENT_DEATH_BOMB, EVENT_TRANCE_ACTIVE, EVENT_TRANCE_PASSIVE, EVENT_TRANCE_EXIT, EVENT_CONTINUE>();

        ImHook<EVENT_SPELLCARD_ENTER>((void*)0x413270, [](PCONTEXT pCtx) {
            AnlyEventRec(EVENT_SPELLCARD_ENTER, *(uint32_t*)(pCtx->Esp + 4), (uint32_t)U8_REF(DATA_DIFFCULTY));
            DataRef<EVENT_SPELLCARD_ENTER>()++;
        });
        ImHook<EVENT_SPELLCARD_GET>((void*)0x413acf, [](PCONTEXT pCtx) {
            AnlyEventRec(EVENT_SPELLCARD_GET, GetMemContent(0x4c2178, 0x78), (uint32_t)U8_REF(DATA_DIFFCULTY));
            DataRef<EVENT_SPELLCARD_GET>()++;
        });
        ImHook<EVENT_MISS>((void*)0x405c3e, [](PCONTEXT pCtx) {
            AnlyEventRec(EVENT_MISS);
            DataRef<EVENT_MISS>()++;
        });
        ImHook<EVENT_BOMB>((void*)0x4433f7, [](PCONTEXT pCtx) {
            AnlyEventRec(EVENT_BOMB);
            DataRef<EVENT_BOMB>()++;
        });
        ImHook<EVENT_DEATH_BOMB>((void*)0x44354c, [](PCONTEXT pCtx) {
            AnlyEventRec(EVENT_DEATH_BOMB);
            DataRef<EVENT_DEATH_BOMB>()++;
        });
        ImHook<EVENT_TRANCE_ACTIVE>((void*)0x405883, [](PCONTEXT pCtx) {
            AnlyEventRec(EVENT_TRANCE_ACTIVE);
            DataRef<EVENT_TRANCE_ACTIVE>()++;
        });
        ImHook<EVENT_TRANCE_PASSIVE>((void*)0x444a43, [](PCONTEXT pCtx) {
            AnlyEventRec(EVENT_TRANCE_PASSIVE);
            DataRef<EVENT_TRANCE_PASSIVE>()++;
        });
        ImHook<EVENT_TRANCE_EXIT>((void*)0x405c44, [](PCONTEXT pCtx) {
            if (!(GetMemContent(0x4c2164, 0x14) & 2)) {
                AnlyEventRec(EVENT_TRANCE_EXIT);
                DataRef<EVENT_TRANCE_EXIT>()++;
            }
        });
        ImHook<EVENT_CONTINUE>((void*)0x440758, [](PCONTEXT pCtx) {
            AnlyEventRec(EVENT_CONTINUE);
            DataRef<EVENT_CONTINUE>()++;
        });

        DataBatchCache<0>(true);
    }
    HOOKSET_DEFINE(THMarketeerHook)
    EHOOK_DY(mkt_skip_1, (void*)0x44d90d)
    {
        if (MarketeerGetStatus() == 2) {
            auto c = GetMemContent(0x4c22e0, 0x28);
            if (c > 1 && c < 5) {
                pCtx->Eip = 0x44dc88;
            }
        } else if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                if (sig->GetCmd() == MARKETEER_NEW_GAME) {
                    *(uint32_t*)GetMemAddr(0x4c22e0, 0x28) = 0;
                    DataRef<MARKETEER_LIMIT_LOCK>() = 0;
                    MarketeerSetCaption(MKT_CAPTION_ACCEPTING);
                    pCtx->Eip = 0x44d91d;
                } else {
                    MarketeerAdvCursor();
                    pCtx->Eip = 0x44dc88;
                }
            } else {
                MarketeerSetCaption(MKT_CAPTION_WAITING_SIGNAL);
                pCtx->Eip = 0x44dc88;
            }
        }
    }
    EHOOK_DY(mkt_stage_start, (void*)0x448ec0)
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
    EHOOK_DY(mkt_game_input, (void*)0x4480d8)
    {
        if (MarketeerGetStatus() == 2) {
            uint16_t inputBuf[3];
            inputBuf[0] = *(uint16_t*)0x4e4c08;
            inputBuf[1] = *(uint16_t*)0x4e4c14;
            inputBuf[2] = *(uint16_t*)0x4e4c18;
            MarketeerPush(inputBuf, 6);
        } else if (MarketeerGetStatus() == 1) {
            uint16_t outputBuf[3];
            auto cmd = MarketeerInputHelper(outputBuf, 6);
            if (cmd == MARKETEER_NEW_GAME) {
                U32_REF(DATA_SCENE_ID) = 10;
            } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                U32_REF(DATA_SCENE_ID) = 4;
            }
            *(uint16_t*)0x4e4c08 = outputBuf[0];
            *(uint16_t*)0x4e4c14 = outputBuf[1];
            *(uint16_t*)0x4e4c18 = outputBuf[2];
        }
    }
    EHOOK_DY(mkt_next_stage_start, (void*)0x429bee)
    {
        if (MarketeerGetStatus() == 2) {
            auto stage = DataRef<DATA_STAGE>();
            if (stage == 6 || stage == 7) {
                MarketeerOnGameComlete();
            }
        }
    }
    EHOOK_DY(mkt_on_manual_quit_normal, (void*)0x43f463)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerOnQuitToMenu();
        }
    }
    EHOOK_DY(mkt_on_manual_quit_over, (void*)0x4407e1)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerOnQuitToMenu();
        }
    }
    EHOOK_DY(mkt_on_continue, (void*)0x440687)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerPushSignal(MARKETEER_CONTINUE);
        }
    }
    EHOOK_DY(mkt_on_quit, (void*)0x45ca6f)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerEndLive();
        } else if (MarketeerGetStatus() == 1) {
            MarketeerTerminate();
        }
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THMarketeerViewerHook)
    PATCH_DY(mkt_disable_input, (void*)0x4711a0, "\x31\xc0\xc3", 3);
    PATCH_DY(mkt_skip_2, (void*)0x45127d, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_3, (void*)0x451a8c, "\x90\x90\x90\x90\x90\x90", 6);
    EHOOK_DY(mkt_enter, (void*)0x451924)
    {
        auto sig = MarketeerGetCursor();
        if (sig && sig->GetCmd() == MARKETEER_NEW_GAME) {
            uint32_t stage = (sig->param1 & 0x0000ff00) >> 8;
            uint32_t rank = (sig->param1 & 0x00ff0000) >> 16;
            uint32_t player = (sig->param1 & 0xff000000) >> 24;
            U32_REF(DATA_STAGE) = stage;
            U32_REF(DATA_STARTING_STAGE) = stage;
            U32_REF(DATA_SCENE_ID) = 7;
            *(uint32_t*)0x4c22dc = 0x4bb4f0 + stage * 124;
            U32_REF(DATA_DIFFCULTY) = rank;
            U32_REF(DATA_SHOT_TYPE) = player;
        }
    }
    EHOOK_DY(mkt_next_stage_end, (void*)0x429bf3)
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
    EHOOK_DY(mkt_intermission, (void*)0x42bf13)
    {
        if (MarketeerGetStatus() == 1) {
            StreamSignal* sig;
            for (sig = MarketeerGetCursor(); DataRef<MARKETEER_LIMIT_LOCK>() || !sig; sig = MarketeerGetCursor()) {
                Sleep(1);
            }
        }
    }
    EHOOK_DY(mkt_on_continue_1, (void*)0x43fc84)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_CONTINUE) {
                    MarketeerAdvCursor();
                    pCtx->Eip = 0x43fc94;
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
        pCtx->Eip = 0x43fd91;
    }
    EHOOK_DY(mkt_on_continue_2, (void*)0x440400)
    {
        pCtx->Eip = 0x440520;
    }
    EHOOK_DY(mkt_on_continue_3, (void*)0x44041e)
    {
        pCtx->Ecx = 90;
    }
    EHOOK_DY(mkt_skip_ed, (void*)0x42c663)
    {
        if (MarketeerGetStatus() == 1) {
            pCtx->Edx = 4;
        }
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THInitHook)
    static __declspec(noinline) void THGuiCreate()
    {
        // Hooks
        auto res = MarketeerInit("th13", (void*)0x4dd0a8);
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
        s.th13_gui_init_1.Disable();
        s.th13_gui_init_2.Disable();
    }
    PATCH_DY(th13_disable_demo, (void*)0x44c5a1, "\xff\xff\xff\x7f", 4);
    EHOOK_DY(th13_disable_mutex, (void*)0x45c1bc)
    {
        pCtx->Eip = 0x45c340;
    }
    PATCH_DY(th13_startup_1, (void*)0x44c107, "\xeb", 1);
    PATCH_DY(th13_startup_2, (void*)0x44cdb7, "\xeb", 1);
    EHOOK_DY(th13_gui_init_1, (void*)0x44d4fd)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    EHOOK_DY(th13_gui_init_2, (void*)0x45e1d5)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    HOOKSET_ENDDEF()
}

void TH13Init()
{
    MarketeerMutex();
    TH13::THInitHook::singleton().EnableAllHooks();
}
}