# Prism for Mega Drive
Collection of useful routines for use in Mega Drive assembly development.  
For use, include `wrapper.asm` at the beginning of your project.
Does basic setup and jumps to a `__main` label, which is where your game should start.  
Expects a file called `header.asm` where the Mega Drive file header is declared.  
Expects a folder structure where prism is located in ../addons/prism, similar to a KallistiOS setup.  
Expects a label called `__end` at the end of your game.  
Defines a label called `gLibrarySentinel`, after which variables can be placed in RAM.  

### Tools used for Mega Drive development:
[__BMP2Tile__](http://gendev.spritesmind.net/page-b2t)  
[__Megadrive Map Maker__](http://gendev.spritesmind.net/page-mmm.html)  
[__Hex Editor HxD__](https://mh-nexus.de/de/hxd/)  
[__ASM68k Assembler__](http://retrocdn.net/File:ASM68k.7z)  
  
Made possible in part due to [BigEvilCorp's cool Mega Drive dev blog](https://bigevilcorporation.co.uk/), so check that out as well if you're interested in Mega Drive development.
