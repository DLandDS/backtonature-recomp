import unittest

from tools.match_check import parse_objdump


OBJDUMP_WITH_LOCAL_LABEL = """\
00000000 <RenderDialogTextPage>:
   0:  27bdffe0  addiu sp,sp,-32
   4:  10000002  b 10 <.L80018410>
   8:  00000000  nop

0000000c <.L8001840C>:
   c:  0c000000  jal 0 <RenderDialogTextPage>
            c: R_MIPS_26 func_800195F4

00000010 <.L80018410>:
  10:  03e00008  jr ra
  14:  27bd0020  addiu sp,sp,32

00000018 <NextFunction>:
  18:  03e00008  jr ra
  1c:  00000000  nop
"""


class MatchCheckObjdumpTests(unittest.TestCase):
    def test_local_labels_stay_inside_the_owning_function(self):
        functions, relocations, starts = parse_objdump(OBJDUMP_WITH_LOCAL_LABEL)

        self.assertEqual(list(functions), ["RenderDialogTextPage", "NextFunction"])
        self.assertEqual(len(functions["RenderDialogTextPage"]), 6)
        self.assertEqual(relocations["RenderDialogTextPage"], {3: "func_800195F4"})
        self.assertEqual(starts["RenderDialogTextPage"], 0)
        self.assertEqual(starts["NextFunction"], 0x18)


if __name__ == "__main__":
    unittest.main()


class NongpMarkerTests(unittest.TestCase):
    def test_marked_scalar_pointer_and_array_declarations_are_found(self):
        from tools.match_check import nongp_symbols

        source = (
            "extern s32 D_8001009C; /* nongp */\n"
            "extern SystemState *D_8005E3A8; /* nongp */\n"
            "extern u8 D_8005E9C8[];\n"
            "extern s32 D_8005E3B0;\n"
            "extern s16 D_8005E328[4];   /*  nongp  */\n"
        )
        self.assertEqual(nongp_symbols(source),
                         {"D_8001009C", "D_8005E3A8", "D_8005E328"})

    def test_only_marked_extern_hints_are_stripped(self):
        from tools.match_check import strip_extern_hints

        asm = ("\t.extern\tD_8001009C, 4\n"
               "\t.extern\tD_8005E3B0, 4\n"
               "\tlw\t$6,D_8001009C\n")
        self.assertEqual(strip_extern_hints(asm, {"D_8001009C"}),
                         "\t.extern\tD_8005E3B0, 4\n\tlw\t$6,D_8001009C\n")
        self.assertEqual(strip_extern_hints(asm, set()), asm)


class JumpTableAlignTests(unittest.TestCase):
    def test_switch_table_alignment_is_lowered_to_four_bytes(self):
        from tools.match_check import align_jump_tables
        asm = "\t.rdata\n\t.align\t3\n$L214:\n\t.word\t$L203\n\t.text\n"
        self.assertEqual(align_jump_tables(asm),
                         "\t.rdata\n\t.align\t2\n$L214:\n\t.word\t$L203\n\t.text\n")

    def test_ordinary_rdata_alignment_is_left_alone(self):
        from tools.match_check import align_jump_tables
        asm = "\t.rdata\n\t.align\t3\nD_80010000:\n\t.word\t0x3ff00000\n"
        self.assertEqual(align_jump_tables(asm), asm)


class LiteralLowHalfTests(unittest.TestCase):
    """spimdisasm can print a %lo as a literal once its %hi was hoisted."""

    def test_literal_low_half_is_checked_against_the_relocated_symbol(self):
        from tools.match_check import compare
        target = [(0x2682E870, "addiu", "$v0, $s4, -0x1790")]
        actual = [(0x26820000, "addiu", "v0,s4,0")]
        self.assertEqual(compare("f", target, actual, 0x80013B04,
                                 {0: "D_8005E870"}), [])

    def test_wrong_literal_low_half_is_still_a_difference(self):
        from tools.match_check import compare
        target = [(0x2682E874, "addiu", "$v0, $s4, -0x178c")]
        actual = [(0x26820000, "addiu", "v0,s4,0")]
        self.assertNotEqual(compare("f", target, actual, 0x80013B04,
                                    {0: "D_8005E870"}), [])
