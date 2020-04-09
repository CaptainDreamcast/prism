; Taken in part from https://bigevilcorporation.co.uk
; So buy Tanglewood, will ya

vdp_control				equ 0x00C00004
vdp_data				equ 0x00C00000

vdp_write_palettes		equ 0xF0000000
vdp_write_tiles			equ 0x40000000
vdp_write_plane_a		equ 0x40000003
vdp_write_plane_b		equ 0x60000002
vdp_write_sprite_table	equ 0x60000003
vdp_write_hscroll       equ 0x50000003
vdp_write_vscroll       equ 0x40000010

vdp_read_sprite_table   equ 0x20000003
; TODO: rewrite this to use address calculation function
; From Gendev:
; #define GFX_READ_VRAM_ADDR(adr)     ((0x0000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00)
; #define GFX_READ_CRAM_ADDR(adr)     ((0x0000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x20)
; #define GFX_READ_VSRAM_ADDR(adr)    ((0x0000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x10)

; #define GFX_WRITE_VRAM_ADDR(adr)    ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00)
; #define GFX_WRITE_CRAM_ADDR(adr)    ((0xC000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x00)
; #define GFX_WRITE_VSRAM_ADDR(adr)   ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x10)

; #define GFX_DMA_VRAM_ADDR(adr)      ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x80)
; #define GFX_DMA_CRAM_ADDR(adr)      ((0xC000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x80)
; #define GFX_DMA_VSRAM_ADDR(adr)     ((0x4000 + ((adr) & 0x3FFF)) << 16) + (((adr) >> 14) | 0x90)

SetAutoIncrement2:
	move.w #0x8F02, vdp_control
	rts

SetHScroll:
	; d0 - Value
	
	move.l  #vdp_write_hscroll, vdp_control 
	move.w  d0, vdp_data ; Plane A
	move.w  d0, vdp_data ; Plane B
	rts
	
SetVScroll:
	; d0 - Value
	
	move.l  #vdp_write_vscroll, vdp_control 
	move.w  d0, vdp_data  ; Plane A
	move.w  d0, vdp_data  ; Plane B
	rts
	
ClearPalettes:

	clr.l d0
	move.w #0x4,d0 
	move.l #vdp_write_palettes, vdp_control 
	mulu.w #0x8, d0

	jmp @ColourLoopEnd
	@ColourLoop:
	move.l #0x0, vdp_data 
	@ColourLoopEnd:
	dbra d0, @ColourLoop

	rts

IncreaseSinglePaletteColor:
	; d0 - Mask
	; d1 - Increase value
	; d3 - Temp Palette Value
	; d4 - Palette Value
	
	move.w d3,d5
	move.w d4,d6
	and.w d0,d5
	and.w d0,d6
	cmp d5,d6
	beq @NoIncrease
	add.w d1,d3
	@NoIncrease:
	
	rts
	
IncreasePaletteIfNecessary:
	; a0 - Palette Adress
	; a1 - Temp Palette Address
	; d0 - Palette Amount
	
	clr.l d5
	clr.l d3
	clr.l d4
	
	move.w d0,d5
	mulu.w #0x10,d5
	
	jmp @IncreaseLoopEnd
	@IncreaseLoop:
	
	move.w d0, -(sp)
	move.w d1, -(sp)
	move.w d5, -(sp)
	move.w d6, -(sp)
	
	move.w (a0)+,d4
	move.w (a1),d3

	move.w #0xF00,d0
	move.w #0x100,d1
	jsr IncreaseSinglePaletteColor
	
	move.w #0x0F0,d0
	move.w #0x010,d1
	jsr IncreaseSinglePaletteColor
	
	move.w #0x00F,d0
	move.w #0x001,d1
	jsr IncreaseSinglePaletteColor
	
	move.w d3,(a1)+
	
	move.w (sp)+,d6
	move.w (sp)+,d5
	move.w (sp)+,d1
	move.w (sp)+,d0
	
	@IncreaseLoopEnd:
	dbra d5,@IncreaseLoop
	
	rts
	
