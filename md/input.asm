; Taken in part from https://bigevilcorporation.co.uk
; So buy Tanglewood, will ya

ReadPad1:
	; GamePadState (w) - Return result (00SA0000 00CBRLDU)

	clr.l d0
	move.b  pad_data_a, d0     ; Read upper byte from data port
	rol.w   #0x8, d0           ; Move to upper byte of d0
	move.b  #0x40, pad_data_a  ; Write bit 7 to data port
	move.b  pad_data_a, d0     ; Read lower byte from data port
	move.b  #0x00, pad_data_a  ; Put data port back to normal

	clr.l d1
	clr.l d3
	move.w gGamePadState,d1	
	move.w d1,d3
	eor.w d0, d1
	and.w d3, d1
	not.w d1
	
	move.w d1, gGamePadFlankState

	move.w d0, gGamePadState
	
	rts
	

pad_data_a				equ 0x00A10003
pad_data_b				equ 0x00A10005
pad_data_c				equ 0x00A10007
pad_ctrl_a				equ 0x00A10009
pad_ctrl_b				equ 0x00A1000B
pad_ctrl_c				equ 0x00A1000D

pad_button_up           equ 0x0
pad_button_down         equ 0x1
pad_button_left         equ 0x2
pad_button_right        equ 0x3
pad_button_a            equ 0xC
pad_button_b            equ 0x4
pad_button_c            equ 0x5
pad_button_start        equ 0xD