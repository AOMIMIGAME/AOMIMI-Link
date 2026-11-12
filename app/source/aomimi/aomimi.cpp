#include <cstdio>
#include <dirent.h>
#include <format>
#include <string>
#include <sys/stat.h>
#include <vector>
#include "aomimi.hpp"
#include "../states/MainUI.hpp"
#include "../sysmodules/httpc.hpp"

namespace {
    // * イベントの相手をフレンドに足す。login なら必要に応じてログインする。report なら失敗を画面の文に残す
    void addPlayers(MainStruct *mainStruct, bool login, bool report) {
        constexpr s64 waitNanoseconds = 15000000000LL;

        Handle event = 0;
        Result rc = svcCreateEvent(&event, RESET_ONESHOT);
        if (R_FAILED(rc)) {
            if (report) {
                LOG_NIMBUS_ERROR(mainStruct, std::format("Event players: event failed {:08X}", (u32)rc).c_str());
            }
            return;
        }

        bool online = false;
        FRD_IsOnline(&online);
        if (!online && login) {
            rc = FRD_Login(event);
            if (R_SUCCEEDED(rc)) {
                rc = svcWaitSynchronization(event, waitNanoseconds);
            }
            FRD_IsOnline(&online);
        }

        if (!online) {
            if (report) {
                LOG_NIMBUS_ERROR(mainStruct, std::format("Event players: not online {:08X}", (u32)rc).c_str());
            }
            svcCloseHandle(event);
            return;
        }

        bool added = false;
        for (const u32 pid : Aomimi::EVENT_PLAYERS) {
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
                if (report) {
                    LOG_NIMBUS_ERROR(mainStruct, std::format("Event players: add failed {:08X}", (u32)rc).c_str());
                }
                continue;
            }
            added = true;
        }

        if (added) {
            rc = FRDA_Save();
            if (R_FAILED(rc) && report) {
                LOG_NIMBUS_ERROR(mainStruct, std::format("Event players: save failed {:08X}", (u32)rc).c_str());
            }
        }

