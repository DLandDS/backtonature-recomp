import importlib.util
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))


class TextCodecTests(unittest.TestCase):
    def setUp(self):
        self.assertIsNotNone(importlib.util.find_spec('text_codec'), 'The readable text codec is not implemented')
        import text_codec
        self.codec = text_codec

    def test_every_halfword_round_trips(self):
        for word in range(65536):
            with self.subTest(word=word):
                self.assertEqual(self.codec.encode(self.codec.decode([word])), [word])

    def test_ligatures_and_verified_apostrophe(self):
        self.assertEqual(self.codec.encode("Can't John's I'll '"), [2, 32, 45, 80, 4093, 9, 46, 39, 45, 81, 4093, 8, 82, 43, 4093, 141])

    def test_control_codes_and_substitutions(self):
        self.assertEqual(self.codec.encode('Hi {PLAYER_NAME}\n{PAGE_MARK}{END}'), [7, 40, 4093, 16394, 65534, 4092, 65535])

    def test_quote_variants_preserve_distinct_glyphs(self):
        self.assertEqual(self.codec.encode(self.codec.decode([0x61, 0x65, 0x8D, 0x8E])), [0x61, 0x65, 0x8D, 0x8E])

    def test_padding_after_terminator(self):
        self.assertEqual(self.codec.decode([0xFFFF, 0, 0]), '{END}{PAD:2}')
        self.assertEqual(self.codec.encode('{END}{PAD:2}'), [65535, 0, 0])

    def test_does_not_add_a_terminator(self):
        self.assertEqual(self.codec.encode('A'), [0])

    def test_rejects_unknown_and_out_of_range_tokens(self):
        for text in ('{BOGUS}', '{RAW:10000}', '{RAW:-1}', '{PAD:-2}', '{END', '\u2603'):
            with self.subTest(text=text), self.assertRaises(ValueError):
                self.codec.encode(text)

    def test_literal_expansion(self):
        source = '/* TEXT("ignore") */\nu16 words[] = TEXT(" Yes\\n No{END}");\n'
        expanded = self.codec.expand_source(source)
        self.assertIn('/* TEXT("ignore") */', expanded)
        self.assertIn('{0x0FFD, 0x0018, 0x0024, 0x0032, 0xFFFE, 0x0FFD, 0x000D, 0x002E, 0xFFFF}', expanded)

    def test_rejects_overlong_member(self):
        with self.assertRaises(ValueError):
            self.codec.expand_source('.small = TEXT("AB{END}");', {'small': 2})

    def test_adjacent_literals(self):
        self.assertEqual(self.codec.expand_source('TEXT("A" "B{END}")'), '{0x0000, 0x0001, 0xFFFF}')


if __name__ == '__main__':
    unittest.main()
