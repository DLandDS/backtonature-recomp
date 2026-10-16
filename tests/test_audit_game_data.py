"""Synthetic parser/range fixtures. These do not validate retail game data."""
import hashlib
import io
import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from audit_game_data import ExeView, hash_range, inspect_exe, validate_hdt


class ArchiveAuditTests(unittest.TestCase):
    def test_little_endian_spans_and_last_member(self):
        offsets = [i * 0x104 for i in range(33)]
        result = validate_hdt(struct.pack('<33I', *offsets), offsets[-1])
        self.assertEqual(result['member_count_from_loader'], 32)
        self.assertEqual(result['members'][31]['end'], 0x2080)
        self.assertEqual(result['members'][1]['offset'], 0x104)
        self.assertEqual(result['members'][1]['span_bytes'], 0x104)
        self.assertTrue(result['end_sentinel_equals_bin_size'])
        self.assertIsNone(result['members'][0]['padding_bytes'])

    def test_short_table_rejected(self):
        for size in (0, 128, 131):
            with self.subTest(size=size), self.assertRaises(ValueError):
                validate_hdt(bytes(size), 2048)

    def test_decreasing_or_outside_bin_rejected(self):
        for offsets, size in [([0, 5, 4] + [8]*30, 8), ([0]*32+[9], 8),
                              ([0]*32+[0xFFFFFFFF], 2048)]:
            with self.subTest(offsets=offsets), self.assertRaises(ValueError):
                validate_hdt(struct.pack('<33I', *offsets), size)

    def test_bin_address_space_is_bounded(self):
        for size in (-1, 0x100000000):
            with self.subTest(size=size), self.assertRaises(ValueError):
                validate_hdt(bytes(132), size)

    def test_prefix_tail_and_hdt_extra_are_unclassified(self):
        result = validate_hdt(struct.pack('<33I', *([10]*32+[100])) + b'\xFF\x00', 120)
        self.assertEqual(result['bin_prefix_bytes_unclassified'], 10)
        self.assertEqual(result['bin_tail_bytes_unclassified'], 20)
        self.assertEqual(result['hdt_extra_bytes_unclassified'], 2)
        self.assertFalse(result['hdt_extra_bytes_all_zero'])
        self.assertFalse(result['end_sentinel_equals_bin_size'])
        self.assertEqual(result['members'][0]['span_bytes'], 0)

    def test_sector_read_covers_both_edges(self):
        result = validate_hdt(struct.pack('<33I', *([2047, 2049]+[4096]*31)), 4096)
        member = result['members'][0]
        self.assertEqual((member['start_sector'], member['sector_skip'], member['read_sector_count']),
                         (0, 2047, 2))

    def test_stream_hash_exact_range_and_eof(self):
        data = b'prefix' + b'abc' + b'tail'
        self.assertEqual(hash_range(io.BytesIO(data), 6, 3), hashlib.sha256(b'abc').hexdigest())
        self.assertEqual(hash_range(io.BytesIO(data), len(data), 0), hashlib.sha256(b'').hexdigest())
        with self.assertRaises(ValueError):
            hash_range(io.BytesIO(data), len(data)-1, 2)
        for start, size in ((-1, 0), (0, -1)):
            with self.subTest(start=start, size=size), self.assertRaises(ValueError):
                hash_range(io.BytesIO(data), start, size)


class ExeAuditTests(unittest.TestCase):
    def fixture(self):
        data = bytearray(0x810)
        data[:8] = b'PS-X EXE'
        struct.pack_into('<II', data, 0x18, 0x80010000, 16)
        data[0x800:0x804] = b'abc\0'
        struct.pack_into('<I', data, 0x80C, 0x12345678)
        return bytes(data)

    def test_virtual_address_mapping_and_exact_end(self):
        view = ExeView(self.fixture())
        self.assertEqual(view.string(0x80010000), 'abc')
        self.assertEqual(view.word(0x8001000C), 0x12345678)
        self.assertEqual(view.read(0x80010010, 0), b'')

    def test_bad_pointer_and_size_rejected(self):
        view = ExeView(self.fixture())
        for address, size in ((0x8000FFFF, 1), (0x80010010, 1), (0xFFFFFFFF, 4),
                              (0x80010000, -1), (0x80010000, 0xFFFFFFFF)):
            with self.subTest(address=address, size=size), self.assertRaises(ValueError):
                view.read(address, size)

    def test_truncation_and_magic_rejected(self):
        for data in (b'', bytes(0x800), self.fixture()[:-1]):
            with self.subTest(size=len(data)), self.assertRaises(ValueError):
                ExeView(data)

    def test_unterminated_string_rejected(self):
        with self.assertRaises(ValueError):
            ExeView(self.fixture()).string(0x80010000, 3)

    def test_fixed_tables_require_retail_identity(self):
        with self.assertRaisesRegex(ValueError, 'hashes'):
            inspect_exe(self.fixture())


if __name__ == '__main__':
    unittest.main()
