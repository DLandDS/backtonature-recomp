"""Exercise native title input/lifecycle and compare raw sprites independently.

Synthetic inputs ship as generators. Optional retail bytes/captures stay local.
The pixel oracle checks the supported static title layers, not PS1 parity.
"""
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

from check_audio_cli import fixture as audio_fixture
from check_graphics_cli import fixture as image_fixture, decode_bank, rgb


def run(executable, *args, code=0):
    env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_RENDER_DRIVER='software', SDL_AUDIODRIVER='dummy')
    result = subprocess.run([str(executable), *map(str, args)], capture_output=True,
                            timeout=45, env=env)
    output = (result.stdout + result.stderr).decode(errors='replace')
    assert result.returncode == code, f'{args}: exit {result.returncode}, expected {code}: {output[-1500:]}'
    return output


def fixture(folder):
    animation = bytearray(3356)
    struct.pack_into('<I4HI5I', animation, 0, 14, 2, 1, 1, 38, 1, 36, 52, 56, 68, 3346)
    struct.pack_into('<8h', animation, 36, 640, 256, 4, 2, 768, 500, 16, 1)
    struct.pack_into('<2H', animation, 52, 1, 0)
    struct.pack_into('<2H2h4B', animation, 56, 0, 1, 0, 0, 0, 128, 128, 128)
    for i in range(38):
        struct.pack_into('<2H', animation, 68+i*4, 1, 0)
    struct.pack_into('<H2h4B', animation, 3346, 0, 0, 0, 1, 255, 0, 0)
    images = image_fixture()
    member0 = b'\0'*8192 + animation + images
    bank = bytes(audio_fixture())
    seq = b'pQES\0\0\0\1\x01\xe0\x07\xa1\x20\x04\x02' + bytes([
        0, 0xC0, 0, 0, 0x90, 60, 127, 0x83, 0x60, 60, 0, 0, 0xFF, 0x2F, 0])
    overlay = bytearray(0xE0)
    struct.pack_into('<I', overlay, 0, 12)
    struct.pack_into('<5I', overlay, 0xCC, 0x27BDFFE8, 0, 0, 0, 0x248449F8)
    members = [b'']*32
    members[0], members[9], members[27] = member0, bank+bank+seq, overlay
    offsets = [0]
    for member in members:
        offsets.append(offsets[-1]+len(member))
    (folder/'A_FILE.HDT').write_bytes(struct.pack('<33I', *offsets))
    (folder/'A_FILE.BIN').write_bytes(b''.join(members))
    exe = bytearray(0x3B800)
    exe[:8] = b'PS-X EXE'
    struct.pack_into('<II', exe, 24, 0x80010000, len(exe)-2048)

    def write(address, content):
        offset = 2048+address-0x80010000
        exe[offset:offset+len(content)] = content

    write(0x80049296+52, bytes([53,54]))
    write(0x80049370, struct.pack('<3I', 0, 0xC20, len(bank)))
    write(0x80049370+53*8, struct.pack('<5I', len(bank), len(bank)+0xC20,
                                    len(bank)*2, 1, len(bank)*2+len(seq)))
    for sound in (0, 5):
        write(0x800495CC+sound*14, struct.pack('<7H', 0,0,0,0,60,0,127))
    (folder/'SLUS_011.15').write_bytes(exe)