        svcCloseHandle(event);
    }

    // * 今のアカウントのフレンド一覧から、イベントの相手だけを消す（ほかのフレンドには触らない）
    void removeEventPlayers() {
        constexpr u32 limit = 100; // * 3DS のフレンドは 100 人まで
        static FriendKey keys[limit];
        u32 count = 0;
        if (R_FAILED(FRD_GetFriendKeyList(keys, &count, 0, limit))) {
            return;
        }

        bool removed = false;
        for (u32 n = 0; n < count && n < limit; n++) {
            for (const u32 pid : Aomimi::EVENT_PLAYERS) {
                if (keys[n].principalId == pid && R_SUCCEEDED(FRDA_RemoveFriend(pid, keys[n].localFriendCode))) {
                    removed = true;
                }
            }
        }
        if (removed) {
            FRDA_Save();
        }
    }

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

    // * --- LLE の Azahar（「オンラインに必要な LLE モジュール」が有効）---
    // * HLE の URL の置き換えが使えないので、本物の 3DS と同じく luma/sysmodules の IPS で接続先を決める。
    // * IPS はビルドのときに公式 Nimbus のリリースから作り、romfs に入れてある（app/aomimi/make_ips.py）
    constexpr const char *IPS_AOMIMI = "romfs:/aomimi_ips/aomimi/";
    constexpr const char *IPS_PRETENDO = "romfs:/aomimi_ips/pretendo/";
    constexpr const char *IPS_ALT = "romfs:/aomimi_ips/alt/"; // * 別の住所の組（あるときだけ）
    constexpr const char *IPS_TARGET = "sdmc:/luma/sysmodules/";
    constexpr const char *IPS_BACKUP = "sdmc:/3ds/aomimi/ips-backup/";
    constexpr Result ResultIpsMissing = MAKERESULT(RL_PERMANENT, RS_NOTFOUND, RM_APPLICATION, RD_NOT_FOUND);
    constexpr Result ResultIpsWrite = MAKERESULT(RL_PERMANENT, RS_CANCELED, RM_APPLICATION, RD_NO_DATA);

    bool readFile(const std::string &path, std::vector<u8> &data) {
        FILE *f = fopen(path.c_str(), "rb");
        if (!f) {
            return false;
        }
        data.clear();
        u8 buffer[1024];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0) {
            data.insert(data.end(), buffer, buffer + n);
        }
        const bool ok = !ferror(f);
        fclose(f);
        return ok;
    }

    bool writeFile(const std::string &path, const std::vector<u8> &data) {
        FILE *f = fopen(path.c_str(), "wb");
        if (!f) {
            return false;
        }
        const bool ok = fwrite(data.data(), 1, data.size(), f) == data.size();
        return fclose(f) == 0 && ok;
    }

    std::vector<std::string> listIps(const char *dir) {
        std::vector<std::string> names;
        DIR *d = opendir(dir);
        if (!d) {
            return names;
        }
        while (dirent *entry = readdir(d)) {
            const std::string name = entry->d_name;
            if (name.size() > 4 && name.ends_with(".ips")) {
                names.push_back(name);
            }
        }
        closedir(d);
        return names;
    }

    // * dir の組（AOMIMI 用、別の住所用、公式）の IPS を書く。
    // * 公式の Nimbus も Nintendo のときは IPS を残したままにしている（act の IPS が任天堂の URL を選ぶ）
    Result installIps(const char *dir) {
        const std::vector<std::string> names = listIps(dir);
        if (names.empty()) {
            return ResultIpsMissing;
        }
        mkdir("sdmc:/luma", 0777);
        mkdir(IPS_TARGET, 0777);

        for (const std::string &name : names) {
            std::vector<u8> wanted, current;
            if (!readFile(dir + name, wanted)) {
                return ResultIpsMissing;
            }
            const std::string target = IPS_TARGET + name;
            if (readFile(target, current)) {
                if (current == wanted) {
                    continue;
                }
                // * どの組とも違う IPS（自分で入れたものなど）は、最初の 1 回だけ控えてから書く
                bool known = false;
                for (const char *other : { IPS_AOMIMI, IPS_PRETENDO, IPS_ALT }) {
                    std::vector<u8> data;
                    if (readFile(other + name, data) && data == current) {
                        known = true;
                    }
                }
                const std::string backup = IPS_BACKUP + name;
                FILE *exists = fopen(backup.c_str(), "rb");
                if (exists) {
                    fclose(exists);
                } else if (!known) {
                    mkdir("sdmc:/3ds", 0777);
                    mkdir("sdmc:/3ds/aomimi", 0777);
                    mkdir(IPS_BACKUP, 0777);
                    if (!writeFile(backup, current)) {
                        return ResultIpsWrite;
                    }
                }
            }
            if (!writeFile(target, wanted)) {
                return ResultIpsWrite;
            }
        }
        return 0;
    }

    // * --- AOMIMI のアカウント（slot 3）の接続先: AOMIMI Network か、別の住所（alternateHost）か ---
    // * 同じアカウントのまま接続先だけを変えられるように、今どちらに向いているかを SD に覚えておく。
    // * 別の住所に向いていたら、AOMIMI を選んだときに AOMIMI Network へ戻す
    constexpr const char *SERVER_FILE = "sdmc:/3ds/aomimi/server.txt";
    bool currentAlt = false; // * 今の接続先が別の住所
    bool selectedAlt = false; // * 選んでいるボタンが別の住所

    bool readCurrentAlt() {
        std::vector<u8> data;
        if (!readFile(SERVER_FILE, data)) {
            return false;
        }
        const std::string text(data.begin(), data.end());
        return !text.starts_with("aomimi");
    }

    void writeCurrentAlt(bool alt) {
        mkdir("sdmc:/3ds", 0777);
        mkdir("sdmc:/3ds/aomimi", 0777);
        const std::string text = alt ? "alt\n" : "aomimi\n";
        writeFile(SERVER_FILE, std::vector<u8>(text.begin(), text.end()));
        currentAlt = alt;
    }
}

namespace Aomimi {
    bool currentAlternate() {
        return currentAlt;
    }

    bool selectedAlternate() {
        return selectedAlt;
    }

    void selectAlternate(bool alt) {
        selectedAlt = alt;
    }

