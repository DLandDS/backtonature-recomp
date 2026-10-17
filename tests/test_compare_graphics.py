import importlib.util
from pathlib import Path
import tempfile
import unittest

SPEC=importlib.util.spec_from_file_location('compare_graphics',Path(__file__).resolve().parents[1]/'tools/compare_graphics.py')
COMPARE=importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(COMPARE)


class ComparisonTests(unittest.TestCase):
    def test_metrics_and_invalid_inputs(self):
        with tempfile.TemporaryDirectory() as folder:
            first,second=Path(folder)/'first.ppm',Path(folder)/'second.ppm'
            first.write_bytes(b'P6\n2 1\n255\n'+bytes([1,2,3,4,5,6]))
            second.write_bytes(first.read_bytes())
            report,diff=COMPARE.compare(first,second)
            self.assertTrue(report['exact_match'])
            self.assertTrue(diff.endswith(bytes(6)))
            second.write_bytes(b'P6\n2 1\n255\n'+bytes([1,2,3,9,5,0]))
            report,diff=COMPARE.compare(first,second)
            self.assertEqual(report['different_pixels'],1)
            self.assertEqual(report['maximum_channel_error'],6)
            self.assertAlmostEqual(report['mean_channel_error'],11/6)
            self.assertTrue(diff.endswith(bytes([0,0,0,5,0,6])))
            second.write_bytes(b'P6\n1 1\n255\n'+bytes(3))
            with self.assertRaises(ValueError): COMPARE.compare(first,second)
            second.write_bytes(b'P6\n1 1\n255\n'+bytes(2))
            with self.assertRaises(ValueError): COMPARE.read_ppm(second)


if __name__=='__main__': unittest.main()
