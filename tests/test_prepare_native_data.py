"""Small synthetic probes plus strict executable identity rejection."""
import copy
import io
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from prepare_native_data import MemberView, build_manifest, inspect_container_0d, inventory_member, prepare


class NativeDataTests(unittest.TestCase):
    def container(self):
        return struct.pack('<BBHI4H', 13, 16, 1, (6 << 24) | 3, 0, 1, 2, 3) + b'abc\0'

    def member(self, data, name='sample', flags='0x00000001'):
        return inventory_member(io.BytesIO(data), {'index': 0, 'offset': 0, 'span_bytes': len(data),
                                                  'name': name, 'flags_raw': flags})

    def test_container_extents_and_member_bounds(self):
        data = self.container()
        result = inspect_container_0d(MemberView(io.BytesIO(b'pre' + data), 3, len(data)))
        self.assertEqual((result['block_count'], result['byte_size']), (1, 20))
        self.assertEqual(result['blocks'][0]['flags_raw'], 6)
        self.assertEqual(result['blocks'][0]['vram_rectangle_words'], [0, 1, 2, 3])
        for offset, size in ((-1, 1), (0, -1), (21, 0), (19, 2)):
            with self.subTest(offset=offset, size=size), self.assertRaises(ValueError):
                MemberView(io.BytesIO(data), 0, len(data)).read(offset, size)
        self.assertEqual(MemberView(io.BytesIO(data), 0, 20).read(20, 0), b'')

    def test_truncated_counts_and_payloads(self):
        data = self.container()
        huge_count = struct.pack('<BBH', 13, 16, 65535)
        huge_length = data[:4] + struct.pack('<I', 0xFFFFFF) + data[8:]
        for bad in (b'', data[:3], data[:-1], huge_count, huge_length, b'bad!'):
            with self.subTest(bad=bad[:4]), self.assertRaises(ValueError):
                inspect_container_0d(MemberView(io.BytesIO(bad), 0, len(bad)))
        with self.assertRaises(ValueError):
            MemberView(io.BytesIO(b'a'), 0, 20).read(0, 20)

    def test_overlay_id_does_not_masquerade_as_resource_magic(self):
        result = self.member(self.container(), 'Obj\\GF_STAFF.Bin', '0x00000000')
        self.assertEqual(result['head_format'], 'ps1-overlay-by-executable-metadata')
        self.assertNotIn('container', result)
        self.assertEqual(self.member(self.container())['head_format'], 'container0d-prefix')
        self.assertEqual(self.member(b'pBAV' + bytes(12))['head_format'], 'vab-header-prefix')
        self.assertEqual(self.member(b'unknown')['head_format'], 'unknown')

    def test_animation_offsets_and_message_slots(self):
        animation = struct.pack('<9I', 14, 0, 0, 0, 36, 36, 36, 36, 37)
        result = self.member(animation)
        self.assertEqual(result['head_format'], 'animation0e-header')
        self.assertFalse(result['table_offsets_within_span'])
        data = bytes([255]) * 4 + bytes(8188)
        data += struct.pack('<I', 7) + self.container() + bytes(8192 - 24)
        result = self.member(data, 'objcg\\messege0.bin')
        self.assertEqual(result['head_format'], 'message-slot-array0d')
        self.assertEqual(result['message_slots']['unused_marker'], 1)
        self.assertEqual(result['message_slots']['valid_container0d'], 1)
        self.assertEqual(result['first_container_sample']['member_offset'], 8196)

    def test_manifest_indices_names_and_raw_flags(self):
        spans = {'members': [{'index': i, 'offset': i * 4, 'span_bytes': 4} for i in range(32)]}
        raw = b'Obj\\GF_TITLE.Bin\0'.ljust(32, b'\0')
        exe = {'afile_records_before_init': [{'index': i, 'name_bytes_hex': raw.hex(),
                                              'flags_raw': '0x00000000'} for i in range(32)]}
        manifest = build_manifest(spans, exe, {})
        self.assertEqual(manifest['members'][27]['name'], 'Obj\\GF_TITLE.Bin')
        for key, value in (('index', 99), ('name_bytes_hex', '00'), ('flags_raw', '0x100000000')):
            bad = copy.deepcopy(exe)
            bad['afile_records_before_init'][0][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                build_manifest(spans, bad, {})
        with self.assertRaises(ValueError):
            build_manifest({'members': []}, exe, {})

    def test_wrong_executable_rejected_before_any_output(self):
        with tempfile.TemporaryDirectory(prefix='btn-native-data-') as directory:
            root = Path(directory)
            (root / 'SLUS_011.15').write_bytes(b'PS-X EXE' + bytes(323584 - 8))
            output = root / 'output'
            with self.assertRaisesRegex(ValueError, 'hashes'):
                prepare(root, output)
            self.assertFalse(output.exists())
            output.mkdir()
            marker = output / 'keep.txt'
            marker.write_text('keep')
            with self.assertRaisesRegex(ValueError, 'already exists'):
                prepare(root, output)
            self.assertEqual(marker.read_text(), 'keep')


if __name__ == '__main__':
    unittest.main()
