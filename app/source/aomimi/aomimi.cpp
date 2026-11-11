#include <format>
#include <string>
#include "aomimi.hpp"
#include "../states/MainUI.hpp"
#include "../sysmodules/httpc.hpp"

namespace {
    constexpr Result ResultAlreadyExists = MAKERESULT(RL_STATUS, RS_INVALIDARG, RM_HTTP, RD_ALREADY_EXISTS);
    constexpr Result ResultNoAzaharCommand = MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_OS, 47);

    bool isAzahar() {
        s64 emu_id = 0;
        svcGetSystemInfo(&emu_id, 0x20000, 0);
        return emu_id == 2;
    }

    Result registerRule(const std::string &pattern, const std::string &replacement) {
        Result res = HTTPC_Azahar_RegisterURLReplacement(pattern.c_str(), replacement.c_str(), pattern.size(), replacement.size());
        if (res == ResultAlreadyExists) {
            res = HTTPC_Azahar_UnregisterURLReplacement(pattern.c_str(), pattern.size());
            if (R_SUCCEEDED(res)) {
                res = HTTPC_Azahar_RegisterURLReplacement(pattern.c_str(), replacement.c_str(), pattern.size(), replacement.size());
            }
        }
        return res;
    }
}

namespace Aomimi {
    NascEnvironment environmentOf(u8 friend_account_id) {
        if (friend_account_id == ACCOUNT_ID) {
            return NascEnvironment::NASC_ENV_Dev;
        }
        return friend_account_id == 2 ? NascEnvironment::NASC_ENV_Test : NascEnvironment::NASC_ENV_Prod;
    }

    Result handleAzahar(u8 friend_account_id) {
        // * 公式の処理: アカウント 2 なら Pretendo の規則を入れ、それ以外なら消す
        Result res = MainUI::handleAzahar(friend_account_id);
        if (R_FAILED(res) || !isAzahar()) {
            return res;
        }

        res = httpcInit(0x1000);
        if (R_FAILED(res)) {
            return res;
        }

        for (const char *pattern : PATTERNS) {
            if (friend_account_id == ACCOUNT_ID) {
                res = registerRule(pattern, HOST);
            } else {
                // * Pretendo や任天堂を選んだら、AOMIMI の規則が先に当たらないように消す
                res = HTTPC_Azahar_UnregisterURLReplacement(pattern, std::string(pattern).size());
                if (R_SUCCEEDED(res)) {
                    res = 0; // * 登録されていなかったという値が返ることがある
                }
            }
            if (R_FAILED(res)) {
                break;
            }
        }

        if (res == ResultNoAzaharCommand) {
            res = 0; // * LLE の HTTP では Azahar の命令がない
        }
        httpcExit();
        return res;
    }

    void addEventPlayers(MainStruct *mainStruct) {
        constexpr s64 waitNanoseconds = 15000000000LL;

        Handle event = 0;
        Result rc = svcCreateEvent(&event, RESET_ONESHOT);
        if (R_FAILED(rc)) {
            LOG_NIMBUS_ERROR(mainStruct, std::format("Event players: event failed {:08X}", (u32)rc).c_str());
            return;
        }

        bool online = false;
        FRD_IsOnline(&online);
        if (!online) {
            rc = FRD_Login(event);
            if (R_SUCCEEDED(rc)) {
                rc = svcWaitSynchronization(event, waitNanoseconds);
            }
            FRD_IsOnline(&online);
        }

        if (!online) {
            LOG_NIMBUS_ERROR(mainStruct, std::format("Event players: not online {:08X}", (u32)rc).c_str());
            svcCloseHandle(event);
            return;
        }

        bool added = false;
        for (const u32 pid : EVENT_PLAYERS) {
            u64 friendCode = 0;
            bool known = false;
            if (R_FAILED(FRD_PrincipalIdToFriendCode(pid, &friendCode)) || R_FAILED(FRD_IsInFriendList(friendCode, &known)) || known) {
                continue;
            }

            svcClearEvent(event);
            rc = FRDA_AddFriendOnline(event, pid);
            if (R_SUCCEEDED(rc)) {
                rc = svcWaitSynchronization(event, waitNanoseconds);
            }
            if (R_FAILED(rc)) {
                LOG_NIMBUS_ERROR(mainStruct, std::format("Event players: add failed {:08X}", (u32)rc).c_str());
                continue;
            }
            added = true;
        }

        if (added) {
            rc = FRDA_Save();
            if (R_FAILED(rc)) {
                LOG_NIMBUS_ERROR(mainStruct, std::format("Event players: save failed {:08X}", (u32)rc).c_str());
            }
        }

        svcCloseHandle(event);
    }
}

namespace {
    // * 左から AOMIMI、Pretendo、任天堂。公式の 2 つのボタン（幅 104）を 0.9 倍にして 3 つ並べる
    constexpr NascEnvironment ORDER[3] = { NascEnvironment::NASC_ENV_Dev, NascEnvironment::NASC_ENV_Test, NascEnvironment::NASC_ENV_Prod };
    constexpr float SCALE = 0.9f;
    constexpr float BUTTON_X[3] = { 10.0f, 113.0f, 216.0f };
    constexpr float BUTTON_Y = 64.0f;
    constexpr float BUTTON_W = 104.0f * SCALE;
    constexpr float BUTTON_H = 113.0f * SCALE;

