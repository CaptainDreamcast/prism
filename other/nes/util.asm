; arg1 1s
; arg2 10s
; out1 1s
; out2 2s
IncreaseTimer:
  LDX arg1
  LDY arg2
  INX 
  CPX #$0A
  BNE IncreaseTimerDone

  CPY #$09
  BEQ IncreaseTimerNoIncrease

  LDX #$00
  INY

  JMP IncreaseTimerDone
IncreaseTimerNoIncrease:
  DEX

IncreaseTimerDone:
  STX out1
  STY out2

  RTS
  
; arg1 1s
; arg2 10s
; arg3 amount
; out1 1s
; out2 2s  
IncreaseTimerAmount
IncreaseTimerAmountLoop
  LDA arg3
  CMP #$00
  BEQ IncreaseTimerAmountDone
  JSR IncreaseTimer
  LDX out1
  STX arg1
  LDX out2
  STX arg2
  LDX arg3
  DEX
  STX arg3
  JMP IncreaseTimerAmountLoop
IncreaseTimerAmountDone:

  LDX arg1
  STX out1
  LDX arg2
  STX out2
  RTS
  
RNG:
  LDA randomValue
  ASL A
  ASL A
  CLC
  ADC randomValue
  CLC
  ADC #03
  STA randomValue
  CMP #$F0
  BCS RNG
  LDA #10
  CMP randomValue
  BCS RNG
  RTS
  
clrmem:
clrmemLoop:
  LDA #$00
  STA $0000, x
  STA $0100, x
  STA $0300, x
  STA $0400, x
  STA $0500, x
  STA $0600, x
  STA $0700, x
  LDA #$FE
  STA $0200, x
  INX
  BNE clrmemLoop
  RTS