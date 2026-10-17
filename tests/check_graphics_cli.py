#!/usr/bin/env python3
"""Exercise graphics CLI and independently compare decoded 0D preview pixels.

Retail data is optional and stays local. The Python decoder does not call the
native parser, use its output as a golden, or compare an image to itself.
"""
import hashlib
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile


def run(executable, *args, code=0, env=None):
    result = subprocess.run([str(executable), *map(str, args)], capture_output=True, timeout=40, env=env)
    if result.returncode != code:
        raise AssertionError(f'{args}: exit {result.returncode}, expected {code}: '
                             + result.stderr.decode(errors='replace')[-2000:])
    return result


def ppm(path):
    data = path.read_bytes()
    header = b'P6\n320 240\n255\n'
    assert data.startswith(header) and len(data) == len(header) + 320 * 240 * 3
    return data[len(header):]


def decode_bank(data):
    assert data[0] == 13
    count = struct.unpack_from('<H', data, 2)[0]
    cursor, blocks = 4, []
    vram = [0] * (1024 * 512)
    for _ in range(count):
        word, x, y, w, h = struct.unpack_from('<I4H', data, cursor)
        length, flags = word & 0xFFFFFF, word >> 24
        payload = data[cursor+12:cursor+12+length]
        assert len(payload) == length and w and h and x+w <= 1024 and y+h <= 512
        if flags & 12 == 4:
            decoded = payload[:w*h*2]
            assert len(decoded) == w*h*2
        else:
            decoded = bytearray()
            position = 0
            while len(decoded) < w*h*2:
                control = struct.unpack_from('<b', payload, position)[0]
                position += 1
                if control >= 0:
                    decoded.extend(payload[position:position+1] * max(control, 1))
                    position += 1
                else:
                    decoded.extend(payload[position:position-control])
                    position -= control
                assert position <= len(payload)
            decoded = decoded[:w*h*2]
        if flags & 8:
            rows = bytearray(w*h*2)
            for column in range(w):
                for byte in range(2):
                    for row in range(h):
                        rows[(row*w+column)*2+byte] = decoded[(column*2+byte)*h+row]
            decoded = rows
        words = struct.unpack('<' + 'H'*(w*h), decoded)
        for row in range(h):
            vram[(y+row)*1024+x:(y+row)*1024+x+w] = words[row*w:(row+1)*w]
        blocks.append((x, y, w, h, flags))
        cursor += 12 + ((length+3) & ~3)
    return vram, blocks


