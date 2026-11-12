# AOMIMI Link

AOMIMI Link is a modified version of [Nimbus](https://github.com/PretendoNetwork/nimbus) by Pretendo Network.
It switches the 3DS between three online services: AOMIMI Network, Pretendo Network and Nintendo.

AOMIMI Link is not affiliated with, endorsed by, or supported by Pretendo Network or Nintendo.
Please do not ask Pretendo Network for support with AOMIMI Link.

## License

Nimbus is licensed under the GNU General Public License v3.0 (see `LICENSE`).
AOMIMI Link is distributed under the same license, and its full source code is available in this repository.

## Modifications

Modified by the AOMIMI project, starting 2026-11-11. Changes from the original Nimbus:

- Added a third account (friend account 3, NASC environment Dev) for AOMIMI Network, next to Nintendo (account 1) and Pretendo Network (account 2). The Pretendo Network account works the same as in the original Nimbus.
- On Azahar, the URL replacement rules for AOMIMI Network are registered when the AOMIMI account is selected and removed otherwise (`app/source/aomimi/`).
- The account screen shows three buttons. New button images, home menu icon, banner and top screen image (`app/aomimi/make_buttons.py`, `app/aomimi/make_meta.py`). The Pretendo Network logo is only used on the button that selects the Pretendo Network account.
- The AOMIMI event players are added to the friend list right after switching to the AOMIMI account (before the reboot). If that fails, they are added the next time AOMIMI Link is opened on the AOMIMI account, or when the AOMIMI account is selected again.
- Renamed the application to AOMIMI Link (title id `0xD40D3`, so it can be installed next to the original Nimbus). The Home Menu shows AOMIMIGAME as the publisher; the original authors are credited in the README and on the top screen.
- Only the app is built and distributed. The sysmodule patch sources and the plugin for real hardware are not changed, and the plugin is not part of AOMIMI Link.
- For Azahar with LLE online modules (where the HLE URL replacement is not available), AOMIMI Link embeds two sets of sysmodule IPS patches and writes the matching set to `luma/sysmodules` when switching accounts: the official Nimbus patches for Pretendo Network and Nintendo, and a copy with the server addresses changed for AOMIMI Network. The sets are made at build time from the official Nimbus release of the same version (`app/aomimi/make_ips.py`): the NASC and account URLs are replaced with the AOMIMI Network address, and the other Pretendo Network host names are replaced with an unresolvable name. Existing IPS patches that match neither set are saved to `3ds/aomimi/ips-backup` once. After an official Nimbus update (`3ds/nimbus/update`), the AOMIMI set is restored if the AOMIMI account is selected.
- Selecting the account that is already in use no longer exits the app (exiting failed when the app was started directly from Azahar). It shows a message instead.
- The AOMIMI Network address is the domain `aomimi.f5.si`.
- The scam warning on the top screen is split into three lines, because the longer name did not fit in two.
- The "Launch plugin" control (Y) is disabled, because the plugin is not distributed. "Unlink PNID" (X) is only shown and used while the Pretendo Network button is selected.
- When AOMIMI Link is opened on the Nintendo or Pretendo Network account, the AOMIMI event players (PIDs 1999999001 and 1999999002, which only exist on AOMIMI Network) are removed from that friend list if an earlier version left them there. Other friends are not touched.
- The AOMIMI account remembers which server it points to in `3ds/aomimi/server.txt`. If it points to a server other than AOMIMI Network, selecting AOMIMI switches it back.
- The screen parts (buttons, top screen notes, controls) are in `app/source/aomimi/ui/`. The title id and product code are passed to `meta/nimbus-cia.rsf` from the Makefile, and local build settings can be placed in `app/extra.mk` (not part of this repository).

---

# AOMIMI Link（日本語）

AOMIMI Link は、Pretendo Network の [Nimbus](https://github.com/PretendoNetwork/nimbus) をもとにした改造版です。
3DS の接続先を、AOMIMI Network・Pretendo Network・Nintendo の 3 つのオンラインサービスの間で切り替えます。

AOMIMI Link は、Pretendo Network および任天堂とは関係がなく、承認や支援も受けていません。
AOMIMI Link について、Pretendo Network に問い合わせないでください。

## ライセンス

Nimbus は GNU General Public License v3.0 で公開されています（`LICENSE` を参照）。
AOMIMI Link も同じライセンスで配布しており、ソースコードはすべてこのリポジトリで公開しています。

## 変更点

2026-11-11 から AOMIMI プロジェクトが改造しています。元の Nimbus からの変更点は次のとおりです。

- Nintendo（アカウント 1）と Pretendo Network（アカウント 2）に加えて、AOMIMI Network 用の 3 つ目のアカウント（フレンドのアカウント 3、NASC の環境 Dev）を追加しました。Pretendo Network のアカウントは、元の Nimbus と同じように動作します。
- Azahar では、AOMIMI のアカウントを選んだときに AOMIMI Network 用の URL の置き換えの規則を登録し、それ以外を選んだときに削除します（`app/source/aomimi/`）。
- アカウントの画面に 3 つのボタンを表示します。ボタンの画像、ホームメニューのアイコン、バナー、上画面の画像を新しくしました（`app/aomimi/make_buttons.py`、`app/aomimi/make_meta.py`）。Pretendo Network のロゴは、Pretendo Network のアカウントを選ぶボタンにだけ使っています。
- AOMIMI のアカウントに切り替えた直後（再起動の前）に、AOMIMI のイベント対戦管理 bot をフレンドに追加します。追加できなかった場合は、次に AOMIMI のアカウントで AOMIMI Link を開いたとき、または AOMIMI のアカウントをもう一度選んだときに追加します。
- アプリの名前を AOMIMI Link に変更しました（タイトル ID `0xD40D3`。元の Nimbus と並べてインストールできます）。ホームメニューの発行元は AOMIMIGAME と表示されます。元の作者は README と上画面に記載しています。
- ビルドして配布するのはアプリだけです。システムモジュールのパッチのソースと実機用のプラグインは変更しておらず、プラグインは AOMIMI Link に含まれていません。
- 「オンラインに必要な LLE モジュール」が有効な Azahar（HLE の URL の置き換えが使えない場合）向けに、システムモジュールの IPS を 2 組埋め込み、アカウントの切り替え時に対応する組を `luma/sysmodules` に書き込みます。Pretendo Network と Nintendo には公式 Nimbus の IPS、AOMIMI Network には接続先を変更した IPS を使います。この 2 組は、ビルド時に同じ版の公式 Nimbus のリリースから作成します（`app/aomimi/make_ips.py`）。NASC とアカウントの URL を AOMIMI Network の接続先に置き換え、そのほかの Pretendo Network のホスト名は解決できない名前に置き換えています。どちらの組とも一致しない既存の IPS は、初回のみ `3ds/aomimi/ips-backup` に保存します。公式 Nimbus の更新（`3ds/nimbus/update`）のあとは、AOMIMI のアカウントが選ばれていれば AOMIMI 用の IPS に戻します。
- 使用中のアカウントを選んだときに、アプリを終了しないようにしました（Azahar から直接起動した場合に終了できず、落ちていたため）。代わりにメッセージを表示します。
- AOMIMI Network の接続先は、ドメイン `aomimi.f5.si` です。
- 名前が長くなり 2 行に収まらないため、上画面の詐欺に関する注意書きを 3 行に分けました。
- プラグインを配布しないため、「プラグインを起動」（Y）を無効にしました。「PNID のリンクを解除」（X）は、Pretendo Network のボタンを選んでいるときだけ表示され、使えます。
- Nintendo または Pretendo Network のアカウントで AOMIMI Link を開いたとき、以前の版が追加した AOMIMI のイベント対戦管理 bot（PID 1999999001 と 1999999002。AOMIMI Network にだけ存在します）がフレンド一覧に残っていれば削除します。ほかのフレンドは変更されません。
- AOMIMI のアカウントがどの接続先に向いているかを `3ds/aomimi/server.txt` に保存します。AOMIMI Network 以外を指している場合、AOMIMI を選ぶと AOMIMI Network に戻します。
- 画面の部品（ボタン、上画面の注意書き、操作の説明）は `app/source/aomimi/ui/` にあります。タイトル ID とプロダクトコードは Makefile から `meta/nimbus-cia.rsf` に渡しており、手元だけのビルドの設定は `app/extra.mk` に置けます（このリポジトリには含みません）。
