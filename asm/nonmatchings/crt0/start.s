nonmatching start, 0x24

glabel start
    /* 1A78 80011278 0680023C */  lui        $v0, %hi(D_8005E378)
    /* 1A7C 8001127C 78E34224 */  addiu      $v0, $v0, %lo(D_8005E378)
    /* 1A80 80011280 0B80033C */  lui        $v1, %hi(D_800B7E20)
    /* 1A84 80011284 207E6324 */  addiu      $v1, $v1, %lo(D_800B7E20)
  .L80011288:
    /* 1A88 80011288 000040AC */  sw         $zero, 0x0($v0)
    /* 1A8C 8001128C 04004224 */  addiu      $v0, $v0, 0x4
    /* 1A90 80011290 2B084300 */  sltu       $at, $v0, $v1
    /* 1A94 80011294 FCFF2014 */  bnez       $at, .L80011288
    /* 1A98 80011298 00000000 */   nop
endlabel start
