# AOMIMI Link
## Account manager for Azahar

AOMIMI Link is a modified version of [Nimbus](https://github.com/PretendoNetwork/nimbus) by Pretendo Network. It switches between AOMIMI Network, Pretendo and Nintendo accounts.
It is not affiliated with Pretendo Network or Nintendo. Please do not contact Pretendo Network about AOMIMI Link.

[日本語](README.ja.md)

## Usage

1. Scan the URL below with "Remote Install" → "Scan QR Code" in FBI (or grab `aomimi-link.cia` from the [Releases](https://github.com/AOMIMIGAME/AOMIMI-Link/releases) page and install it using FBI)
    - `https://github.com/AOMIMIGAME/AOMIMI-Link/releases/latest/download/aomimi-link.cia`
2. Run AOMIMI Link in Azahar and select whether to use an AOMIMI, Pretendo or Nintendo account
    - The first time you select AOMIMI, a new account for AOMIMI Network is created
    - Selecting AOMIMI also adds the event battle bots to your friend list (if this fails, they will be added the next time AOMIMI Link is opened)
    - Switching accounts reboots the system

- When AOMIMI Link is opened on a Pretendo or Nintendo account, any remaining event battle bots are removed from that friend list (other friends are not affected)
- AOMIMI Link is for Azahar only. Real hardware is not supported
- If "required online LLE modules" is enabled in Azahar, the IPS patches in `luma/sysmodules` are replaced automatically when switching accounts. Existing IPS patches are saved to `3ds/aomimi/ips-backup` the first time
- To update, scan the same URL with FBI again. The latest version is always installed

## Building

1. Clone the repository using `git clone https://github.com/AOMIMIGAME/AOMIMI-Link`
2. Run the build script in the devkitARM image with Docker
    - `docker run --rm -v "$PWD:/src" devkitpro/devkitarm:latest bash /src/build-in-docker.sh`
3. You will get `app/nimbus.cia`

- To regenerate the button and icon images, run `python make_buttons.py` and `python make_meta.py` in `app/aomimi` (requires Pillow)
- Updates to the original Nimbus are checked and merged daily by GitHub Actions

## License

AOMIMI Link is distributed under the GNU General Public License v3.0, the same as Nimbus (see [LICENSE](LICENSE)).
The changes from Nimbus are listed in [AOMIMI.md](AOMIMI.md).

## Credits

Thanks to:

- [Pretendo Network](https://github.com/PretendoNetwork) for Nimbus, which AOMIMI Link is based on
- [pinklimes](https://github.com/gitlimes) for the Nimbus CIA version banner
- [TraceEntertains](https://github.com/TraceEntertains) for making a CIA version of Nimbus and maintaining the project
- [DaniElectra](https://github.com/DaniElectra) for making the 3DS HTTP and Socket patches and maintaining the project
- [SciresM](https://github.com/SciresM) for making the 3DS SSL patches
- [zaksabeast](https://github.com/zaksabeast) for the original 3ds-Friend-Account-Manager and all the research into the friends and act system titles
- [shutterbug2000](https://github.com/shutterbug2000) for the GUI
- [libctru](https://github.com/devkitPro/libctru) for the `frda.c` base, homebrew template, and other library functions (and thanks to citro2d for part of a system font function)
- [Universal-Core](https://github.com/Universal-Team/Universal-Core) for the string drawing functions
- [Fangal-Airbag](https://github.com/Fangal-Airbag) for making the account switcher GUI support button controls
- All other 3DS researchers