FadeInPalettes:
	; a0 (l) - Adresse of palette
	; d0 (w) - Palette amount

	move.w d0,-(sp)
	
	clr.l d0
	move.l a0,a5
	move.l #gPaletteBuffer,a6
	move.l a6,a0
	move.w #(0x10*4), d0
	move.w #0x0,d1
	jsr ClearW 
	
	move.w (sp)+,d0
	
	move.w #0xF,d6
	jmp @FadeInLoopEnd
	@FadeInLoop:
	
	move.l a5,a0
	move.l a6,a1
	jsr IncreasePaletteIfNecessary
	
	move.w d0,-(sp)
	move.l a6,a0
	jsr LoadPalettes
	move.w (sp)+,d0
	
	move.w d0,-(sp)
	jsr WaitForScreenStart   ; Wait for start of vblank
	
	jsr  WaitForScreenEnd ; Wait for end of vblank
	move.w (sp)+,d0
	
	@FadeInLoopEnd:
	dbra d6,@FadeInLoop
	
	@End:
	rts

CopyW:
	; a0 - dest address (l)
	; a1 - src address (l)
	; d0 - size in words (w)
	
	clr.l d3
	clr.l d6
	move.w d0,d6
	
	jmp @CopyLoopEnd
	@CopyLoop:
	move.w (a1)+,d3
	move.w d3,(a0)+
	@CopyLoopEnd:
	dbra d6,@CopyLoop
	
	rts

DecreaseSinglePaletteColor:
	; d0 - Mask
	; d1 - Increase value
	; d3 - Temp Palette Value
	
	move.w d3,d5
	and.w d0,d5
	cmp #0x0,d5
	beq @NoDecrease
	sub.w d1,d3
	@NoDecrease:
	
	rts
	
DecreasePaletteIfNecessary:
	; a1 - Temp Palette Address
	; d0 - Palette Amount
	
	clr.l d5
	clr.l d3
	clr.l d4
	
	move.w d0,d5
	mulu.w #0x10,d5
	
	jmp @DecreaseLoopEnd
	@DecreaseLoop:
	
	move.w d0, -(sp)
	move.w d1, -(sp)
	move.w d5, -(sp)
	move.w d6, -(sp)
	
	move.w (a1),d3

	move.w #0xF00,d0
	move.w #0x100,d1
	jsr DecreaseSinglePaletteColor
	
	move.w #0x0F0,d0
	move.w #0x010,d1
	jsr DecreaseSinglePaletteColor
	
	move.w #0x00F,d0
	move.w #0x001,d1
	jsr DecreaseSinglePaletteColor
	
	move.w d3,(a1)+
	
	move.w (sp)+,d6
	move.w (sp)+,d5
	move.w (sp)+,d1
	move.w (sp)+,d0
	
	@DecreaseLoopEnd:
	dbra d5,@DecreaseLoop
	
	rts
	
FadeOutPalettes:
	; a0 (l) - Adresse of palette
	; d0 (w) - Palette amount

	move.w d0,-(sp)
	
	clr.l d0
	move.l a0,a5
	move.l #gPaletteBuffer,a6
	
	move.l a6,a0
	move.l a5,a1
	move.w #(0x10*4), d0
	jsr CopyW 
	
	move.w (sp)+,d0
	
	move.w #0xF,d6
	jmp @FadeInLoopEnd
	@FadeInLoop:
	
	move.l a5,a0
	move.l a6,a1
	jsr DecreasePaletteIfNecessary
	
	move.w d0,-(sp)
	move.l a6,a0
	jsr LoadPalettes
	move.w (sp)+,d0
	
	move.w d0,-(sp)
	jsr WaitForScreenStart   ; Wait for start of vblank
	
	jsr  WaitForScreenEnd ; Wait for end of vblank
	move.w (sp)+,d0
	
	@FadeInLoopEnd:
	dbra d6,@FadeInLoop
	
	@End:
	rts
	
LoadPalettes:
; a0 (l) - Adresse of palette
; d0 (w) - Palette amount


	move.l #vdp_write_palettes, vdp_control ; Set up VDP to write to CRAM address 0x0000
	mulu.w #0x8, d0

	jmp @ColourLoopEnd
	@ColourLoop:
	move.l (a0)+, vdp_data ; Move data to VDP data port, and increment source address
	@ColourLoopEnd:
	dbra d0, @ColourLoop

	rts

