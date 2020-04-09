  .bank 0
  .org $C000 
RESET:
  SEI          ; disable IRQs
  CLD          ; disable decimal mode
  LDX #$40
  STX $4017    ; disable APU frame IRQ
  LDX #$FF
  TXS          ; Set up stack
  INX          ; now X = 0
  STX $2000    ; disable NMI
  STX $2001    ; disable rendering
  STX $4010    ; disable DMC IRQs

  JSR WaitForVblank
  JSR clrmem 
  JSR WaitForVblank

  LDA #$05
  STA randomValue

  JSR __init

Forever:
  JMP Forever     ;jump back to Forever, infinite loop

NMI:
UpdateGame:
  LDA #$00
  STA $2003       ; set the low byte (00) of the RAM address
  LDA #$02
  STA $4014       ; set the high byte (02) of the RAM address, start the transfer

  JSR RNG

  JSR __update

FrameFinish:  
  RTI             ; return from interrupt