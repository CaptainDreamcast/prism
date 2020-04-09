; arg1 - X1
; arg2 - Y1
; arg3 - half size X1
; arg4 - half size Y1

; arg5 - X2
; arg6 - Y2
; arg7 - half size X2
; arg8 - half size Y2

; out1 - result

UpdateSingleCollision:

  LDA arg1
  CLC
  ADC arg3
  STA temp1
  LDA arg5 
  SEC            
  SBC arg7        
  STA temp2
  CMP temp1 ; player - sizeX > enemxX + sizeX -> skip
  BCS UpdateSingleCollisionNone

  LDA arg1
  SEC            
  SBC arg3
  STA temp1
  LDA arg5 
  CLC
  ADC arg7      
  STA temp2
  LDA temp1
  CMP temp2 ; player + sizeX < enemxX - sizeX -> skip
  BCS UpdateSingleCollisionNone

  LDA arg2
  CLC
  ADC arg4
  STA temp1
  LDA arg6
  SEC            
  SBC arg8        
  STA temp2
  LDA temp2
  CMP temp1 ; player - sizeY > enemxY + sizeY -> skip
  BCS UpdateSingleCollisionNone
  
  LDA arg2
  SEC            
  SBC arg4 
  STA temp1
  LDA arg6
  CLC
  ADC arg8 
  STA temp2
  LDA temp1
  CMP temp2 ; player + sizeY < enemxY - sizeY -> skip
  BCS UpdateSingleCollisionNone
  
  LDA #$01
  STA out1
  JMP UpdateSingleCollisionDone

UpdateSingleCollisionNone:
  LDA #$00
  STA out1  
  
UpdateSingleCollisionDone:  
  RTS