DrawTextPlaneA:
; a0 (l) - String address
; d0 (w) - First tile ID of font
; d1 (bb)- XY coord (in tiles)
; d2 (b) - Palette

	clr.l    d3                     ; Clear d3 ready to work with
	move.b   d1, d3                 ; Move Y coord (lower byte of d1) to d3
	mulu.w   #0x0040, d3            ; Multiply Y by line width (H40 mode - 64 lines horizontally) to get Y offset
	ror.l    #0x8, d1               ; Shift X coord from upper to lower byte of d1
	add.b    d1, d3                 ; Add X coord to offset
	mulu.w   #0x2, d3               ; Convert to words
	swap     d3                     ; Shift address offset to upper word
	add.l    #vdp_write_plane_a, d3 ; Add PlaneA write cmd + address
	move.l   d3, vdp_control        ; Send to VDP control port
	
	clr.l    d3                     ; Clear d3 ready to work with again
	move.b   d2, d3                 ; Move palette ID (lower byte of d2) to d3
	rol.l    #0x8, d3               ; Shift palette ID to bits 14 and 15 of d3
	rol.l    #0x5, d3               ; Can only rol bits up to 8 places in one instruction
	
	lea      ASCIIMap, a1           ; Load address of ASCII map into a1
 
	@CharCopy:
	move.b   (a0)+, d2              ; Move ASCII byte to lower byte of d2
	cmp.b    #0x0, d2               ; Test if byte is zero (string terminator)
	beq.b    @End                   ; If byte was zero, branch to end
	 
	sub.b    #ASCIIStart, d2        ; Subtract first ASCII code to get table entry index
	move.b   (a1,d2.w), d3          ; Move tile ID from table (index in lower word of d2) to lower byte of d3
	add.w    d0, d3                 ; Offset tile ID by first tile ID in font
	move.w   d3, vdp_data           ; Move palette and pattern IDs to VDP data port
	jmp      @CharCopy              ; Next character
	 
	@End:
	rts


LoadMapPlane:
    ; a0 (l) - Map address (ROM)
	; a3 (l) - Plane address
	; d0 (b) - Size in words
	; d1 (b) - Y offset
	; d2 (w) - First tile ID
	; d3 (b) - Palette ID

	mulu.w  #0x0040, d1            ; Multiply Y offset by line width (in words)
	swap    d1                     ; Shift to upper word
	add.l   a3, d1 ; Add PlaneA write cmd + address
	move.l  d1, vdp_control        ; Move dest address to VDP control port

	rol.l   #0x08, d3              ; Shift palette ID to bits 14-15
	rol.l   #0x05, d3              ; Can only rol 8 bits at a time

	subq.b  #0x01, d0              ; Num words in d0, minus 1 for counter
	
	@Copy:
	move.w  (a0)+, d4              ; Move tile ID from map data to lower d4
	and.l   #%0011111111111111, d4 ; Mask out original palette ID
	or.l    d3, d4                 ; Replace with our own
	add.w   d2, d4                 ; Add first tile offset to d4
	move.w  d4, vdp_data           ; Move to VRAM
	dbra    d0, @Copy              ; Loop

    rts	
	
LoadMapPlaneA:
    ; a0 (l) - Map address (ROM)
	; d0 (b) - Size in words
	; d1 (b) - Y offset
	; d2 (w) - First tile ID
	; d3 (b) - Palette ID

	move.l   #vdp_write_plane_a, a3 
	jsr LoadMapPlane

    rts
	
LoadMapPlaneB:
    ; a0 (l) - Map address (ROM)
	; d0 (b) - Size in words
	; d1 (b) - Y offset
	; d2 (w) - First tile ID
	; d3 (b) - Palette ID

	move.l   #vdp_write_plane_b, a3 
	jsr LoadMapPlane

    rts

