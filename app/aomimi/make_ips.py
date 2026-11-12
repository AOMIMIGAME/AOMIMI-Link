"""LLE の Azahar 用の IPS を作る（AOMIMI Link に埋め込む）。

公式 Nimbus のリリース（combined.<版>.zip の 3ds/nimbus/update/）にあるシステムモジュールの IPS を元に、2 組を作る:
  romfs/aomimi_ips/pretendo/  公式のまま（Pretendo と Nintendo を選んだとき）
  romfs/aomimi_ips/aomimi/    接続先を AOMIMI Network に変えたもの（AOMIMI を選んだとき）

公式の IPS は本体のシステムのファイルを元に作られるので、ここでは作り直さず、
公式の IPS の中の接続先の文字だけを、長さを変えずに書き換える。

使い方: python3 make_ips.py <combined.zip> <romfs のフォルダ>
"""
import pathlib
import re
import shutil
import sys
import zipfile

# * AOMIMI Network の接続先（app/source/aomimi/server.hpp の HOST と同じ）
HOST = b'aomimi.f5.si'

FRIENDS = '0004013000003202.ips'
ACT = '0004013000003802.ips'

# * friends: NASC の URL（patches/friends/src/nasc_url.s の .org 0x16129a、.area 38）。
#   公式は URL の後半だけを書き換えているので、URL 全体を 1 つの記録で書く
FRIENDS_OFFSET = 0x16129A - 0x100000
FRIENDS_AREA = 38
NASC_URL = b'https://' + HOST + b'/ac/'

# * act: アカウントサーバーの URL（patches/act/src/account_url.s の unofficial_url）
ACT_OLD = b'https://account.pretendo.cc/v1/api/'
ACT_NEW = b'https://' + HOST + b'/v1/api/'

# * http と socket: nintendowifi.net などを置き換える先。Pretendo へ行かないように、
#   解決できない名前にする（同じ 11 文字）。NASC とアカウントは上の URL で直接 AOMIMI へ行く
OTHER_OLD = b'pretendo.cc'
OTHER_NEW = b'aomimi.test'


def ips(records):
    out = bytearray(b'PATCH')
    for offset, data in records:
        assert 0 < len(data) <= 0xFFFF and offset < 0x1000000 and offset != 0x454F46
        out += offset.to_bytes(3, 'big') + len(data).to_bytes(2, 'big') + data
    return bytes(out + b'EOF')


def replace_same_length(data, old, new):
    assert len(new) <= len(old)
    count = data.count(old)
    return data.replace(old, new + b'\0' * (len(old) - len(new))), count


def main():
    zip_path, romfs = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
    official = {}
    with zipfile.ZipFile(zip_path) as z:
        for name in z.namelist():
            # * combined.zip では 3ds/nimbus/update/ に入っている。システムモジュール（00040130）の IPS だけ使う
            #   （00040030 は Miiverse のアプリの IPS で、AOMIMI では使わない）
            base = name.rsplit('/', 1)[-1]
            if re.fullmatch(r'00040130[0-9A-F]{8}\.ips', base):
                official[base] = z.read(name)
    if FRIENDS not in official or ACT not in official:
        sys.exit(f'公式の IPS が見つかりません: {sorted(official)}')

    out = romfs / 'aomimi_ips'
    if out.exists():
        shutil.rmtree(out)
    (out / 'pretendo').mkdir(parents=True)
    (out / 'aomimi').mkdir(parents=True)

    assert len(NASC_URL) + 1 <= FRIENDS_AREA
    for name, data in sorted(official.items()):
        (out / 'pretendo' / name).write_bytes(data)
        if name == FRIENDS:
            # * 領域の残りも 0 で埋める（元の長い URL の後ろが残らないように）
            new = ips([(FRIENDS_OFFSET, NASC_URL + b'\0' * (FRIENDS_AREA - len(NASC_URL)))])
        elif name == ACT:
            new, count = replace_same_length(data, ACT_OLD, ACT_NEW)
            if count != 1:
                sys.exit(f'{name}: アカウントの URL が {count} 個（1 個のはず）')
        else:
            new, _ = replace_same_length(data, OTHER_OLD, OTHER_NEW)
        if b'pretendo' in new:
            sys.exit(f'{name}: pretendo の名前が残っています')
        (out / 'aomimi' / name).write_bytes(new)
        print(f'{name}: {len(data)} -> {len(new)} bytes')


if __name__ == '__main__':
    main()
