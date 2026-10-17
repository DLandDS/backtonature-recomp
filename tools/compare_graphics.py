#!/usr/bin/env python3
"""Compare two P6 PPM captures without resizing, alignment, or tolerance.

Use matching display crops and identical scene/input/tick for PS1 comparisons.
Outputs must be new files. An image difference exits 1; bad inputs exit 2.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import sys


def read_ppm(path):
    data = path.read_bytes()
    match = re.match(rb'P6\s+(\d+)\s+(\d+)\s+255(?:\r\n|\n|\s)', data)
    if not match:
        raise ValueError(f'{path}: expected P6 PPM with max value 255 (no comments)')
    width, height = map(int, match.groups())
    pixels = data[match.end():]
    if width <= 0 or height <= 0 or len(pixels) != width*height*3:
        raise ValueError(f'{path}: invalid dimensions or RGB payload length')
    return (width, height), pixels


def compare(reference, actual):
    shape, first = read_ppm(reference)
    actual_shape, second = read_ppm(actual)
    if shape != actual_shape:
        raise ValueError('Captures have different dimensions; supply matching display crops')
    diff = bytes(abs(a-b) for a,b in zip(first,second))
    changed = sum(any(diff[i:i+3]) for i in range(0,len(diff),3))
    report = {
        'format': 'btn-graphics-comparison-v1', 'width': shape[0], 'height': shape[1],
        'reference': str(reference.resolve()), 'actual': str(actual.resolve()),
        'reference_file_sha256': hashlib.sha256(reference.read_bytes()).hexdigest(),
        'actual_file_sha256': hashlib.sha256(actual.read_bytes()).hexdigest(),
        'reference_rgb_sha256': hashlib.sha256(first).hexdigest(),
        'actual_rgb_sha256': hashlib.sha256(second).hexdigest(),
        'different_pixels': changed, 'total_pixels': shape[0]*shape[1],
        'maximum_channel_error': max(diff), 'mean_channel_error': sum(diff)/len(diff),
        'exact_match': not changed,
        'scope': 'Pixel comparison only; caller must establish reference origin and matching scene/input/tick',
    }
    return report, b'P6\n%d %d\n255\n' % shape + diff


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--actual', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--diff', type=Path, required=True)
    parser.add_argument('--scene', required=True, help='Scene, resource/member/offset and input sequence identity')
    parser.add_argument('--tick', type=int, required=True)
    args = parser.parse_args()
    try:
        if args.tick < 0:
            raise ValueError('Tick must be nonnegative')
        if args.report.exists() or args.diff.exists() or args.report.resolve() == args.diff.resolve():
            raise ValueError('Report and diff must be distinct new output files')
        report, difference = compare(args.reference,args.actual)
        report.update(scene=args.scene,tick=args.tick)
        # Exclusive creation also protects aliases of either input.
        with args.diff.open('xb') as output:
            output.write(difference)
        with args.report.open('x',encoding='utf-8') as output:
            json.dump(report,output,indent=2)
            output.write('\n')
        print(f'graphics-compare: {report["different_pixels"]}/{report["total_pixels"]} pixels differ; '
              f'max channel error {report["maximum_channel_error"]}')
        return 0 if report['exact_match'] else 1
    except (OSError, ValueError) as error:
        print(f'graphics-compare failed: {error}',file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