SetTilePlane:
	; a3 - plane address
	; d0 - tile ID (w)
	; d1 - x in tiles (w)
	; d2 - y in tiles (w)
	; d3 - palette ID (b)
	
	clr.l d4
	move.w d2,d4
	mulu.w  #0x40, d4            ; Multiply Y offset by line width (in words
	add.w d1,d4
	mulu.w #0x2,d4
	
	
	swap    d4                     ; Shift to upper word
	add.l   a3, d4 ; Add PlaneA write cmd + address
	move.l  d4, vdp_control        ; Move dest address to VDP control port

	rol.l   #0x08, d3              ; Shift palette ID to bits 14-15
	rol.l   #0x05, d3              ; Can only rol 8 bits at a time

	clr.l d4
	move.w  d0, d4              ; Move tile ID from map data to lower d4
	and.l   #%0011111111111111, d4 ; Mask out original palette ID
	or.l    d3, d4                 ; Replace with our own
	move.w  d4, vdp_data           ; Move to VRAM
	
	rts
	
SetTilePlaneA:
	; d0 - tile ID (w)
	; d1 - x in tiles (w)
	; d2 - y in tiles (w)
	; d3 - palette ID (b)
	
	move.l  #vdp_write_plane_a, a3 
	jsr SetTilePlane
	
	rts
	
SetTilePlaneB:
	; d0 - tile ID (w)
	; d1 - x in tiles (w)
	; d2 - y in tiles (w)
	; d3 - palette ID (b)
	
	move.l  #vdp_write_plane_b, a3 
	jsr SetTilePlane
	
	rts
	
LoadTiles:
	; a0 - Font address (l)
	; d0 - VRAM address (w)
	; d1 - Num chars (b)
	
	swap	d0						; VRAM addr in upper word
	add.l	#vdp_write_tiles, d0	; VRAM write cmd + VRAM destination address
	move.l	d0, vdp_control			; Send address to VDP cmd port
	
	subq.l	#0x1, d1				; Num chars - 1
	@CharCopy:
	move.w	#0x07, d2				; 8 longwords in tile
	@LongCopy:
	move.l	(a0)+, vdp_data			; Copy one line of tile to VDP data port
	dbra	d2, @LongCopy
	dbra	d1, @CharCopy
	
	rts
	
LoadSpriteTables:
	; a0 - Sprite data address
	; d0 - Number of sprites
	move.l	#vdp_write_sprite_table, vdp_control
	
	subq.b	#0x1, d0				; 2 sprites attributes
	@AttrCopy:
	move.l	(a0)+, vdp_data
	move.l	(a0)+, vdp_data
	dbra	d0, @AttrCopy
	
	rts

SetSpritePosX:
	; Set sprite X position
	; d0 (b) - Sprite ID
	; d1 (w) - X coord
	clr.l	d3						; Clear d3
	move.b	d0, d3					; Move sprite ID to d3
	
	mulu.w	#0x8, d3				; Sprite array offset
	add.b	#0x6, d3				; X coord offset
	swap	d3						; Move to upper word
	add.l	#vdp_write_sprite_table, d3	; Add to sprite attr table (at 0xD400)
	
	move.l	d3, vdp_control			; Set dest address
	move.w	d1, vdp_data			; Move X pos to data port
	
	rts
		
SetSpritePosY:
	; Set sprite Y position
	; d0 (b) - Sprite ID
	; d1 (w) - Y coord
	clr.l	d3						; Clear d3
	move.b	d0, d3					; Move sprite ID to d3
	
	mulu.w	#0x8, d3				; Sprite array offset
	swap	d3						; Move to upper word
	add.l	#vdp_write_sprite_table, d3	; Add to sprite attr table (at 0xD400)
	
	move.l	d3, vdp_control			; Set dest address
	move.w	d1, vdp_data			; Move X pos to data port
	
	rts
	
	
SetSpriteMirrorX:
	; Set sprite X position
	; d0 (b) - Sprite ID
	; d1 (b) - 1 mirror, 0 no mirror
	; a0 (b) - current state of VDP

	clr.l	d3						; Clear d3
	move.b	d0, d3					; Move sprite ID to d3
	
	mulu.w	#0x8, d3				; Sprite array offset
	add.b	#0x4, d3				; mirror offset
	swap	d3						; Move to upper word
	
	add.l	#vdp_write_sprite_table, d3	; Add to sprite attr table (at 0xD400)
	
	move.l	d3, vdp_control			; Set dest address
	
	clr.l d3
	move.b (a0), d3

	and.b #(%11110111), d3
	rol.b #0x3, d1
	or.b d1, d3	; calc (d1 << 3) | (VDP_cont & %11110111)
	move.b d3, (a0)
		
	move.w	(a0), vdp_data			; Move new data to data port
	
	rts
	
