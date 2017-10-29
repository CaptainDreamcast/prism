randomize_a_sign
        sta     TestPosition
        jsr     Random
        cmpa    #-60
        bgt     randomize_a_sign_neg
        lda     TestPosition
        nega
        sta     TestPosition
randomize_a_sign_neg

        lda     TestPosition

        rts

update_drawing:
        jsr     Wait_Recal              ; Vectrex BIOS recalibration
        lda     #$80                    ; scaling factor of $80 to A
        sta     VIA_t1_cnt_lo           ; move to time 1 lo, this
                                        ; means scaling

        jsr     Intensity_5F            ; Sets the intensity of the
                                        ; vector beam to $5

        rts




init_general
        jsr     Read_Btns  ; for some reason first time buttons are always active
        lda     #0
        sta     Button1Flank
        rts


read_button1_flank
        jsr     Read_Btns

        ldb     Vec_Button_1_1

        lda     Button1Flank
        cmpa    #0
        bne     read_flank_zero1

        cmpb    #0
        beq     read_flank_zero1

        lda     #1
        jmp     read_flank_over1
read_flank_zero1
        lda     #0
			
read_flank_over1		
			
        stb     Button1Flank

        rts