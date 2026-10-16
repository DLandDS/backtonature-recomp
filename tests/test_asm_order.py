import unittest

from tools.asm_order import extract_source_order, place_rodata_after_include, reorder

# The shape cc1 actually emits: declarations in source order, bodies in the
# wrong one, with top-level asm() hoisted ahead of every compiled function.
CC1_OUTPUT = """\t.file\t1 "src/unit.c"
gcc2_compiled.:
\t.text
\t.align\t2
\t.globl\tfunc_A
 #APP
\t.include "macro.inc"
.section .text
.set noat
.set noreorder
.include "asm/matchings/unit/func_B.s"
.set reorder
.set at

 #NO_APP
\t.align\t2
\t.globl\tfunc_C
\t.text
\t.ent\tfunc_A
func_A:
\tli\t$2,1
\t.end\tfunc_A
\t.text
\t.ent\tfunc_C
func_C:
\tli\t$2,3
\t.end\tfunc_C
"""

CC1_HOISTED_DECLARATIONS = """\t.file\t1 "src/unit.c"
\t.globl\tfunc_A
\t.globl\tfunc_C
 #APP
\t.include "macro.inc"
.section .text
.set noat
.set noreorder
.include "asm/nonmatchings/unit/func_B.s"
.set reorder
.set at
 #NO_APP
\t.text
\t.ent\tfunc_A
func_A:
\tli\t$2,1
\t.end\tfunc_A
\t.text
\t.ent\tfunc_C
func_C:
\tli\t$2,3
\t.end\tfunc_C
"""

CC1_MARKERLESS_ASM = """\t.file\t1 "src/unit.c"
\t.globl\tfunc_A
\t.globl\tfunc_C
\t.include "macro.inc"
.section .text
.set noat
.set noreorder
.include "asm/nonmatchings/unit/func_B.s"
.set reorder
.set at
\t.text
\t.ent\tfunc_A
func_A:
\tli\t$2,1
\t.end\tfunc_A
\t.text
\t.ent\tfunc_C
func_C:
\tli\t$2,3
\t.end\tfunc_C
"""

C_SOURCE = """\
void func_A(void)
{
}
INCLUDE_ASM("asm/nonmatchings/unit", func_B);
int *func_C(int value)
{
    return 0;
}
"""

GROUPED_SOURCE = """\
void func_A(void) { }
INCLUDE_ASM("asm/nonmatchings/unit", func_B);
void func_C(void) { }
INCLUDE_ASM("asm/nonmatchings/unit", func_D);
"""

CC1_GROUPED_APP = """ #APP
\t.include "macro.inc"
.section .text
.set noat
.set noreorder
.include "asm/nonmatchings/unit/func_B.s"
.set reorder
.set at
\t.include "macro.inc"
.section .text
.set noat
.set noreorder
.include "asm/nonmatchings/unit/func_D.s"
.set reorder
.set at
 #NO_APP
\t.globl\tfunc_A
\t.globl\tfunc_C
\t.ent\tfunc_A
func_A:
\t.end\tfunc_A
\t.ent\tfunc_C
func_C:
\t.end\tfunc_C
"""

CC1_UNTERMINATED_ASM_ONLY = """ #APP
\t.include "macro.inc"
.section .text
.set noat
.set noreorder
.include "asm/nonmatchings/unit/func_B.s"
.set reorder
.set at

\t.text
"""


def body_order(text: str) -> list[str]:
    out = []
    for line in text.splitlines():
        s = line.strip()
        if s.split(maxsplit=1)[:1] == [".ent"]:
            out.append(s.split()[1])
        elif ".include " in s and "macro.inc" not in s:
            out.append(s.rsplit("/", 1)[1].removesuffix('.s"'))
    return out


