
bsize				equ 0x1		; byte
wsize 				equ 0x2 	; word
lsize 				equ 0x4		; long word
tsize 				equ 0x20 	; tile
psize 				equ 0x40  	; palette


LibraryData:
vblank_counter		equ 0x00FF0000 ; l
hblank_counter		equ (vblank_counter+lsize) ; l
i0					equ (hblank_counter+lsize) ; l
i1					equ (i0+lsize) ; l
i2					equ (i1+lsize) ; l
i3					equ (i2+lsize) ; l
i4					equ (i3+lsize) ; l
i5					equ (i4+lsize) ; l
i6					equ (i5+lsize) ; l
i7					equ (i6+lsize) ; l
gGamePadFlankState	equ (i7+lsize) ; w
gGamePadState		equ (gGamePadFlankState+wsize) ; w
ScreenReturnValue	equ (gGamePadState+wsize) ; w
gCurrentLevelID 	equ (ScreenReturnValue+wsize) ; w
gScreenPointer 		equ (gCurrentLevelID+wsize) ; l
gActivePaletteAmount equ (gScreenPointer+lsize) ; w
gPaletteBuffer		equ (gActivePaletteAmount+wsize) ; 4*16 words as palette buffer
gLibrarySentinel     equ (gPaletteBuffer+4*0x10*wsize)
