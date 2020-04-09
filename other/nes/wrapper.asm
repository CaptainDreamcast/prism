  .inesprg 1   ; 1x 16KB PRG code
  .ineschr 1   ; 1x  8KB CHR data
  .inesmap 0   ; mapper 0 = NROM, no bank swapping
  .inesmir 1   ; background mirroring
  
  .include "../addons/prism/other/nes/variables.asm"
  .include "variables.asm"
  
  .include "../addons/prism/other/nes/logic.asm"
  .include "../addons/prism/other/nes/util.asm"
  .include "../addons/prism/other/nes/collision.asm"
  .include "../addons/prism/other/nes/input.asm"
  .include "../addons/prism/other/nes/ppu.asm"
