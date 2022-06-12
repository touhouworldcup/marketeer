#include "thprac_utils.h"

namespace THPrac {
namespace TH18 {
    void THDataInit()
    {
        AnlyDataInit();

        DataRef<DATA_SCENE_ID>(U32_ARG(0x4cd5e8));
        DataRef<DATA_RND_SEED>(U16_ARG(0x4cf288));
        DataRef<DATA_DIFFCULTY>(U8_ARG(0x4ccd00));
        DataRef<DATA_SHOT_TYPE>(U8_ARG(0x4cccf4));
        DataRef<DATA_STAGE>(U8_ARG(0x4cccdc));
        DataRef<DATA_STARTING_STAGE>(U8_ARG(0x4ccce0));
    }
    void MktNextStageEnd()
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
    HOOKSET_DEFINE(THMarketeerHook)
    EHOOK_DY(mkt_skip_1, (void*)0x465f3d)
    {
        DWORD skipAddress = 0x466203;
        DWORD enterAddress = 0x465f48;
        if (MarketeerGetStatus() == 2) {
            auto c = GetMemContent(0x4cf43c, 0x24);
            if (c > 1 && c < 5) {
                pCtx->Eip = skipAddress;
            }
        } else if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                if (sig->GetCmd() == MARKETEER_NEW_GAME) {
                    *(uint32_t*)GetMemAddr(0x4cf43c, 0x24) = 0;
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
    EHOOK_DY(mkt_stage_start, (void*)0x462ea0)
    {
        if (MarketeerGetStatus() == 2) {
            uint32_t stage = DataRef<DATA_STAGE>();
            uint32_t startingStage = DataRef<DATA_STARTING_STAGE>();
            uint32_t player = DataRef<DATA_SHOT_TYPE>();
            uint32_t rank = DataRef<DATA_DIFFCULTY>();
            uint16_t seed = U16_REF(DATA_RND_SEED);

            uint8_t cardList[8] {};
            uint32_t j = 0;
            uint32_t* list = nullptr;
            for (uint32_t* i = (uint32_t*)GetMemContent(0x4cf298, 0x1c); i && j < 8; i = (uint32_t*)i[1]) {
                list = i;
                cardList[j++] = ((uint32_t**)list)[0][1];
            }
            uint32_t cardA = *(uint32_t*)(&cardList[0]);
            uint32_t cardB = *(uint32_t*)(&cardList[4]);

            MarketeerOnStageStart(stage == startingStage, stage, rank, player, seed, cardA, cardB);

        } else if (MarketeerGetStatus() == 1) {
            StreamSignal* sig;
            uint16_t outputBuf[3];
            for (sig = MarketeerGetCursor(); DataRef<MARKETEER_LIMIT_LOCK>() || !sig; sig = MarketeerGetCursor()) {
                MarketeerFreezeGame();
            }
            auto cmd = sig->GetCmd();
            if (cmd == MARKETEER_NEW_GAME || cmd == MARKETEER_NEW_STAGE) {
                if (cmd == MARKETEER_NEW_GAME) {
                    uint8_t cardList[8] {};
                    *(uint32_t*)(&cardList[0]) = sig->param4;
                    *(uint32_t*)(&cardList[4]) = sig->param5;
                    for (auto& card : cardList) {
                        if (card) {
                            uint32_t cardStruct = *(uint32_t*)0x4cf298;
                            uint32_t cardId = card;
                            __asm {
                                push 2;
                                push cardId;
                                mov ecx, cardStruct;
                                mov eax, 0x411460;
                                call eax;
                            }
                        }
                    }
                }
                U16_REF(DATA_RND_SEED) = (uint16_t)sig->param3;
                DataRef<MARKETEER_FRAME_REC_1>() = 0;
                MarketeerAlignStream(sig);
                while (!MarketeerReadStream(0, 6, outputBuf)) {
                    MarketeerFreezeGame();
                }
                MarketeerAdvCursor();
            }
            DataRef<MARKETEER_LOADING_LOCK>() = 0;
        }
    }
    EHOOK_DY(mkt_game_input, (void*)0x462966)
    {
        if (!GetMemContent(0x4cf2a4)) {
            if (MarketeerGetStatus() == 2) {
                uint16_t inputBuf[3];
                inputBuf[0] = *(uint16_t*)0x4ca428;
                inputBuf[1] = *(uint16_t*)0x4ca434;
                inputBuf[2] = *(uint16_t*)0x4ca438;
                MarketeerPush(inputBuf, 6);
            } else if (MarketeerGetStatus() == 1) {
                uint16_t outputBuf[3] {};
                if (!DataRef<MARKETEER_LOADING_LOCK>()) {
                    auto cmd = MarketeerInputHelper(outputBuf, 6);
                    if (cmd == MARKETEER_NEW_GAME) {
                        U32_REF(DATA_SCENE_ID) = 10;
                    } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                        U32_REF(DATA_SCENE_ID) = 4;
                    }
                }
                *(uint16_t*)0x4ca428 = outputBuf[0];
                *(uint16_t*)0x4ca434 = outputBuf[1];
                *(uint16_t*)0x4ca438 = outputBuf[2];
            }
        }
    }
    EHOOK_DY(mkt_next_stage_start, (void*)0x444650)
    {
        if (MarketeerGetStatus() == 2) {
            auto stage = DataRef<DATA_STAGE>();
            if (stage == 6 || stage == 7) {
                MarketeerOnGameComlete();
            }
        }
    }
    EHOOK_DY(mkt_on_scene_switch, (void*)0x455057)
    {
        if (MarketeerGetStatus() == 2) {
            if (*(uint32_t*)(pCtx->Esi + 0x7f8) == 4 && *(uint32_t*)(pCtx->Esi + 0x7f4) == 7) {
                MarketeerOnQuitToMenu();
            }
        } else if (MarketeerGetStatus() == 1) {
            if (*(uint32_t*)(pCtx->Esi + 0x7f8) == 15) {
                *(uint32_t*)(pCtx->Esi + 0x7f8) = pCtx->Eax = 4;
            } else if (*(uint32_t*)(pCtx->Esi + 0x7f8) == 10) {
                DataRef<MARKETEER_LOADING_LOCK>() = 1;
            }
        }
    }
    EHOOK_DY(mkt_on_continue, (void*)0x45a2df)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerPushSignal(MARKETEER_CONTINUE);
        }
    }
    EHOOK_DY(mkt_on_quit, (void*)0x47226e)
    {
        if (MarketeerGetStatus() == 2) {
            MarketeerEndLive();
        } else if (MarketeerGetStatus() == 1) {
            MarketeerTerminate();
        }
    }
    EHOOK_DY(mkt_th18_on_buy_card, (void*)0x41845b)
    {
        if (MarketeerGetStatus() == 2) {
            uint32_t eax = GetMemContent(pCtx->Edi + 0xe4);
            uint32_t ecx = GetMemContent(pCtx->Edi + eax * 4 + 0xa30);
            MarketeerPushSignal(MARKETEER_TH18_CARD, GetMemContent(ecx + 4), pCtx->Eax);
        }
    }
    EHOOK_DY(mkt_th18_on_blank_card, (void*)0x4185a4)
    {
        if (MarketeerGetStatus() == 2) {
            uint8_t cardList[16] {};
            uint32_t cardListIdx = 0;

            uint32_t v49 = 0;
            uint32_t v50 = pCtx->Edi + 0xa30;
            uint32_t v95 = pCtx->Edi + 0xa30;
            uint32_t v51, v53, v96;
            do {
                v51 = GetMemContent(v50);
                bool v52 = GetMemContent(v51 + 0x1c) == 0;
                v53 = GetMemContent(v51 + 4);
                v96 = v53;
                if (v52) {
                    cardList[cardListIdx++] = v53;
                }
                ++v49;
                v50 = v95 + 4;
                v95 = v95 + 4;
            } while (v49 < GetMemContent(pCtx->Edi + 0xa2c));

            MarketeerPushSignal(MARKETEER_TH18_BLANK, *(uint32_t*)(&(cardList[0])), *(uint32_t*)(&(cardList[4])),
                *(uint32_t*)(&(cardList[8])), *(uint32_t*)(&(cardList[12])));
        }
    }
    HOOKSET_ENDDEF()
    HOOKSET_DEFINE(THMarketeerViewerHook)
    EHOOK_DY(mkt_th18_shop_escape_1, (void*)0x4181f9)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_TH18_CARD) {
                    {
                        uint32_t cardStruct = *(uint32_t*)0x4cf298;
                        uint32_t cardId = sig->param2;
                        __asm {
                            push 2;
                            push cardId;
                            mov ecx, cardStruct;
                            mov eax, 0x411460;
                            call eax;
                        }
                    }
                    pCtx->Eip = 0x4183d9;
                    MarketeerSetCaption(MKT_CAPTION_PLAYING);
                    return;
                } else if (cmd == MARKETEER_NEW_GAME) {
                    MarketeerAdvCursor();
                    U32_REF(DATA_SCENE_ID) = 10;
                } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                    MarketeerAdvCursor();
                    U32_REF(DATA_SCENE_ID) = 4;
                } else {
                    MarketeerAdvCursor();
                }
            }
        }
        MarketeerSetCaption(MKT_CAPTION_WAITING_SIGNAL);
        pCtx->Eip = 0x4184a0;
    }
    EHOOK_DY(mkt_th18_shop_escape_2, (void*)0x418402)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig && sig->GetCmd() == MARKETEER_TH18_CARD) {
                pCtx->Eax = sig->param3;
                pCtx->Ecx = GetMemContent(0x4ccd34);
                if (pCtx->Ecx < pCtx->Eax) {
                    pCtx->Eip = 0x41846f;
                } else {
                    pCtx->Eip = 0x418465;
                }
                MarketeerAdvCursor();
                return;
            }
        }
    }
    EHOOK_DY(mkt_th18_blank_card_skip_1, (void*)0x41855b)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_TH18_BLANK) {
                    pCtx->Eip = 0x41856b;
                    MarketeerSetCaption(MKT_CAPTION_PLAYING);
                    return;
                } else if (cmd == MARKETEER_NEW_GAME) {
                    MarketeerAdvCursor();
                    U32_REF(DATA_SCENE_ID) = 10;
                } else if (cmd == MARKETEER_QUIT_TO_MENU) {
                    MarketeerAdvCursor();
                    U32_REF(DATA_SCENE_ID) = 4;
                } else {
                    MarketeerAdvCursor();
                }
            }
        }
    }
    EHOOK_DY(mkt_th18_blank_card_skip_2, (void*)0x418596)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig->GetCmd() == MARKETEER_TH18_BLANK) {
                uint8_t cardList[16] {};
                *(uint32_t*)(&(cardList[0])) = sig->param2;
                *(uint32_t*)(&(cardList[4])) = sig->param3;
                *(uint32_t*)(&(cardList[8])) = sig->param4;
                *(uint32_t*)(&(cardList[12])) = sig->param5;
                for (auto& card : cardList) {
                    if (card) {
                        uint32_t cardStruct = *(uint32_t*)0x4cf298;
                        uint32_t cardId = card;

                        __asm {
                            push 2;
                            push cardId;
                            mov ecx, cardStruct;
                            mov eax, 0x411460;
                            call eax;
                        }
                    }
                }
                MarketeerAdvCursor();
            }
        }
        pCtx->Eip = 0x4185e8;
    }
    EHOOK_DY(mkt_th18_blank_card_skip_3, (void*)0x4185ed)
    {
        pCtx->Eip = 0x4185fd;
    }
    EHOOK_DY(mkt_th18_block_cards, (void*)0x407eac)
    {
        pCtx->Eip = 0x407f07;
    }
    EHOOK_DY(mkt_disable_input, (void*)0x401c50)
    {
        if (MarketeerGetStatus() == 1) {
            pCtx->Eax = 0;
            pCtx->Eip = 0x401e3a;
        }
    }
    PATCH_DY(mkt_skip_2, (void*)0x466589, "\x90\x90\x90\x90\x90\x90", 6);
    PATCH_DY(mkt_skip_3, (void*)0x466d19, "\x90\x90\x90\x90\x90\x90", 6);
    EHOOK_DY(mkt_enter, (void*)0x467140)
    {
        auto sig = MarketeerGetCursor();
        if (sig && sig->GetCmd() == MARKETEER_NEW_GAME) {
            uint32_t stage = (sig->param1 & 0x0000ff00) >> 8;
            uint32_t rank = (sig->param1 & 0x00ff0000) >> 16;
            uint32_t player = ((sig->param1 & 0xff000000) >> 24);
            U32_REF(DATA_STAGE) = stage;
            U32_REF(DATA_STARTING_STAGE) = stage;
            U32_REF(DATA_SCENE_ID) = 7;
            *(uint32_t*)0x4cf428 = 0x4c9410 + stage * 0xd4;
            U32_REF(DATA_DIFFCULTY) = rank;
            U32_REF(DATA_SHOT_TYPE) = player;

            DataRef<MARKETEER_LOADING_LOCK>() = 1;
        }
    }
    EHOOK_DY(mkt_next_stage_end_1, (void*)0x4448b7)
    {
        MktNextStageEnd();
    }
    EHOOK_DY(mkt_next_stage_end_2, (void*)0x444ce4)
    {
        MktNextStageEnd();
    }
    EHOOK_DY(mkt_next_stage_end_3, (void*)0x444b81)
    {
        MktNextStageEnd();
    }
    EHOOK_DY(mkt_next_stage_end_4, (void*)0x444d57)
    {
        MktNextStageEnd();
    }
    EHOOK_DY(mkt_intermission, (void*)0x44314e)
    {
        if (MarketeerGetStatus() == 1) {
            StreamSignal* sig;
            for (sig = MarketeerGetCursor(); DataRef<MARKETEER_LIMIT_LOCK>() || !sig; sig = MarketeerGetCursor()) {
                Sleep(1);
            }
        }
    }
    EHOOK_DY(mkt_on_continue_1, (void*)0x459262)
    {
        if (MarketeerGetStatus() == 1) {
            auto sig = MarketeerGetCursor();
            if (sig) {
                auto cmd = sig->GetCmd();
                if (cmd == MARKETEER_CONTINUE) {
                    MarketeerAdvCursor();
                    pCtx->Eip = 0x459268;
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
        pCtx->Eip = 0x45949f;
    }
    EHOOK_DY(mkt_on_continue_2, (void*)0x459943)
    {
        pCtx->Eip = 0x459949;
    }
    EHOOK_DY(mkt_on_continue_3, (void*)0x45994b)
    {
        pCtx->Eax = 90;
    }
    HOOKSET_ENDDEF()
    
    HOOKSET_DEFINE(THInitHook)
    static __declspec(noinline) void THGuiCreate()
    {
        // Hooks
        auto res = MarketeerInit("th18", (void*)0x568c30);
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
        s.th18_gui_init_1.Disable();
        s.th18_gui_init_2.Disable();
    }
    PATCH_DY(th18_disable_demo, (void*)0x464f7e, "\xff\xff\xff\x7f", 4);
    EHOOK_DY(th18_disable_mutex, (void*)0x474435)
    {
        pCtx->Eip = 0x47445d;
    }
    EHOOK_DY(th18_disable_topmost, (void*)0x4722c7)
    {
        RECT rect;
        GetWindowRect(*(HWND*)0x568c30, &rect);
        if (rect.right != GetSystemMetrics(SM_CXSCREEN) || rect.bottom != GetSystemMetrics(SM_CYSCREEN) || rect.left != 0 || rect.top != 0) {
            pCtx->Eip = 0x4722f2;
        }
    }
    PATCH_DY(th18_startup_1, (void*)0x464c4f, "\x90\x90", 2);
    PATCH_DY(th18_startup_2, (void*)0x465bb0, "\xeb", 1);
    EHOOK_DY(th18_gui_init_1, (void*)0x465ce5)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    EHOOK_DY(th18_gui_init_2, (void*)0x4740c0)
    {
        THGuiCreate();
        THInitHookDisable();
    }
    HOOKSET_ENDDEF()
}

void TH18Init()
{
    MarketeerMutex();
    TH18::THInitHook::singleton().EnableAllHooks();
}
}