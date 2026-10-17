import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from audit_native_title import inspect_overlay


class TitleAuditTests(unittest.TestCase):
    def test_dispatch_bounds_and_layout(self):
        base = 0x800B7E20
        entry, table = 0x800B7EEC-base, 0x800C49F8-base
        data = bytearray(table+32)
        struct.pack_into('<I',data,0,12)
        struct.pack_into('<I',data,entry,0x27BDFFE8)
        struct.pack_into('<I',data,entry+16,0x248449F8)
        struct.pack_into('<8I',data,table,*([base+entry]*8))
        self.assertEqual(inspect_overlay(data,base)['entry_offset'],0xCC)
        for raw,load in ((data[:100],base),(data[:-1],base),(data,0x800C0000)):
            with self.assertRaises(ValueError):
                inspect_overlay(raw,load)
        for offset,value in ((0,13),(entry,0),(entry+16,0),(table,base-4),(table,base+1),(table,base+len(data))):
            broken = bytearray(data)
            struct.pack_into('<I',broken,offset,value)
            with self.assertRaises(ValueError):
                inspect_overlay(broken,base)


if __name__ == '__main__':
    unittest.main()