def bmp(path):
    data = path.read_bytes()
    assert data[:2] == b'BM'
    start, = struct.unpack_from('<I', data, 10)
    width, height, planes, depth, compression = struct.unpack_from('<iiHHI', data, 18)
    assert width == 320 and abs(height) == 240 and planes == 1 and depth in (24,32)
    assert compression in (0,3)
    if compression == 3:
        assert depth == 32 and struct.unpack_from('<3I', data, 54) == (0xFF0000,0xFF00,0xFF)
    stride = ((width*depth+31)//32)*4
    assert len(data) >= start+stride*240
    result = bytearray()
    for y in range(240):
        row = y if height < 0 else 239-y
        for x in range(320):
            offset = start+row*stride+x*(depth//8)
            result += data[offset:offset+3][::-1]
    return bytes(result)


def reference(folder, *, menu=False, tick=1, selection=1):
    offsets = struct.unpack_from('<33I', (folder/'A_FILE.HDT').read_bytes())
    with (folder/'A_FILE.BIN').open('rb') as stream:
        stream.seek(offsets[0]+8192)
        animation = stream.read(3356)
        # The bounded first bank is about 140 KiB in the audited input.
        bank = stream.read(min(1024*1024,offsets[1]-offsets[0]-11548))
    vram, _ = decode_bank(bank)
    sections = struct.unpack_from('<5I', animation, 16)
    frame = [14 | (22<<5) | (29<<10)]*(320*240)
    layers = [(31,0,0),(32,0,0)]
    layers += [(34,0,0),(35,0,0),(36,100 if tick%30 < 15 else 104,179 if selection else 155)] if menu else [(33,0,0)]
    for index, ax, ay in layers:
        _, offset = struct.unpack_from('<2H', animation, sections[3]+index*4)
        parts, fx, fy = struct.unpack_from('<H2h', animation, sections[4]+offset)
        count, offset = struct.unpack_from('<2H', animation, sections[1]+parts*4)
        for n in range(count):
            image, palette, dx, dy, attr = struct.unpack_from('<2H2hB', animation, sections[2]+offset+n*12)
            ix, iy, w, h = struct.unpack_from('<4h', animation, sections[0]+image*8)
            px, py = struct.unpack_from('<2h', animation, sections[0]+palette*8)
            depth = 8 if attr & 128 else 4
            scale = 16//depth
            w *= scale
            for row in range(h):
                y = ay+fy+dy+row
                if not 0 <= y < 240:
                    continue
                v = ((iy&255)+(h-1-row if attr&2 else row))&255
                for col in range(w):
                    x = ax+fx+dx+col
                    if not 0 <= x < 320:
                        continue
                    u = ((ix&63)*scale+(w-1-col if attr&1 else col))&255
                    word = vram[(iy//256*256+v)*1024+ix//64*64+u//scale]
                    color = vram[py*1024+px+((word>>((u%scale)*depth))&((1<<depth)-1))]
                    if not color:
                        continue
                    assert not (attr&16 and color&0x8000), 'Unsupported oracle blending in static title layers'
                    frame[y*320+x] = color
    return b''.join(map(rgb, frame))


def check(executable, retail=None):
    with tempfile.TemporaryDirectory(prefix='btn-title-') as temporary:
        root = Path(temporary)
        fixture(root)
        source = Path(retail).resolve() if retail else root
        common = ['--title','--data-dir',source,'--headless']
        for args in (['--title'], ['--title','--runtime-test','--data-dir',source],
                     ['--runtime-test','--headless'], ['--title','--data-dir',source,'--output',root/'bad.bmp']):
            run(executable,*args,code=2)
        capture = root/'prompt.bmp'
        run(executable,*common,'--frames','2','--output',capture)
        assert bmp(capture) == reference(source)
        before = capture.read_bytes()
        run(executable,*common,'--output',capture,code=1)
        assert capture.read_bytes() == before
        events = [(0,64),(1,0),(2,1),(3,1),(4,0),(5,16),(6,0),(7,32),(8,0),
                  (9,2),(10,0),(11,16),(12,0),(13,32),(14,0),(15,32),(16,0),(17,64),(18,0)]
        script = root/'menu.input'
        script.write_bytes(b''.join(struct.pack('<II',*event) for event in events))
        capture = root/'menu.bmp'
        output = run(executable,*common,'--frames','20','--input-script',script,'--output',capture)
        assert 'New Game unavailable' in output and 'save/load requires phase 07' in output
        assert 'title stopped: ticks=20 page=1 selection=1' in output
        assert 'tick=3 page=' not in output, 'Held direction must not repeat menu selection'
        assert bmp(capture) == reference(source,menu=True,tick=20)
        for invalid in (b'',b'bad',struct.pack('<II',1,0),struct.pack('<II',0,128),
                        struct.pack('<4I',0,0,0,0),struct.pack('<4I',0,0,10,0)):
            script.write_bytes(invalid)
            assert 'Replay requires' in run(executable,*common,'--frames','10','--input-script',script,code=1)
        # Desktop/dummy device lifecycle, including pause/queue startup and menu presentation.
        run(executable,'--title','--data-dir',source,'--frames','3')
        if not retail:
            exe = root/'SLUS_011.15'
            original = exe.read_bytes()
            exe.write_bytes(b'invalid')
            assert 'incompatible' in run(executable,*common,code=1)
            exe.write_bytes(original)
            archive = root/'A_FILE.BIN'
            data = archive.read_bytes()
            archive.write_bytes(data[:-1])
            run(executable,*common,code=1)
    print('native-title: CLI/replay bounds, native menu, unsupported destinations, independent sprite pixels and SDL lifecycle passed'
          + (' (local retail resources; PS1 parity not established)' if retail else ' (synthetic fixtures)'))


if __name__ == '__main__':
    check(Path(sys.argv[1]).resolve(),Path(sys.argv[2]) if len(sys.argv)>2 else None)
