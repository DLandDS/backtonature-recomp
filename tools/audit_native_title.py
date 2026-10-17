#!/usr/bin/env python3
"""Record the verified US title overlay/resource map without exporting game code.

This is an inventory, not a decompiler or proof of native/hardware parity.
Output must be new and local, normally under ignored build/.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

from audit_game_data import ExeView, file_identity, inspect_exe, validate_hdt
from prepare_native_data import MemberView, inspect_container_0d


def inspect_overlay(data, base):
    entry = 0x800B7EEC-base
    table = 0x800C49F8-base
    if entry < 0 or table < 0 or len(data) < max(entry+20,table+32):
        raise ValueError('Title overlay does not cover the audited entry/dispatch table')
    if struct.unpack_from('<I',data)[0] != 12 or struct.unpack_from('<I',data,entry)[0] != 0x27BDFFE8:
        raise ValueError('Title overlay header/entry differs from the audited layout')
    if struct.unpack_from('<I',data,entry+16)[0] != 0x248449F8:
        raise ValueError('Title dispatch table operand differs from the audited layout')
    handlers = struct.unpack_from('<8I',data,table)
    if any(address % 4 or not base <= address < base+len(data) for address in handlers):
        raise ValueError('Title dispatch handler exceeds overlay')
    return dict(member=27,span_bytes=len(data),sha256=hashlib.sha256(data).hexdigest(),
                load_address=f'0x{base:08X}',entry_address='0x800B7EEC',entry_offset=entry,
                dispatch_table_address='0x800C49F8',handlers=[f'0x{p:08X}' for p in handlers],
                native_scope='Start/menu branch only; intro, attract mode, scenery and name entry incomplete')


def audit(directory):
    exe_path = directory/'SLUS_011.15'
    raw = exe_path.read_bytes()
    inspect_exe(raw)  # Fixed addresses only after pristine US hash verification.
    exe = ExeView(raw)
    bin_path = directory/'A_FILE.BIN'
    spans = validate_hdt((directory/'A_FILE.HDT').read_bytes(),bin_path.stat().st_size)['members']
    with bin_path.open('rb') as stream:
        overlay = MemberView(stream,spans[27]['offset'],spans[27]['span_bytes'])
        result = inspect_overlay(overlay.read(0,overlay.size),exe.word(0x80010098))
        member0 = MemberView(stream,spans[0]['offset'],spans[0]['span_bytes'])
        header = member0.read(8192,36)
        if struct.unpack_from('<I',header)[0] != 14:
            raise ValueError('Title common animation 0E signature missing')
        images,lists,parts,ranges,frames = struct.unpack_from('<4HI',header,4)
        extent = struct.unpack_from('<I',header,32)[0]+frames*10
        if extent != 3356 or ranges != 38:
            raise ValueError('Title common animation layout differs from audited input')
        result['animation'] = dict(member=0,offset=8192,bytes=extent,images=images,
                                   lists=lists,parts=parts,ranges=ranges,frames=frames,
                                   sha256=hashlib.sha256(member0.read(8192,extent)).hexdigest())
        bank = inspect_container_0d(MemberView(stream,member0.start+11548,member0.size-11548))
        result['images'] = dict(member=0,offset=11548,bytes=bank['byte_size'],blocks=bank['block_count'],
                                sha256=hashlib.sha256(member0.read(11548,bank['byte_size'])).hexdigest(),
                                validation=bank['validation'])
    if exe.read(0x80049296+26*2,2) != bytes([53,54]):
        raise ValueError('Title sound request 26 does not select bank 53 / sequence 54')
    result['sound'] = dict(request=26,bank=53,sequence=54,effects_bank=0,effects_ids=[0,5],
                          pairing_evidence='Overlay 800BA4C4 calls sound_request(26); sound_request.c maps its pair table')
    result['inputs'] = [file_identity(directory/name) for name in ('A_FILE.HDT','A_FILE.BIN','SLUS_011.15')]
    result['validation'] = 'Static source/data audit only; native compilation, tests and PS1 parity are separate gates'
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--data-dir',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args = parser.parse_args()
    try:
        report = audit(args.data_dir)
        args.output.parent.mkdir(parents=True,exist_ok=True)
        with args.output.open('x',encoding='utf-8') as stream:
            json.dump(report,stream,indent=2)
            stream.write('\n')
    except (OSError,ValueError,struct.error) as error:
        print(f'Title audit failed: {error}',file=sys.stderr)
        return 1
    print(f'Title audit: {report["span_bytes"]} overlay bytes, {report["images"]["blocks"]} image blocks, bank 53 / sequence 54')
    return 0


if __name__ == '__main__':
    sys.exit(main())