SetSpriteMirrorY:
	; Set sprite Y position
	; d0 (b) - Sprite ID
	; d1 (b) - 1 mirror, 0 no mirror
	; a0 (b) - current state of VDP

	clr.l	d3						; Clear d3
	move.b	d0, d3					; Move sprite ID to d3
	
	mulu.w	#0x8, d3				; Sprite array offset
	add.b	#0x4, d3				; mirror offset
	swap	d3						; Move to upper word
	
	add.l	#vdp_write_sprite_table, d3	; Add to sprite attr table (at 0xD400)
	
	move.l	d3, vdp_control			; Set dest address
	
	clr.l d3
	move.b (a0), d3

	and.b #(%11101111), d3
	rol.b #0x4, d1
	or.b d1, d3	; calc (d1 << 4) | (VDP_cont & %11101111)
	move.b d3, (a0)
		
	move.w	(a0), vdp_data			; Move new data to data port
	
	rts
	
	
SetSpriteTileID:
	; Set sprite tile ID
	; d0 (b) - Sprite ID
	; d1 (b) - Tile ID

	clr.l	d3						; Clear d3
	move.b	d0, d3					; Move sprite ID to d3
	
	mulu.w	#0x8, d3				; Sprite array offset
	add.b	#0x4, d3				; mirror/tile offset
	swap	d3						; Move to upper word

	add.l	#vdp_read_sprite_table, d3	; Add to sprite attr table (at 0xD400)
	move.l	d3, vdp_control			; Set dest address

	clr.l d4
	move.w vdp_data,d4
	
	sub.l	#vdp_read_sprite_table, d3

	add.l	#vdp_write_sprite_table, d3	; Add to sprite attr table (at 0xD400)
	move.l	d3, vdp_control			; Set dest address

	and.w #0xFF00,d4
	or.w d1,d4
	
	move.w	d4, vdp_data			; Move new data to data port
	
	rts
	

SetSpritePriorityOff:
	; Set sprite tile ID
	; d0 (b) - Sprite ID

	clr.l	d3						; Clear d3
	move.b	d0, d3					; Move sprite ID to d3
	
	mulu.w	#0x8, d3				; Sprite array offset
	add.b	#0x4, d3				; priority offset
	swap	d3						; Move to upper word

	add.l	#vdp_read_sprite_table, d3	; Add to sprite attr table (at 0xD400)
	move.l	d3, vdp_control			; Set dest address

	clr.l d4
	move.w vdp_data,d4
	
	sub.l	#vdp_read_sprite_table, d3

	add.l	#vdp_write_sprite_table, d3	; Add to sprite attr table (at 0xD400)
	move.l	d3, vdp_control			; Set dest address

	and.w #0x7FFF,d4
	
	move.w	d4, vdp_data			; Move new data to data port
	
	rts
	
	
		
	
	