    void initServer(MainStruct *mainStruct) {
        // * 起動したとき: 今の接続先を読み、選んでいるボタンも今の接続先に合わせる
        currentAlt = readCurrentAlt();
        selectedAlt = hasAlternate() && mainStruct->buttonSelected == NascEnvironment::NASC_ENV_Dev && currentAlt;
    }

    bool serverChangeRequested(MainStruct *mainStruct) {
        return mainStruct->currentAccount == NascEnvironment::NASC_ENV_Dev && mainStruct->buttonSelected == NascEnvironment::NASC_ENV_Dev
            && selectedAlt != currentAlt;
    }

    Result switchServer(MainStruct *mainStruct) {
        (void)mainStruct;
        return handleAzahar(ACCOUNT_ID);
    }

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

        const bool alt = friend_account_id == ACCOUNT_ID && selectedAlt && hasAlternate();
        for (const char *pattern : PATTERNS) {
            if (friend_account_id == ACCOUNT_ID) {
                res = registerRule(pattern, alt ? alternateHost() : HOST);
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

        httpcExit();
        if (res == ResultNoAzaharCommand) {
            // * LLE の HTTP では Azahar の命令がない。代わりに IPS を選んだ先に合わせて書き換える
            // * （次の再起動で読み込まれる）
            res = installIps(friend_account_id != ACCOUNT_ID ? IPS_PRETENDO : alt ? IPS_ALT : IPS_AOMIMI);
        }
        if (R_SUCCEEDED(res) && friend_account_id == ACCOUNT_ID) {
            writeCurrentAlt(alt);
        }
        return res;
    }

    void restoreIpsAfterUpdate(MainStruct *mainStruct) {
        if (!isAzahar() || (u8)mainStruct->currentAccount + 1 != ACCOUNT_ID) {
            return;
        }
        const Result res = installIps(currentAlt && hasAlternate() ? IPS_ALT : IPS_AOMIMI);
        if (R_FAILED(res)) {
            LOG_NIMBUS_ERROR(mainStruct, std::format("Failed to restore the AOMIMI patches: {:08X}", (u32)res).c_str());
        }
    }

    void showAlreadySelected(MainStruct *mainStruct) {
        // * イベントの相手を足せなかったときは、その文（先に入っている）を残す
        if ((u8)mainStruct->currentAccount + 1 == ACCOUNT_ID) {
            LOG_NIMBUS_ERROR(mainStruct, "You are already using this account.\nThe event battle bots are in your friend list.");
        } else {
            LOG_NIMBUS_ERROR(mainStruct, "You are already using this account.");
        }
    }

    void addEventPlayers(MainStruct *mainStruct) {
        addPlayers(mainStruct, true, true);
    }

    void addEventPlayersAfterSwitch(MainStruct *mainStruct) {
        // * unloadAccount で入った「通信をしない状態」を解いて、ネットにつながるのを少し待つ
        if (R_FAILED(NDMU_LeaveExclusiveState())) {
            return;
        }
        const bool ac = R_SUCCEEDED(acInit());
        if (ac) {
            for (int n = 0; n < 40; n++) {
                u32 status = 0;
                if (R_SUCCEEDED(ACU_GetWifiStatus(&status)) && status != 0) {
                    break;
                }
                svcSleepThread(250000000LL);
            }
        }
        addPlayers(mainStruct, true, false);
        if (ac) {
            acExit();
        }
    }

    void checkEventPlayersOnce(MainStruct *mainStruct) {
        static bool checked = false;
        if (checked) {
            return;
        }
        checked = true;
        if (mainStruct->needsReboot || mainStruct->errorString[0] != 0) {
            return;
        }
        if (mainStruct->currentAccount != NascEnvironment::NASC_ENV_Dev) {
            // * Pretendo や任天堂のアカウントに、前の版で足したイベントの相手が残っていれば消す。
            // * この PID は AOMIMI Network の中だけの番号で、ほかのサーバーでは別の人に割り振られるかもしれない
            removeEventPlayers();
            return;
        }
        // * 起動したときにオンラインなら、足りない相手だけ黙って足す（ログインはしない。待たせないため）
        addPlayers(mainStruct, false, false);
    }
}
