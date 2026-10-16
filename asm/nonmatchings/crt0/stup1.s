/* Handwritten function */
nonmatching stup1, 0x78

glabel stup1
    /* 1A9C 8001129C 04000224 */  addiu      $v0, $zero, 0x4
    /* 1AA0 800112A0 00000000 */  nop
    /* 1AA4 800112A4 00000000 */  nop
    /* 1AA8 800112A8 00000000 */  nop
    /* 1AAC 800112AC 00000000 */  nop
    /* 1AB0 800112B0 0180043C */  lui        $a0, %hi(D_80011324)
    /* 1AB4 800112B4 24138424 */  addiu      $a0, $a0, %lo(D_80011324)
    /* 1AB8 800112B8 21208200 */  addu       $a0, $a0, $v0
    /* 1ABC 800112BC 0000828C */  lw         $v0, 0x0($a0)
    /* 1AC0 800112C0 0080083C */  lui        $t0, %hi(D_80000004)
    /* 1AC4 800112C4 25E84800 */  or         $sp, $v0, $t0
    /* 1AC8 800112C8 0B80043C */  lui        $a0, %hi(D_800B7E20)
    /* 1ACC 800112CC 207E8424 */  addiu      $a0, $a0, %lo(D_800B7E20)
    /* 1AD0 800112D0 C0200400 */  sll        $a0, $a0, 3
    /* 1AD4 800112D4 C2200400 */  srl        $a0, $a0, 3
    /* 1AD8 800112D8 0580033C */  lui        $v1, %hi(D_8005648C)
    /* 1ADC 800112DC 8C64638C */  lw         $v1, %lo(D_8005648C)($v1)
    /* 1AE0 800112E0 00000000 */  nop
    /* 1AE4 800112E4 23284300 */  subu       $a1, $v0, $v1
    /* 1AE8 800112E8 2328A400 */  subu       $a1, $a1, $a0
    /* 1AEC 800112EC 25208800 */  or         $a0, $a0, $t0
    /* 1AF0 800112F0 0680013C */  lui        $at, %hi(D_8005E378)
    /* 1AF4 800112F4 78E33FAC */  sw         $ra, %lo(D_8005E378)($at)
    /* 1AF8 800112F8 06801C3C */  lui        $gp, %hi(_gp)
    /* 1AFC 800112FC 74E29C27 */  addiu      $gp, $gp, %lo(_gp)
    /* 1B00 80011300 21F0A003 */  addu       $fp, $sp, $zero
    /* 1B04 80011304 6909010C */  jal        InitHeap
    /* 1B08 80011308 04008420 */   addi      $a0, $a0, %lo(D_80000004) /* handwritten instruction */
    /* 1B0C 8001130C 06801F3C */  lui        $ra, %hi(D_8005E378)
    /* 1B10 80011310 78E3FF8F */  lw         $ra, %lo(D_8005E378)($ra)
endlabel stup1
    /* 1B14 80011314 00000000 */  nop
