Script to make prism game for HTML5

Usage (Command line): make_web.bat GameName
Example: make_web.bat DolmexicaInfiniteDemo100

To mod the game, change the files in the parent folder's assets folder.
If you want to modify the index.html used to display the game, for example to display your own logo, modify common/index.html.
The GUI/batch file will output a zipped folder you can directly upload to itch.io, newgrounds, or similar sites.

Requires PowerShell 3 for zipping so that step might not work with older Windows versions. The contents of the zip file are also in the build folder, so if automatic zipping fails, zip that one manually.

Uses extremely slimmed down versions of portable python (https://sourceforge.net/projects/winpython/) and emscripten (https://emscripten.org/)

Known issues: 
- Opening the index.html in the build folder in your local browser will not work. To test the game, upload it to one of the sites above, or use emrun from emscripten.