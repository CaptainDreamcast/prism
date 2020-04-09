 SeparateNumberIntoRegisters:
	; d1 - Number XYZ
	; (return) d1 - X
	; (return) d2 - Y
	; (return) d3 - Z

	and.l #0xFFF,d1
	divu.w #100,d1
	move.l d1,d2
	swap d2
	and.l #0xF,d1
	
	and.l #0xFFF,d2
	divu.w #10,d2
	move.l d2,d3
	swap d3
	and.l #0xF,d2
	and.l #0xF,d3
	
	rts
	
ClearW:
	; a0 - address (l)
	; d0 - length in words (w)
	; d1 - value (w)

	jmp @LoopEnd
	@Loop:
	move.w d1,(a0)+
	
	@LoopEnd:
	dbra d0,@Loop
	
	rts