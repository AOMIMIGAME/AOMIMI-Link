// AOMIMI Link の画面の部品（下画面の 3 つのボタン、上画面の注意書き、操作の説明）。
// 手元だけの設定（app/extra.mk）で、このフォルダを別の画面の部品に差し替えられる
#include <format>
#include <string>
#include "../aomimi.hpp"
#include "../../states/MainUI.hpp"

namespace Aomimi {
    // * AOMIMI のアカウントの別の接続先。AOMIMI Link にはない（AOMIMI Network だけ）
    bool hasAlternate() {
        return false;
    }

    const char *alternateHost() {
        return HOST;
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

    void select(MainStruct *m, NascEnvironment environment) {
        m->buttonSelected = environment;
        Aomimi::selectAlternate(false);
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
            // * AOMIMI のアカウントが別の接続先に向いているときは、AOMIMI を「読み込み中」にしない（押すと AOMIMI Network に戻る）
            const bool loaded = mainStruct->currentAccount == environment && (environment != NascEnvironment::NASC_ENV_Dev || !currentAlternate());
            C2D_DrawSprite(spriteOf(mainStruct, environment, loaded, mainStruct->buttonSelected == environment));
        }
    }

    bool handleButtonInput(MainStruct *mainStruct, u32 kDown, touchPosition touch) {
        if (kDown & KEY_TOUCH) {
            for (int n = 0; n < 3; n++) {
                if (touch.px >= BUTTON_X[n] && touch.px <= BUTTON_X[n] + BUTTON_W && touch.py >= BUTTON_Y && touch.py <= BUTTON_Y + BUTTON_H) {
                    select(mainStruct, ORDER[n]);
                    return true;
                }
            }
            return false;
        }

        int index = indexOf(mainStruct->buttonSelected);
        if (kDown & KEY_LEFT) {
            select(mainStruct, ORDER[(index + 2) % 3]);
        } else if (kDown & KEY_RIGHT) {
            select(mainStruct, ORDER[(index + 1) % 3]);
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