AnimateSpriteFwd:
	; Advance sprite to next frame, and advance frame counter

	; d0 (w) Sprite address (VRAM)
	; d1 (w) Size of one sprite frame (in tiles)
	; d2 (w) Number of anim frames
	; a0 --- Address of sprite data (ROM)
	; a1 --- Address of animation data (ROM)
	; a2 --- Address of animation frame counter (RAM, writeable)

	clr.l  d3              ; Clear d3
	move.b (a2), d3        ; Read current anim frame number (d3)
	addi.b #0x1, (a2)      ; Advance frame number
	cmp.b  d3, d2          ; Check new frame count with num anim frames
	bne    @NotAtEnd       ; Branch if we haven't reached the end of anim
	move.b #0x0, (a2)      ; At end of anim, wrap frame counter back to zero
	@NotAtEnd:

	move.b (a1,d3.w), d4   ; Get original frame index (d4) from anim data array
	move.b (a2), d2        ; Read next anim frame number (d2)
	move.b (a1,d3.w), d5   ; Get next frame index (d5) from anim data array

	cmp.b  d3, d4          ; Has anim frame index changed?
	beq    @NoChange       ; If not, there's nothing more to do

	; spriteDataAddr = spriteDataAddr + (sizeOfFrame * newTileID)
	move.l a0, d2          ; Move sprite data ROM address to d2 (can't do maths on address registers)
	move.w d1, d4          ; Move size of one sprite frame (in tiles) to d4 (can't trash d1, it's needed later)
	mulu.w #tsize, d4   ; Multiply by size of one tile
	mulu.w d5, d4          ; Multiply with new frame index to get new ROM offset (result in d4)
	add.w  d4, d2          ; Add to sprite data address
	move.l d2, a0          ; Back to address register

	jsr LoadTiles          ; New tile address is in a0, VRAM address already in d0, num tiles already in d1 - jump straight to load tiles

	@NoChange:
	rts

	

WaitForScreenStart:
	move.w  vdp_control, d0	; Move VDP status word to d0
	andi.w  #0x0008, d0     ; AND with bit 4 (vblank), result in status register
	bne     WaitForScreenStart ; Branch if not equal (to zero)
	rts

WaitForScreenEnd:
	move.w  vdp_control, d0	; Move VDP status word to d0
	andi.w  #0x0008, d0     ; AND with bit 4 (vblank), result in status register
	beq     WaitForScreenEnd   ; Branch if equal (to zero)
	rts

WaitFrames:
	; d0 - Number of frames to wait

	move.l  vblank_counter, d1 ; Get start vblank count

	@Wait:
	move.l  vblank_counter, d2 ; Get end vblank count
	subx.l  d1, d2             ; Calc delta, result in d2
	cmp.l   d0, d2             ; Compare with num frames
	bge     @End               ; Branch to end if greater or equal to num frames
	jmp     @Wait              ; Try again
	
	@End:
	rts
	
	
ResetScrolling:
	jsr WaitForScreenStart   ; Wait for start of vblank

	move.w #0x0,d6
	move.l  #vdp_write_hscroll, vdp_control ; Write to start of hscroll data
	move.w  d6, vdp_data                    ; Write hscroll value plane A
	move.w  d6, vdp_data                    ; Write hscroll value plane B
	
	jsr     WaitForScreenEnd ; Wait for end of vblank
	rts

ClearPlane:
	; a0 - Plane address

	move.l   a0, vdp_control        ; Move dest address to VDP control port
	
	clr.l d5
	clr.l d6
	
	move.w #0x20,d6
	
	jmp @RowLoopEnd
	@RowLoop:
	@ColumnLoop:

	move.w  #0x0, vdp_data           ; Move to VRAM
	
	@ColumnLoopEnd:
	dbra d5,@ColumnLoop

	@RowLoopEnd:
	move.w #0x40,d5
	dbra d6,@ColumnLoopEnd
	
    rts
	
ClearPlaneA:
	move.l   #vdp_write_plane_a, a0  
	jsr ClearPlane
	rts
	
ClearPlaneB:
	move.l   #vdp_write_plane_b, a0  
	jsr ClearPlane
	rts
	
	
ClearScreen:
	jsr DeactivateSprites
	jsr ClearPlaneA
	jsr ClearPlaneB
	rts
	
DeactivateSprites:
	lea     DummySpriteDesc, a0		; Sprite table data
	move.w  #0x1, d0			; 3 sprites
	jsr     LoadSpriteTables	
	rts

DummySpriteDesc:
    dc.w 0x0000            ; Y coord (+ 128)
    dc.b (%0101) ; Width (bits 0-1) and height (bits 2-3) in tiles
    dc.b 0x0              ; Index of next sprite (linked list)
    dc.b (%00100000)              ; H/V flipping (bits 3/4), palette index (bits 5-6), priority (bit 7)
    dc.b PlayerTileID     ; Index of first tile
    dc.w 0x0000            ; X coord (+ 128)
	
	
