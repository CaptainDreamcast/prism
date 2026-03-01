Script to make prism game for Dreamcast

Usage (Command line): make_cdi.bat CD_NAME CdiName [AudioSectorCount]
Example: make_cdi.bat DOLMEXICA_INFINITE_DEMO_100 DolmexicaInfiniteDemo100

To mod the game, change the files in the parent folder's assets folder (you must avoid minuses in file names).
If you want your own IP.BIN data, edit ip/ip.txt
If you want your own boot logo at the license screen, edit ip/INSERT.png (just remember you have to use a paletted 320x90 image with no more than 128 colors for that one)
Run this program from either the command line or use the GUI (which just calls the script with the parameters you set).

The AudioSectorCount can be used to push the game data to the outer part of the CD-R, so the Dreamcast laser can read it faster (which reduces load times). To calculate it to bytes, assume 2352 bytes per sector (audio padding). Don't worry about this parameter if you don't know what it means, it's optional.

Uses cdi4dc by dreamcast legend SiZiOUS (http://sizious.com/), logotools by dreamcast legend ADK/Napalm (http://napalm-x.thegypsy.com/andrewk/dc/) and makeip by dreamcast legend Marcus C. (http://mc.pp.se/dc/sw.html)
Feel free to read up on how these programs work, making your own dreamcast disc is always the most fun

Known issues: 
- A large CD_NAME will lead to errors when creating the image, so try to use a small name there.
- This tool in particular struggles with paths that have special characters in them. For Windows 10, I was able to fix this by going to Language Settings -> Administrative Language Settings, then select "Change System Locale" under Language for non-Unicode programs and checking the checkbox that says "Beta: Use Unicode UTF-8 for worldwide language support".