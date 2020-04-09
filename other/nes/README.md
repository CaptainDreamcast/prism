# Prism for NES
Collection of useful routines for use in NES assembly development.  
For use, include `wrapper.asm` at the beginning of your project.
Expects a file called `variables.asm` where the game variables are declared.  
Does basic setup, declares common variables and jumps to a `__init` routine, which is where your game initialization should go. 
Expects an `__update` routine that is called every frame.
Expects a folder structure where prism is located in ../addons/prism, similar to a KallistiOS setup.  

### Tools used for Mega Drive development:
[__FCEUX__](http://www.fceux.com/web/home.html)  
[__NESASM3__](http://www.nespowerpak.com/nesasm/NESASM3.zip)  
[__YY-CHR__](https://www.romhacking.net/utilities/119/)  
[__6502 guide__](http://nesdev.com/6502guid.txt)

Roughly based on [this NES dev guide](http://www.vbforums.com/showthread.php?858389-NES-6502-Programming-Tutorial-Part-1-Getting-Started), stray comments are probably from there.