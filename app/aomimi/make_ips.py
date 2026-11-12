"""LLE の Azahar 用の IPS を作る（AOMIMI Link に埋め込む）。

公式 Nimbus のリリース（combined.<版>.zip の 3ds/nimbus/update/）にあるシステムモジュールの IPS を元に作る:
  romfs/aomimi_ips/pretendo/  公式のまま（Pretendo と Nintendo を選んだとき）
  romfs/aomimi_ips/aomimi/    接続先を AOMIMI Network に変えたもの（AOMIMI を選んだとき）
  romfs/aomimi_ips/alt/       接続先を別の住所に変えたもの（その住所を渡したときだけ）

公式の IPS は本体のシステムのファイルを元に作られるので、ここでは作り直さず、
公式の IPS の中の接続先の文字だけを、長さを変えずに書き換える。

使い方: python3 make_ips.py <combined.zip> <romfs のフォルダ> [別の住所]
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

# * act: アカウントサーバーの URL（patches/act/src/account_url.s の unofficial_url）
ACT_OLD = b'https://account.pretendo.cc/v1/api/'

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


def redirect(official, host):
    """公式の IPS の接続先を host に変えた組を作る"""
    nasc_url = b'https://' + host + b'/ac/'
    act_new = b'https://' + host + b'/v1/api/'
    if len(nasc_url) + 1 > FRIENDS_AREA or len(act_new) > len(ACT_OLD):
        sys.exit(f'{host.decode()}: 住所が長すぎて IPS に入りません')
    out = {}
    for name, data in sorted(official.items()):
        if name == FRIENDS:
            # * 領域の残りも 0 で埋める（元の長い URL の後ろが残らないように）
            new = ips([(FRIENDS_OFFSET, nasc_url + b'\0' * (FRIENDS_AREA - len(nasc_url)))])
        elif name == ACT:
            new, count = replace_same_length(data, ACT_OLD, act_new)
            if count != 1:
                sys.exit(f'{name}: アカウントの URL が {count} 個（1 個のはず）')
        else:
            new, _ = replace_same_length(data, OTHER_OLD, OTHER_NEW)
        if b'pretendo' in new:
            sys.exit(f'{name}: pretendo の名前が残っています')
        out[name] = new
    return out


def main():
    zip_path, romfs = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
    alt_host = sys.argv[3].encode('ascii') if len(sys.argv) > 3 and sys.argv[3] else None
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

    sets = {'pretendo': official, 'aomimi': redirect(official, HOST)}
    if alt_host:
        sets['alt'] = redirect(official, alt_host)

    out = romfs / 'aomimi_ips'
    if out.exists():
        shutil.rmtree(out)
    for kind, files in sets.items():
        (out / kind).mkdir(parents=True)
        for name, data in files.items():
            (out / kind / name).write_bytes(data)
        print(f'{kind}: ' + ', '.join(f'{name} {len(data)}' for name, data in sorted(files.items())))


if __name__ == '__main__':
    main()
