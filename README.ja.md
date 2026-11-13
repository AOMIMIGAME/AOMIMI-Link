# AOMIMI Link
## Azahar 用のアカウントマネージャー

AOMIMI Link は、Pretendo Network の [Nimbus](https://github.com/PretendoNetwork/nimbus) をもとにした改造版です。AOMIMI Network・Pretendo・Nintendo のアカウントを切り替えられます。
Pretendo Network および任天堂とは関係ありません。AOMIMI Link について、Pretendo Network に問い合わせないでください。

[English](README.md)

## 使い方

1. FBI の「Remote Install」→「Scan QR Code」で下の URL を読み込む（または [Releases](https://github.com/AOMIMIGAME/AOMIMI-Link/releases) ページから `aomimi-link.cia` を入手し、FBI でインストールする）
    - `https://github.com/AOMIMIGAME/AOMIMI-Link/releases/latest/download/aomimi-link.cia`
2. Azahar で AOMIMI Link を起動し、AOMIMI・Pretendo・Nintendo のどのアカウントを使うかを選ぶ
    - AOMIMI を初めて選んだときは、AOMIMI Network 用のアカウントが新しく作られます
    - アカウントを切り替えると、本体が再起動します

- イベント対戦管理 bot は、妖怪ウォッチ2 のフレンドの一覧に最初から表示されます。フレンドに追加する必要はありません
- AOMIMI Link は Azahar 専用です。実機には対応していません
- Azahar で「オンラインに必要な LLE モジュール」が有効な場合は、アカウントの切り替え時に `luma/sysmodules` の IPS が自動で書き換えられます。既存の IPS は、初回のみ `3ds/aomimi/ips-backup` に保存されます
- 更新するときは、同じ URL を FBI でもう一度読み込んでください。常に最新版がインストールされます

## ビルド

1. `git clone https://github.com/AOMIMIGAME/AOMIMI-Link` で、リポジトリをクローンする
2. Docker で devkitARM のイメージを使い、ビルドスクリプトを実行する
    - `docker run --rm -v "$PWD:/src" devkitpro/devkitarm:latest bash /src/build-in-docker.sh`
3. `app/nimbus.cia` ができる

- ボタンやアイコンの画像を作り直すときは、`app/aomimi` の中で `python make_buttons.py` と `python make_meta.py` を実行してください（Pillow が必要です）
- 公式の Nimbus の更新は、GitHub Actions が毎日確認して取り込みます

## ライセンス

AOMIMI Link は、Nimbus と同じ GNU General Public License v3.0 で配布しています（[LICENSE](LICENSE)）。
Nimbus からの変更点は [AOMIMI.md](AOMIMI.md) にまとめています。

## クレジット

感謝を込めて:

- [Pretendo Network](https://github.com/PretendoNetwork): AOMIMI Link のもとになった Nimbus
- [pinklimes](https://github.com/gitlimes): Nimbus の CIA 版のバナー
- [TraceEntertains](https://github.com/TraceEntertains): Nimbus の CIA 版の作成と、プロジェクトのメンテナンス
- [DaniElectra](https://github.com/DaniElectra): 3DS の HTTP と Socket のパッチの作成と、プロジェクトのメンテナンス
- [SciresM](https://github.com/SciresM): 3DS の SSL パッチの作成
- [zaksabeast](https://github.com/zaksabeast): 元になった 3ds-Friend-Account-Manager と、friends と act のシステムタイトルについての研究のすべて
- [shutterbug2000](https://github.com/shutterbug2000): GUI
- [libctru](https://github.com/devkitPro/libctru): `frda.c` のベース、homebrew のテンプレート、その他のライブラリ関数（システムフォントの関数の一部については citro2d にも感謝）
- [Universal-Core](https://github.com/Universal-Team/Universal-Core): 文字列の描画関数
- [Fangal-Airbag](https://github.com/Fangal-Airbag): アカウント切り替えの GUI をボタン操作に対応させたこと
- そのほかすべての 3DS の研究者たち
