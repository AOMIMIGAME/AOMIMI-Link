#pragma once

// AOMIMI サーバーの設定。公式の Nimbus との違いはできるだけここと aomimi/ にまとめる
// （公式の更新を取り込むとき、ぶつかる場所を少なくするため）。

#include <3ds.h>
#include <array>

namespace Aomimi {
    // * 3 つ目のフレンドのアカウント。公式は 1 = 任天堂、2 = Pretendo を使い、環境 Dev（2）は空いている
    constexpr u8 ACCOUNT_ID = 3;

    // * Azahar の URL の置き換え先。ドメインができたら差し替える
    constexpr const char *HOST = "192.168.0.200";

    // * Azahar の置き換えの規則。ホスト名を丸ごと置き換える（IP でも動くように）
    constexpr std::array<const char *, 3> PATTERNS = {{
        "^.*nintendowifi\\.net$",
        "^.*nintendo\\.net$",
        "^.*pokemon-gl\\.com$",
    }};

    // * イベントの相手（サーバーの偽のプレイヤー）。フレンドに足すとゲームに出てくる
    constexpr std::array<u32, 2> EVENT_PLAYERS = {{ 1999999001, 1999999002 }};

    // * 実機用のプラグイン（nimbus.3gx）は配らないので、Y の「プラグインを起動」は使わない
    constexpr bool PLUGIN_ENABLED = false;
}
