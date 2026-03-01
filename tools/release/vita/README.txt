Script to make prism game for Vita

Usage (Command line): make_vita.bat GameName TitleID Version
Example: make_web.bat "Dolmexica Infinite" DOLM00001 01.00

Make sure the TitleID is 9 characters, you'll get Error 0xF0030000 while installing if it's not.
To mod the game, change the files in the parent folder's assets folder.
If you want to modify the appearance of the game in the Vita menu, modify the files in common/sce_sys/ before building.
The output will be a .vpk file you can upload to your Vita and install there.

Temporary files created during the packaging are created in the build folder. If something goes wrong during packaging, these files might be helpful with debugging.

Uses mksfoex and pack-vpk from the vita sdk toolchain: https://github.com/vitasdk/vita-toolchain
