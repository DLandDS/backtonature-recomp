import importlib.util
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))


class TextBankTests(unittest.TestCase):
    def setUp(self):
        self.assertIsNotNone(importlib.util.find_spec('text_bank'), 'Readable text bank build is not implemented')
        import text_bank
        self.bank = text_bank

    def test_rebuilds_original_bank_bytes(self):
        payload = self.bank.load_bank(ROOT / 'src/text/text_bank.json')
        baserom = (ROOT / 'baserom/SLUS_011.15').read_bytes()
        self.assertEqual(payload, baserom[0x41F5C:0x41F5C + 13406])

    def test_preserves_labels_and_other_data(self):
        original = (ROOT / 'asm/data/39314.data.s').read_text()
        generated = self.bank.replace_bank(original, self.bank.load_bank(ROOT / 'src/text/text_bank.json'))
        import re
        self.assertEqual(re.findall(r'^(?:dlabel|enddlabel|nonmatching) .*$', generated, re.M),
                         re.findall(r'^(?:dlabel|enddlabel|nonmatching) .*$', original, re.M))
        self.assertIn('/* 453BC 80054BBC 00000000 */ .word 0x00000000', generated)

    def test_extracts_bank_from_baserom_layout(self):
        import json
        layout = json.loads((ROOT / 'src/text/text_bank.layout.json').read_text(encoding='utf-8'))
        self.assertTrue(all('text' not in entry for entry in layout['entries']),
                        'The committed layout must not carry game text')
        baserom = (ROOT / 'baserom/SLUS_011.15').read_bytes()
        document = self.bank.extract_bank(layout, baserom)
        self.assertEqual(self.bank.encode_bank(document), baserom[0x41F5C:0x41F5C + 13406])

    def test_rejects_overlapping_or_missing_entries(self):
        import copy
        import json
        document = json.loads((ROOT / 'src/text/text_bank.json').read_text())
        for mutate in ('overlap', 'missing', 'overflow'):
            changed = copy.deepcopy(document)
            if mutate == 'overlap':
                changed['entries'][1]['offset'] = 0
            elif mutate == 'missing':
                changed['entries'].pop(0)
            else:
                changed['entries'][0]['text'] = 'A' * 10
            with self.subTest(case=mutate), self.assertRaises(ValueError):
                self.bank.encode_bank(changed)


if __name__ == '__main__':
    unittest.main()
