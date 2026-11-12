#pragma once

#include "../common.hpp"
#include "server.hpp"

namespace Aomimi {
    // * 公式の MainUI::handleAzahar（Pretendo の規則）のあとに、AOMIMI の規則を入れる・消す
    Result handleAzahar(u8 friend_account_id);
    // * 公式の更新（3ds/nimbus/update）で公式の IPS が入ったあと、AOMIMI のアカウントなら AOMIMI 用に戻す（LLE の Azahar 用）
    void restoreIpsAfterUpdate(MainStruct *mainStruct);

    // * イベントの相手をフレンドに足す。失敗しても切り替えは止めない（イベントに出られないだけ）
    void addEventPlayers(MainStruct *mainStruct);
    // * AOMIMI に切り替えた直後（再起動の前）に足す。失敗しても切り替えは止めない
    void addEventPlayersAfterSwitch(MainStruct *mainStruct);
    // * 起動したときに 1 回だけ: AOMIMI のアカウントなら足りない相手を黙って足す。Pretendo・任天堂のアカウントなら、残っているイベントの相手を消す
    void checkEventPlayersOnce(MainStruct *mainStruct);
    // * 使用中のアカウントを選んだとき: アプリを閉じずに、もう切り替えてあることを出す
    void showAlreadySelected(MainStruct *mainStruct);

    // * フレンドのアカウントを作るときの環境（2 = Pretendo は Test、3 = AOMIMI は Dev）
    NascEnvironment environmentOf(u8 friend_account_id);

    // * --- AOMIMI のアカウントの接続先（AOMIMI Network か、別の住所か）---
    // * 起動したときに SD（3ds/aomimi/server.txt）から今の接続先を読む
    void initServer(MainStruct *mainStruct);
    bool currentAlternate();
    bool selectedAlternate();
    void selectAlternate(bool alternate);
    // * 同じ AOMIMI のアカウントのまま、接続先だけを切り替えるとき true
    bool serverChangeRequested(MainStruct *mainStruct);
    // * 接続先だけを切り替える（Azahar の規則か IPS を入れ直す。このあと再起動する）
    Result switchServer(MainStruct *mainStruct);

    // * --- 画面の部品（ui/ui.cpp。手元だけの設定で差し替えられる）---
    // * 別の接続先を選べるか、その住所
    bool hasAlternate();
    const char *alternateHost();
    // * 下の画面のボタン（AOMIMI、Pretendo、任天堂）。起動したときに 1 回、絵の位置を決める
    void placeButtons(MainStruct *mainStruct);
    void drawButtons(MainStruct *mainStruct);
    // * 触った・左右を押したときの選択。ボタンを触ったら true（決定）
    bool handleButtonInput(MainStruct *mainStruct, u32 kDown, touchPosition touch);

    // * 上の画面: 元にした Nimbus（GPL-3.0）と、Pretendo Network・任天堂とは関係がないこと
    void drawNotice();
    // * 公式の DrawScamWarning（2 行。名前が長いとはみ出す）の代わりに 3 行で
    void drawScamWarning();
    // * 公式の DrawControls の代わり。Y は出さず、X は Pretendo を選んでいるときだけ
    void drawControls(MainStruct *mainStruct);
}
