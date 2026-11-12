#!/bin/bash
# Builds only the AOMIMI Link app CIA inside devkitpro/devkitarm. The sysmodule patches are not rebuilt:
# the IPS for LLE Azahar are made from the official Nimbus release (app/aomimi/make_ips.py) and embedded in the app.
# SRC is the repository root (default /src for the local docker run; GitHub Actions passes its workspace).
set -eu
SRC="${SRC:-/src}"
export DEVKITPRO=/opt/devkitpro DEVKITARM=/opt/devkitpro/devkitARM
export PATH=$DEVKITPRO/tools/bin:$DEVKITARM/bin:$PATH
apt-get -qq update >/dev/null && apt-get -qq install -y curl unzip g++ make >/dev/null 2>&1 || true
dkp-pacman -Sy --noconfirm --needed libctru citro2d citro3d 3ds-curl 3ds-mbedtls 3ds-zlib 3ds-jansson 3dstools tex3ds 2>&1 | tail -2
mkdir -p /tools
if ! command -v makerom >/dev/null; then
    curl -fsSL -o /tmp/mr.zip https://github.com/3DSGuy/Project_CTR/releases/download/makerom-v0.18.4/makerom-v0.18.4-ubuntu_x86_64.zip
    unzip -oq /tmp/mr.zip -d /tools
fi
if ! command -v bannertool >/dev/null; then
    curl -fsSL -o /tmp/bt.zip https://github.com/diasurgical/bannertool/releases/download/1.2.0/bannertool.zip
    unzip -oq /tmp/bt.zip -d /tmp/bt
    cp /tmp/bt/linux-x86_64/bannertool /tools/
fi
chmod +x /tools/*
export PATH=/tools:$PATH
makerom -v 2>&1 | head -1 || true
cd "$SRC/app"
# LLE の Azahar 用の IPS: 公式 Nimbus の同じ版のリリースから作り、romfs に入れる（app/aomimi/make_ips.py）
version=$(sed -n 's/^VERSION_MAJOR := //p; s/^VERSION_MINOR := //p; s/^VERSION_MICRO := //p' Makefile | paste -sd.)
if [ ! -f "/tmp/combined.$version.zip" ]; then
    curl -fsSL -o "/tmp/combined.$version.zip" "https://github.com/PretendoNetwork/nimbus/releases/download/v$version/combined.$version.zip"
fi
python3 aomimi/make_ips.py "/tmp/combined.$version.zip" romfs
make clean >/dev/null 2>&1 || true
make cia -j4 2>&1 | tail -8
ls -la "$SRC"/app/*.cia
