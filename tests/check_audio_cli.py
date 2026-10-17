#!/usr/bin/env python3
"""Check audio CLI, independent SPU PCM, XA channel filtering, and local banks.

All inputs/exports live in a temporary directory; retail bytes never enter Git.
The desktop path uses SDL's dummy devices for a short queue/drain smoke test.
"""
import hashlib
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import wave

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from prepare_native_audio import read_catalog


def run(exe,*args,code=0,env=None):
    result=subprocess.run([str(exe),*map(str,args)],capture_output=True,timeout=60,env=env)
    if result.returncode!=code:
        raise AssertionError(f'{args}: exit {result.returncode}, expected {code}: '
                             +result.stderr.decode(errors='replace')[-1500:])
    return result


def fixture():
    b=bytearray(0xC40)
    b[:4]=b'pBAV'
    struct.pack_into('<I',b,4,7)
    struct.pack_into('<I',b,12,len(b))
    struct.pack_into('<3H',b,18,1,1,1)
    b[24:26]=bytes([127,64]);b[32:37]=bytes([1,127,0,0,64])
    b[0x820:0x828]=bytes([10,0,127,64,60,0,0,127])
    struct.pack_into('<3H',b,0x830,15,0x1FC0,0)
    struct.pack_into('<H',b,0x836,1)
    struct.pack_into('<H',b,0xA22,4)
    b[0xC20:0xC30]=bytes([1,4])+b'\x11'*14
    b[0xC30:0xC40]=bytes([1,3])+b'\x22'*14
    return b


def decode_spu(data):
    """Independent reference: integer recurrence before pitch/envelope/mixing."""
    old=older=0
    result=[]
    for i in range(0,len(data),16):
        block=data[i:i+16]
        shift,filter=block[0]&15,block[0]>>4
        if shift>12: shift=9
        positive,negative=[(0,0),(60,0),(115,-52),(98,-55),(122,-60)][filter]
        for byte in block[2:]:
            for n in (byte&15,byte>>4):
                if n>=8:n-=16
                value=max(-32768,min(32767,((n*4096)>>shift)+((old*positive+older*negative+32)>>6)))
                result.append(value);older,old=old,value
        if block[1]&1:break
    return result


def wav(path):
    with wave.open(str(path),'rb') as f:
        assert f.getnchannels()==2 and f.getsampwidth()==2
        rate,count=f.getframerate(),f.getnframes()
        return rate,count,struct.unpack('<'+'h'*(count*2),f.readframes(count))


def xa_sector(channel,left=1,right=2):
    b=bytearray(2352);b[:12]=b'\0'+b'\xff'*10+b'\0';b[15]=2
    b[16:24]=bytes([1,channel,0x64,1])*2
    for g in range(18):
        offset=24+g*128;b[offset:offset+16]=b'\x0c'*16
        b[offset+16:offset+128]=bytes([left|(right<<4)])*112
    return b


def check(exe,data_dir=None):
    with tempfile.TemporaryDirectory(prefix='btn-audio-') as temp:
        folder=Path(temp);bank=folder/'test.vab';bank.write_bytes(fixture())
        base=['--audio-test','--bank',bank,'--headless','--seconds','1','--volume','100']
        out=folder/'sample.wav';run(exe,*base,'--output',out)
        rate,count,pcm=wav(out);assert (rate,count)==(44100,44100)
        reference=decode_spu(fixture()[0xC20:])
        assert list(pcm[::2][:56])==reference and pcm[112:]==(0,)*(len(pcm)-112)
        old=out.read_bytes();run(exe,*base,'--output',out,code=1);assert out.read_bytes()==old
        run(exe,*base,'--output',bank,code=1);assert bank.read_bytes()==fixture()
        loop=folder/'loop.wav';run(exe,*base,'--loop','--output',loop);assert wav(loop)[2][112]==2048
        fade=folder/'fade.wav';run(exe,*base,'--loop','--fade-ms','1000','--output',fade)
        assert abs(wav(fade)[2][-2])<10
        for args in (['--sample','0'],['--volume','101'],['--seconds','0'],['--fade-ms','1001'],['--channel','1']):
            run(exe,*base,*args,code=2)
        malformed=folder/'bad.vab';malformed.write_bytes(fixture()[:-1])
        run(exe,'--audio-test','--bank',malformed,'--headless',code=1)
        seq=folder/'test.seq'
        seq.write_bytes(b'pQES\0\0\0\1\x01\xe0\x07\xa1\x20\x04\x02'
                        +bytes([0,0xC0,0,0,0x90,60,127,0x83,0x60,60,0,0,255,47,0]))
        music=folder/'music.wav';run(exe,'--audio-test','--bank',bank,'--sequence',seq,'--headless','--seconds','1','--output',music)
        assert max(wav(music)[2])>0
        xa=folder/'test.xa';xa.write_bytes(xa_sector(0,7,7)+xa_sector(3)+xa_sector(3,3,4))
        xa_base=['--audio-test','--xa',xa,'--sector-size','2352','--file','1','--channel','3','--headless','--seconds','1','--volume','100']
        xaout=folder/'xa.wav';run(exe,*xa_base,'--output',xaout)
        rate,count,pcm=wav(xaout);assert (rate,count)==(37800,37800)
        assert pcm[:4032]==(1,2)*2016 and pcm[4032:8064]==(3,4)*2016 and not any(pcm[8064:])
        run(exe,*xa_base,'--channel','4',code=1)
        xa.write_bytes(xa.read_bytes()[:-1]);run(exe,*xa_base,code=1)
        env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy',SDL_RENDER_DRIVER='software')
        run(exe,'--audio-test','--bank',bank,'--seconds','1',env=env)
        if data_dir:
            catalog,resources=read_catalog(Path(data_dir))
            banks=0;samples=0
            for record,payload in resources:
                path=folder/record['file'];path.write_bytes(payload)
                if record['kind']!='bank': continue
                banks+=1
                # Check every bank structurally and the first sample independently.
                out=folder/f'bank-{record["index"]}.wav'
                run(exe,'--audio-test','--bank',path,'--headless','--seconds','1','--volume','100','--output',out)
                reference=decode_spu(payload[record['header_bytes']:record['header_bytes']+record['sample_bytes'][0]])
                pcm=wav(out)[2][::2];n=min(len(reference),len(pcm))
                assert list(pcm[:n])==reference[:n],record['index']
                samples+=1
            # Caller explicitly selects the bank; no automatic adjacency pairing.
            run(exe,'--audio-test','--bank',folder/'bank-003.vab','--sequence',folder/'sequence-004.seq',
                '--headless','--seconds','2','--output',folder/'retail-sequence.wav')
            assert max(wav(folder/'retail-sequence.wav')[2])>0
            print(f'Local data: {banks} banks parsed, {samples} sample PCM prefixes independently compared; 1 sequence rendered')
    print('audio CLI checks passed')


if __name__=='__main__':
    check(Path(sys.argv[1]).resolve(),sys.argv[2] if len(sys.argv)>2 else None)