def rgb(word):
    return bytes(((word & 31)*255//31, ((word >> 5) & 31)*255//31, ((word >> 10) & 31)*255//31))


def reference(data, image, palette, depth=None):
    vram, blocks = decode_bank(data)
    x, y, w, h, flags = blocks[image]
    depth = depth or (15 if flags & 3 == 2 else 8 if flags & 1 else 4)
    scale = {4: 4, 8: 2, 15: 1}[depth]
    px, py, *_ = blocks[palette]
    background = rgb(3 | (4 << 5) | (6 << 10))
    result = bytearray(background * (320*240))
    for row in range(min(h, 200)):
        for column in range(min(w*scale, 312)):
            u = (((x & 63)*scale) + column) & 255
            v = ((y & 255) + row) & 255
            word = vram[((y//256)*256+v)*1024 + ((x//64)*64+u//scale) % 1024]
            if depth != 15:
                mask = (1 << depth)-1
                index = (word >> ((u % scale)*depth)) & mask
                word = vram[py*1024 + (px+index) % 1024]
            if word:
                offset = ((row+40)*320+column+8)*3
                result[offset:offset+3] = rgb(word)
    return bytes(result)


def reference_animation(data, bank_offset, range_index=0, frame_index=0):
    vram, _ = decode_bank(data[bank_offset:])
    sections = struct.unpack_from('<5I', data, 16)
    count, offset = struct.unpack_from('<2H', data, sections[3]+range_index*4)
    assert frame_index < count
    parts, fx, fy = struct.unpack_from('<H2h', data, sections[4]+offset+frame_index*10)
    framebuffer = [3 | (4 << 5) | (6 << 10)] * (320*240)
    if parts <= 0xFFFD:
        count, offset = struct.unpack_from('<2H', data, sections[1]+parts*4)
        for part in range(count):
            image, palette, dx, dy, attr = struct.unpack_from('<2H2hB', data, sections[2]+offset+part*12)
            ix, iy, width, height = struct.unpack_from('<4h', data, sections[0]+image*8)
            px, py = struct.unpack_from('<2h', data, sections[0]+palette*8)
            depth = 8 if attr & 128 else 4
            scale = 16 // depth
            width *= scale
            left, top = 160+fx+dx, 120+fy+dy
            for row in range(height):
                y = top+row
                if not 0 <= y < 240:
                    continue
                v = ((iy & 255)+(height-1-row if attr & 2 else row)) & 255
                for column in range(width):
                    x = left+column
                    if not 0 <= x < 320:
                        continue
                    u = ((ix & 63)*scale+(width-1-column if attr & 1 else column)) & 255
                    word = vram[((iy//256)*256+v)*1024+((ix//64)*64+u//scale) % 1024]
                    index = (word >> ((u % scale)*depth)) & ((1 << depth)-1)
                    color = vram[py*1024+(px+index) % 1024]
                    if not color:
                        continue
                    dest = y*320+x
                    if attr & 16 and color & 0x8000:
                        back = framebuffer[dest]
                        color = (color & 0x8000) | sum((((back >> shift) & 31)//2
                            + ((color >> shift) & 31)//2) << shift for shift in (0,5,10))
                    framebuffer[dest] = color
    return b''.join(map(rgb, framebuffer))


def fixture():
    colors = [0, 31, 31 << 5, 31 << 10] + [0x8000] * 12
    palette = struct.pack('<I4H16H', (6 << 24) | 32, 768, 500, 16, 1, *colors)
    pixels = bytes([0x21, 0x03, 0x10, 0x32] * 4)
    # Four words, two rows, RLE literal crosses the row boundary.
    payload = bytes([256-len(pixels)]) + pixels
    block = struct.pack('<I4H', len(payload), 640, 256, 4, 2) + payload
    block += b'\0' * (-len(payload) % 4)
    return b'\x0d\x10\x02\0' + palette + block


def reference_font(data):
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
    from text_codec import encode
    vram, _ = decode_bank(data)
    framebuffer = bytearray(rgb(3 | (4 << 5) | (6 << 10)) * (320*32))
    x, y = 8, 8
    for code in encode('BTN Graphics\nA B C 012{END}'):
        if code == 0xFFFF:
            break
        if code == 0xFFFE:
            x, y = 8, y+12
            continue
        if code != 0xFFD:
            for dy in range(12):
                for dx in range(8):
                    u, v = (code % 16)*8+dx, (code // 16)*12+dy
                    index = (vram[v*1024+896+u//4] >> ((u%4)*4)) & 15
                    color = vram[254*1024+896+index]
                    if color:
                        offset = ((y+dy)*320+x+dx)*3
                        framebuffer[offset:offset+3] = rgb(color)
        x += 8
    return bytes(framebuffer)


def write_archive(directory, member):
    (directory/'A_FILE.HDT').write_bytes(struct.pack('<33I', 0, *([len(member)]*32)))
    (directory/'A_FILE.BIN').write_bytes(member)


def preview(executable, directory, member, data, image, palette, output):
    result = run(executable, '--asset-viewer', '--data-dir', directory, '--member', member,
                 '--image', image, '--palette', palette, '--headless', '--output', output)
    actual, expected = ppm(output), reference(data, image, palette)
    assert actual == expected, f'member {member} image {image}: RGB differs from independent decoder'
    return hashlib.sha256(actual).hexdigest(), result


def check(executable, retail=None):
    with tempfile.TemporaryDirectory(prefix='btn-graphics-') as folder:
        root = Path(folder)
        if retail:
            offsets = struct.unpack('<33I', (retail/'A_FILE.HDT').read_bytes()[:132])
            compared = 0
            with (retail/'A_FILE.BIN').open('rb') as stream:
                for member, images in ((0, (1,3,5,7,9)), (1, (1,)), (7, (1,2))):
                    stream.seek(offsets[member])
                    data = stream.read(min(offsets[member+1]-offsets[member], 128*1024))
                    for image in images:
                        preview(executable,retail,member,data,image,0,root/f'{member}-{image}.ppm')
                        compared += 1
                animation_samples = ((14,11636),(18,3112),(20,4172),(22,9020),(24,10664),(26,7304))
                for member, bank_offset in animation_samples:
                    stream.seek(offsets[member])
                    data = stream.read(offsets[member+1]-offsets[member])
                    output = root/f'animation-{member}.ppm'
                    run(executable,'--asset-viewer','--data-dir',retail,'--member',member,
                        '--offset',bank_offset,'--animation-member',member,'--frame','0','--headless','--output',output)
                    assert ppm(output) == reference_animation(data,bank_offset), f'Animation {member} differs'
            font_image = root/'font.ppm'
            run(executable,'--graphics-test','--data-dir',retail,'--headless','--output',font_image)
            with (retail/'A_FILE.BIN').open('rb') as stream:
                stream.seek(offsets[0])
                font_data = stream.read(6904)
            assert ppm(font_image)[:320*32*3] == reference_font(font_data), 'English glyph atlas/CLUT differs'
            print(f'native-graphics-data: {compared} images and {len(animation_samples)} animation frames '
                  'match independent decode; PS1 comparison pending')
            return
        run(executable,'--help')
        for arguments in ((), ('--asset-viewer',), ('--graphics-test','--frames','0'),
                          ('--graphics-test','--depth','7'), ('--graphics-test','--animation-member','14'),
                          ('--graphics-test','--asset-viewer'), ('--graphics-test','--frames','-1')):
            run(executable,*arguments,code=2)
        first,second,animated = (root/name for name in ('first.ppm','second.ppm','animated.ppm'))
        run(executable,'--graphics-test','--headless','--output',first)
        run(executable,'--graphics-test','--headless','--output',second)
        assert ppm(first)==ppm(second), 'Fixed tick must reproduce exactly'
        run(executable,'--graphics-test','--headless','--tick','8','--output',animated)
        assert ppm(first)!=ppm(animated), 'Model rotation must affect the framebuffer'
        before=first.read_bytes()
        run(executable,'--graphics-test','--headless','--output',first,code=1)
        assert first.read_bytes()==before
        comparison = Path(__file__).resolve().parents[1]/'tools/compare_graphics.py'
        run(Path(sys.executable),comparison,'--reference',first,'--actual',second,
            '--report',root/'same.json','--diff',root/'same.ppm','--scene','synthetic/no-input','--tick','0')
        run(Path(sys.executable),comparison,'--reference',first,'--actual',animated,
            '--report',root/'different.json','--diff',root/'different.ppm','--scene','synthetic/no-input','--tick','8',code=1)
        data=fixture(); write_archive(root,data)
        preview(executable,root,0,data,1,0,root/'fixture.ppm')
        run(executable,'--asset-viewer','--data-dir',root,'--image','10','--headless',code=1)
        run(executable,'--asset-viewer','--data-dir',root,'--image','1','--palette','1','--headless',code=1)
        for cut in (0,3,15,len(data)-1):
            write_archive(root,data[:cut])
            run(executable,'--asset-viewer','--data-dir',root,'--headless',code=1)
        write_archive(root,data)
        env=os.environ.copy(); env.update(SDL_VIDEODRIVER='dummy',SDL_RENDER_DRIVER='software')
        run(executable,'--graphics-test','--frames','2',env=env)
        print('native-graphics-cli: passed (synthetic; supplied-data and PS1 parity are separate checks)')


if __name__ == '__main__':
    check(Path(sys.argv[1]).resolve(),Path(sys.argv[2]).resolve() if len(sys.argv)>2 else None)