    // * [環境][loaded][selected]
    C2D_Sprite *spriteOf(MainStruct *m, NascEnvironment environment, bool loaded, bool selected) {
        switch (environment) {
        case NascEnvironment::NASC_ENV_Dev:
            return loaded ? (selected ? &m->aomimi_loaded_selected : &m->aomimi_loaded_deselected)
                          : (selected ? &m->aomimi_unloaded_selected : &m->aomimi_unloaded_deselected);
        case NascEnvironment::NASC_ENV_Test:
            return loaded ? (selected ? &m->pretendo_loaded_selected : &m->pretendo_loaded_deselected)
                          : (selected ? &m->pretendo_unloaded_selected : &m->pretendo_unloaded_deselected);
        default:
            return loaded ? (selected ? &m->nintendo_loaded_selected : &m->nintendo_loaded_deselected)
                          : (selected ? &m->nintendo_unloaded_selected : &m->nintendo_unloaded_deselected);
        }
    }

    int indexOf(NascEnvironment environment) {
        for (int n = 0; n < 3; n++) {
            if (ORDER[n] == environment) {
                return n;
            }
        }
        return 2;
    }
}

namespace Aomimi {
    void placeButtons(MainStruct *mainStruct) {
        for (int n = 0; n < 3; n++) {
            for (int loaded = 0; loaded < 2; loaded++) {
                for (int selected = 0; selected < 2; selected++) {
                    C2D_Sprite *sprite = spriteOf(mainStruct, ORDER[n], loaded, selected);
                    C2D_SpriteSetScale(sprite, SCALE, SCALE);
                    C2D_SpriteSetPos(sprite, BUTTON_X[n], BUTTON_Y);
                }
            }
        }
    }

    void drawButtons(MainStruct *mainStruct) {
        for (const NascEnvironment environment : ORDER) {
            C2D_DrawSprite(spriteOf(mainStruct, environment, mainStruct->currentAccount == environment, mainStruct->buttonSelected == environment));
        }
    }

    bool handleButtonInput(MainStruct *mainStruct, u32 kDown, touchPosition touch) {
        if (kDown & KEY_TOUCH) {
            for (int n = 0; n < 3; n++) {
                if (touch.px >= BUTTON_X[n] && touch.px <= BUTTON_X[n] + BUTTON_W && touch.py >= BUTTON_Y && touch.py <= BUTTON_Y + BUTTON_H) {
                    mainStruct->buttonSelected = ORDER[n];
                    return true;
                }
            }
            return false;
        }

        int index = indexOf(mainStruct->buttonSelected);
        if (kDown & KEY_LEFT) {
            mainStruct->buttonSelected = ORDER[(index + 2) % 3];
        } else if (kDown & KEY_RIGHT) {
            mainStruct->buttonSelected = ORDER[(index + 1) % 3];
        }
        return false;
    }
}

namespace Aomimi {
    void drawNotice() {
        // * 真ん中の絵（高さ 76、中心 120）と、下の注意書き（2 行）のあいだに置く
        static const char *lines[2] = {
            "Based on Nimbus by Pretendo Network (GPL-3.0)",
            "Not affiliated with Pretendo Network or Nintendo",
        };
        const float size = 0.42f;
        float y = 162.0f;
        C2D_TextBufClear(textBuf); // * 公式の文字の描き方と同じく、使う前に空にする
        for (const char *line : lines) {
            C2D_Text text;
            C2D_TextFontParse(&text, font, textBuf, line);
            C2D_TextOptimize(&text);
            C2D_DrawText(&text, C2D_WithColor | C2D_AlignCenter, 200.0f, y, 0.5f, size, size, C2D_Color32(170, 175, 190, 0xFF));
            y += GetStringHeight(size, line);
        }
    }
}

namespace Aomimi {
    void drawScamWarning() {
        const std::string lines[3] = {
            std::format("{} is free and open-source.", APP_TITLE),
            "If you paid to have it installed, you have been scammed.",
            "Demand your money back",
        };
        const float size = 0.45f;
        const float offset = 6.0f;
        float height = 0.0f;
        for (const auto &line : lines) {
            height += GetStringHeight(size, line.c_str());
        }

        C2D_TextBufClear(textBuf);
        float y = (240 + 2) - offset - height;
        for (const auto &line : lines) {
            C2D_Text text;
            C2D_TextFontParse(&text, font, textBuf, line.c_str());
            C2D_TextOptimize(&text);
            C2D_DrawText(&text, C2D_WithColor | C2D_AlignLeft, offset, y, 0.5f, size, size, C2D_Color32(255, 255, 255, 0xFF));
            y += GetStringHeight(size, line.c_str());
        }
    }

    void drawControls(MainStruct *mainStruct) {
        const char *text = mainStruct->buttonSelected == NascEnvironment::NASC_ENV_Test ? "A: Select\nX: Unlink PNID" : "A: Select";
        const float size = 0.5f;
        const float offset = 6.0f;
        const float bottom = (240 + 2) - offset;

        C2D_TextBufClear(textBuf);
        C2D_Text c2d_text;
        C2D_TextFontParse(&c2d_text, font, textBuf, text);
        C2D_TextOptimize(&c2d_text);
        C2D_DrawText(&c2d_text, C2D_WithColor | C2D_AlignLeft, offset, bottom - GetStringHeight(size, text), 0.5f, size, size, C2D_Color32(255, 255, 255, 0xFF));
    }
}