class AsmOrderTests(unittest.TestCase):
    def test_c_rodata_can_follow_an_asm_data_include(self):
        compiled = """.globl D_80010358
.rdata
.align 2
D_80010358:
.word 0
.word func_A
.text
.include "macro.inc"
.section .text
.set noat
.set noreorder
.include "asm/nonmatchings/ui/D_80010318.s"
.set reorder
.set at
.text
.include "macro.inc"
.section .text
.set noat
.set noreorder
.include "asm/nonmatchings/ui/func_B.s"
.set reorder
.set at
"""
        ordered = place_rodata_after_include(compiled, "D_80010358", "D_80010318")

        self.assertLess(ordered.index('D_80010318.s"'), ordered.index("D_80010358:"))
        self.assertLess(ordered.index("D_80010358:"), ordered.index('func_B.s"'))
        self.assertEqual(ordered.count("D_80010358:"), 1)

    def test_source_order_overrides_cc1s_grouped_c_declarations(self):
        order = extract_source_order(C_SOURCE)
        out, undeclared = reorder(CC1_HOISTED_DECLARATIONS, order)

        self.assertEqual(order, ["func_A", "func_B", "func_C"])
        self.assertEqual(body_order(out), order)
        self.assertEqual(undeclared, [])

    def test_markerless_include_block_is_moved_as_one_body(self):
        order = extract_source_order(C_SOURCE)
        out, undeclared = reorder(CC1_MARKERLESS_ASM, order)

        self.assertEqual(body_order(out), order)
        self.assertEqual(out.count('.include "macro.inc"'), 1)
        self.assertEqual(undeclared, [])

    def test_grouped_app_region_is_split_into_independent_asm_bodies(self):
        order = extract_source_order(GROUPED_SOURCE)
        out, undeclared = reorder(CC1_GROUPED_APP, order)

        self.assertEqual(order, ["func_A", "func_B", "func_C", "func_D"])
        self.assertEqual(body_order(out), order)
        self.assertEqual(undeclared, [])

    def test_unterminated_asm_only_region_keeps_trailing_text_directive(self):
        out, undeclared = reorder(CC1_UNTERMINATED_ASM_ONLY, ["func_B"])

        self.assertEqual(body_order(out), ["func_B"])
        self.assertIn("\t.text", out)
        self.assertEqual(undeclared, [])

    def test_hoisted_asm_is_put_back_between_the_c_functions(self):
        # This is the whole point: cc1 emits B, A, C and the unit must be A, B, C
        # or every address after the first is wrong once it is linked.
        self.assertEqual(body_order(CC1_OUTPUT), ["func_B", "func_A", "func_C"])
        out, undeclared = reorder(CC1_OUTPUT)
        self.assertEqual(body_order(out), ["func_A", "func_B", "func_C"])
        self.assertEqual(undeclared, [])

    def test_every_body_survives_exactly_once(self):
        out, _ = reorder(CC1_OUTPUT)
        for marker in ("func_A:", "func_C:", "func_B.s"):
            self.assertEqual(out.count(marker), 1, marker)
        self.assertIn("li\t$2,1", out)
        self.assertIn("li\t$2,3", out)

    def test_a_body_with_no_declaration_is_reported(self):
        # A static function has no .globl, so its source position cannot be
        # recovered. Silently guessing would mis-place code, so it is reported.
        text = CC1_OUTPUT.replace("\t.globl\tfunc_C\n", "")
        out, undeclared = reorder(text)
        self.assertEqual(undeclared, ["func_C"])
        self.assertIn("func_C:", out)

    def test_nested_function_moves_with_its_parent(self):
        # GNU C nested functions come out as `name.N` just before their parent.
        # They are not in the source order; they must stay ahead of the parent.
        text = CC1_OUTPUT.replace(
            "\t.text\n\t.ent\tfunc_C\n",
            "\t.text\n\t.ent\tfunc_N.12\nfunc_N.12:\n\tli\t$2,9\n\t.end\tfunc_N.12\n"
            "\t.text\n\t.ent\tfunc_C\n")
        out, undeclared = reorder(text)
        self.assertEqual(body_order(out), ["func_A", "func_B", "func_N.12", "func_C"])
        self.assertEqual(undeclared, [])

    def test_pure_c_unit_is_left_in_source_order(self):
        text = CC1_OUTPUT.replace(
            ' #APP\n\t.include "macro.inc"\n.section .text\n.set noat\n'
            '.set noreorder\n.include "asm/matchings/unit/func_B.s"\n'
            '.set reorder\n.set at\n\n #NO_APP\n', "")
        out, undeclared = reorder(text)
        self.assertEqual(body_order(out), ["func_A", "func_C"])
        self.assertEqual(undeclared, [])


if __name__ == "__main__":
    unittest.main()
