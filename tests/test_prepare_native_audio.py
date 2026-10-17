import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from prepare_native_audio import bank_layout


class AudioExportTests(unittest.TestCase):
    def test_bounds_and_reserved_sizes(self):
        header=bytearray(0xC20)
        header[:4]=b'pBAV'
        struct.pack_into('<I',header,4,7)
        struct.pack_into('<3H',header,18,1,1,1)
        struct.pack_into('<H',header,0xA22,4)
        layout=bank_layout(header,32)
        self.assertEqual(layout['sample_bytes'],[32])
        self.assertEqual(layout['header_bytes'],0xC20)
        for data,body in ((header,31),(header[:-1],32),(b'bad',32)):
            with self.assertRaises(ValueError): bank_layout(data,body)
        struct.pack_into('<H',header,0xA24,1)
        with self.assertRaises(ValueError): bank_layout(header,32)


if __name__=='__main__':
    unittest.main()
