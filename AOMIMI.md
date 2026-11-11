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
- When the AOMIMI account is selected again, the AOMIMI event players are added to the friend list.
- Renamed the application to AOMIMI Link (title id `0xD40D3`, so it can be installed next to the original Nimbus).
- Only the app is built and distributed. The sysmodule patches and the plugin for real hardware are not changed and are not part of AOMIMI Link.
- The scam warning on the top screen is split into three lines, because the longer name did not fit in two.
- The "Launch plugin" control (Y) is disabled, because the plugin is not distributed. "Unlink PNID" (X) is only shown and used while the Pretendo Network button is selected.
