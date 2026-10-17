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
    palette = struct.pack('<I4H16H', (6<<24)|32, 896,254,16,1,*([0]+[32767]*15))
    dark_palette = struct.pack('<I4H16H', (6<<24)|32, 896,252,16,1,*([0]+[0x0C41]*15))
    glyph_pixels = bytearray(32*120*2)
    for y in range(120):
        for x in range(128):
            if (x//8+y//12+x+y)%3:
                glyph_pixels[y*64+x//2] |= 1 << ((x%2)*4)
    glyph_bank = b'\x0d\0\x03\0'+palette+dark_palette+struct.pack('<I4H',(4<<24)|len(glyph_pixels),896,0,32,120)+glyph_pixels
    member0 = glyph_bank+b'\0'*(8192-len(glyph_bank)) + animation + images
    ui = bytearray(5084)
    struct.pack_into('<I4HI5I',ui,0,14,4,2,2,44,6,36,68,76,100,5024)
    ui[36:52] = animation[36:52]
    struct.pack_into('<4h',ui,52,784,500,16,1)
    struct.pack_into('<4h',ui,60,640,256,2,2)
    struct.pack_into('<4H',ui,68,1,0,1,12)
    struct.pack_into('<2H2h4B',ui,76,0,2,-13,-1,0,128,128,128)
    # Deliberately overlap differently coloured hand/season pixels on the calendar.
    struct.pack_into('<2H2h4B',ui,88,3,1,1,5,0,128,128,128)
    for index in range(44):
        struct.pack_into('<2H',ui,100+index*4,4 if index==43 else 2 if index==11 else 1,20 if index==43 else 0)
    # Default dialog wait uses range 7, not the heart in range 1. Give them
    # different fixture parts so a wrong range fails the independent oracle.
    struct.pack_into('<2H',ui,100+7*4,2,20)
    for index in range(6):
        struct.pack_into('<H2h4B',ui,5024+index*10,0 if index<2 else 1,0,0,1,255,0,0)
    ui_images = bytearray(images)
    struct.pack_into('<H',ui_images,2,struct.unpack_from('<H',images,2)[0]+1)
    ui_images += struct.pack('<I4H16H',(6<<24)|32,784,500,16,1,*([0]+[0x4210]*15))
    member0 += b'\0'*(151552-len(member0))+ui+ui_images
    bank = bytes(audio_fixture())
    seq = b'pQES\0\0\0\1\x01\xe0\x07\xa1\x20\x04\x02' + bytes([
        0, 0xC0, 0, 0, 0x90, 60, 127, 0x83, 0x60, 60, 0, 0, 0xFF, 0x2F, 0])
    overlay = bytearray(0xE0)
    struct.pack_into('<I', overlay, 0, 12)
    struct.pack_into('<5I', overlay, 0xCC, 0x27BDFFE8, 0, 0, 0, 0x248449F8)
    members = [b'']*32
    members[0], members[9], members[27] = member0, bank+bank+seq, overlay
    prompts = bytearray(0x3AB*8192)
    for index in (0x3A6,0x3A7):
        struct.pack_into('<I',prompts,index*8192,0xFFFFFFFF)
        struct.pack_into('<6H',prompts,index*8192+6144,0xFF3,0,1,2,3,0xFFFF)
    struct.pack_into('<I',prompts,0x3AA*8192,0xFFFFFFFF)
    struct.pack_into('<10H',prompts,0x3AA*8192+6144,0xFFA,0xFFFE,0xFFD,6,7,0xFFFE,0xFFD,8,9,0xFFFF)
    # A small 8bpp portrait with a transparent border, compressed to fit a slot.
    pixels = bytes(0 if x<4 or x>=76 or y<4 or y>=76 else 1 for y in range(80) for x in range(80))
    packed = bytearray(); pos=0
    while pos<len(pixels):
        end=pos+1
        while end<len(pixels) and pixels[end]==pixels[pos] and end-pos<127: end+=1
        packed += bytes([end-pos,pixels[pos]]); pos=end
    portrait = b'\x0d\0\x02\0'+struct.pack('<I4H256H',(6<<24)|512,0,490,256,1,*([0]+[0x7C1F]*255))
    portrait += struct.pack('<I4H',(1<<24)|len(packed),960,175,40,80)+packed+bytes((-len(packed))%4)
    layouts = (((26,16),(28,30,5)),((18,),),((22,),),((29,),),((27,12),))
    for index,(slot,pages) in enumerate(zip((0x3A2,0x397,0x3A5,0x3A4,0x3A3),layouts)):
        words=[]
        for page,lines in enumerate(pages):
            if page: words += [0xFFC,0xFFB,0xFFFE]
            words += [0xF87,0xFF7,0xFF4]
            for row,length in enumerate(lines):
                if row: words.append(0xFFFE)
                words += [(index+row+column)%26 for column in range(length)]
        words.append(0xFFFF)
        struct.pack_into('<I',prompts,slot*8192,104)
        prompts[slot*8192+4:slot*8192+4+len(portrait)] = portrait
        struct.pack_into('<'+'H'*len(words),prompts,slot*8192+6144,*words)
    members[5] = prompts
    offsets = [0]
    for member in members:
        offsets.append(offsets[-1]+len(member))
    (folder/'A_FILE.HDT').write_bytes(struct.pack('<33I', *offsets))
    (folder/'A_FILE.BIN').write_bytes(b''.join(members))
    exe = bytearray(0x50000)
    exe[:8] = b'PS-X EXE'
    struct.pack_into('<II', exe, 24, 0x80010000, len(exe)-2048)

    def write(address, content):
        offset = 2048+address-0x80010000
        exe[offset:offset+len(content)] = content

    write(0x80049296+52, bytes([53,54]))
    write(0x80049370, struct.pack('<3I', 0, 0xC20, len(bank)))
    write(0x80049370+53*8, struct.pack('<5I', len(bank), len(bank)+0xC20,
                                    len(bank)*2, 1, len(bank)*2+len(seq)))
    for sound in range(6):
        write(0x800495CC+sound*14, struct.pack('<7H', 0,0,0,0,60,0,127))
    write(0x8005E364,bytes([13,13,12,10,9,13]))
    write(0x8005E36C,bytes([13,13,7,0,0,13]))
    write(0x8005599C,bytes([5,4,4,4,4,5,4,4,4,4,3,2,2,5]))
    write(0x800559AC,bytes([2,2,2,2,2,2,2,2,1,1,0,0,0,5]))
    write(0x800559D4,struct.pack('<28H',*range(0x60,0x60+28)))
    write(0x80049044+6,struct.pack('<H',252))
    write(0x80049052,struct.pack('<H',3))
    write(0x80058264,struct.pack('<6I',512,300,160,80,40,20))
    for address,base in ((0x800537B2,0),(0x80053888,4)):
        write(address,struct.pack('<7H',base,base+1,0xFFD,base+2,0xFFFE,base+3,0xFFFF))
    write(0x8005175C,struct.pack('<9H',0xFFD,0,1,2,0xFFFE,0xFFD,3,4,0xFFFF))
    write(0x8005597C+6,struct.pack('<H',5))
    for index,address in enumerate((0x80051A94,0x80051AAA,0x80051AC8,0x80051AE0)):
        write(address,struct.pack('<4H',index,0xFFD,index+4,0xFFFF))
    write(0x80053946,struct.pack('<14H',*range(4,12),0x66,0xFFD,0xFFD,0xFFD,0x71,0xFFFF))
    summary=[1,0xFFD,0x400A,0xFFFE,2,0xFFFE,3,0xFFD,0x400D,0xFFFE,4,0xFFD,0x400C,0xFFFE,0xFFFE,5,0xFFFF]
    write(0x80051AF6,struct.pack('<'+'H'*len(summary),*summary))
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


def sdk_wave(folder, angle, sine):
    steps=exe_words(folder,0x80058264,12)[::2]
    angle %= 4096
    quadrant=angle//1024
    z=(angle,2047-angle,angle-2048,4095-angle)[quadrant]
    x,y=2487,0
    for shift,step in enumerate(steps):
        dx,dy=y>>shift,x>>shift
        if z>=0: x,y,z=x-dx,y+dy,z-step
        else: x,y,z=x+dx,y-dy,z+step
    value=y+((z*x)>>12) if sine else x-((z*y)>>12)
    return -value if (quadrant>=2 if sine else quadrant in (1,2)) else value


def reference(folder, *, menu=False, tick=1, selection=1, intro=False):
    offsets = struct.unpack_from('<33I', (folder/'A_FILE.HDT').read_bytes())
    with (folder/'A_FILE.BIN').open('rb') as stream:
        stream.seek(offsets[0])
        font = stream.read(8192)
        stream.seek(offsets[0]+8192)
        animation = stream.read(3356)
        # The bounded first bank is about 140 KiB in the audited input.
        bank = stream.read(min(1024*1024,offsets[1]-offsets[0]-11548))
    vram, _ = decode_bank(bank)
    font_vram, blocks = decode_bank(font)
    for x,y,w,h,_ in blocks:
        for row in range(y,y+h):
            vram[row*1024+x:row*1024+x+w] = font_vram[row*1024+x:row*1024+x+w]
    sections = struct.unpack_from('<5I', animation, 16)
    frame = [0]*(320*240)
    states = [[index,0,0] for index in (0,10,19,22,24,26)]
    ages = [0]*6
    draws = []
    raw_exe = (folder/'SLUS_011.15').read_bytes()
    load, = struct.unpack_from('<I',raw_exe,24)
    seed, = struct.unpack_from('<I',raw_exe,2048+0x8005E290-load)

    def random(n):
        nonlocal seed
        seed = (seed*0x41C64E6D+0x3039)&0xFFFFFFFF
        return ((seed>>16)*n)>>16

    def advance(cursor):
        index, pos, elapsed = cursor
        count, offset = struct.unpack_from('<2H',animation,sections[3]+index*4)
        duration, control = struct.unpack_from('<2B',animation,sections[4]+offset+pos*10+6)
        elapsed += 1
        limit = (duration+1)//2
        if elapsed >= limit:
            elapsed = 0
            pos += 1
            if pos >= count and not control&128:
                pos = control&127
            if control == 255:
                elapsed = limit
                pos -= 1
            elif control&128:
                pos = control&127
        cursor[:] = index,pos,elapsed

    for moment in range(1,tick+1):
        random(65535)
        draws = []
        for actor in range(6):
            for _ in range((1,1,1,3,5,3)[actor]):
                draws.append(tuple(states[actor]))
                advance(states[actor])
            ages[actor] += 1
            if ages[actor] >= (90,60,60,15,20,30)[actor]:
                ages[actor] = 0
                if actor < 2:
                    chosen = random(9 if actor else 10)+(10 if actor else 0)
                    if chosen%2 == 0:
                        chosen -= 1
                    chosen = max(10 if actor else 0,chosen)
                elif actor == 2:
                    chosen = 19+random(3)
                    if chosen == 19:
                        ages[actor] = 30
                else:
                    chosen = 22+(actor-3)*2+random(2)
                states[actor] = [chosen,0,0]
    # Reverse head insertion within the tile bucket; base and actors use
    # separate ordering-table buckets, as in the original overlay.
    tiles = []
    phase = [(tick-1)//2%960,(tick-1)%960]
    for layer in range(2):
        y = phase[layer]%240*(1 if layer else -1)
        angle = phase[1]*4096//120
        trig = sdk_wave(folder,angle,bool(layer))
        horizontal = 320-phase[layer]//240*80+int(trig*12/4096)
        xs = [y-horizontal,y+horizontal-320 if layer else y-horizontal+320,
              y+horizontal-640 if layer else y-horizontal+640]
        for row in range(3):
            for col in range(3):
                tiles.append((37,0,xs[col],y+row*240))
    layers = [(29,0,0,0)]+list(reversed(tiles))
    positions = [(282,173),(33,146),(51,182),(295,205),(258,138),(280,121),
                 (63,149),(10,188),(9,214),(295,52),(294,30),(23,30),(6,109),(36,223)]
    layers += [(cursor[0],cursor[1],*xy) for cursor,xy in reversed(list(zip(draws,positions)))]
    if not intro:
        layers += [(32,0,0,0),(31,0,0,0)]
        layers += [(35,0,0,0),(34,0,0,0),(36,0,100 if tick%30<15 else 104,179 if selection else 155)] if menu else [(33,0,0,0)]
    for index, index_frame, ax, ay in layers:
        _, offset = struct.unpack_from('<2H', animation, sections[3]+index*4)
        parts, fx, fy = struct.unpack_from('<H2h', animation, sections[4]+offset+index_frame*10)
        if parts > 0xFFFD:
            continue
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
                    if attr&16 and color&0x8000:
                        back = frame[y*320+x]
                        color = (color&0x8000)|sum((((back>>shift)&31)//2+((color>>shift)&31)//2)<<shift for shift in (0,5,10))
                    frame[y*320+x] = color
    if intro:
        assert tick<30, 'This oracle snapshot covers scenery before the logo enters'
        subtract = min(255,((0xFF000//10)*max(0,9-tick))//4096)>>3
        frame = [sum(max(0,((c>>shift)&31)-subtract)<<shift for shift in (0,5,10)) for c in frame]
    return b''.join(map(rgb, frame))


def exe_words(folder, address, count):
    exe = (folder/'SLUS_011.15').read_bytes()
    load, = struct.unpack_from('<I',exe,24)
    return struct.unpack_from(f'<{count}H',exe,2048+address-load)


def text_pixels(folder, words, x, y, spacing_x=0, spacing_y=0):
    """Decode the original glyph/CLUT data independently of native rendering."""
    offsets = struct.unpack('<33I', (folder/'A_FILE.HDT').read_bytes())
    with (folder/'A_FILE.BIN').open('rb') as stream:
        stream.seek(offsets[0])
        vram, _ = decode_bank(stream.read(8192))
    clut_y, = exe_words(folder,0x80049044+6,1)
    origin,result = x,{}
    for code in words:
        if code == 0xFFFF:
            break
        if code == 0xFFFE:
            x,y = origin,y+12+spacing_y
            continue
        if code != 0xFFD:
            assert code <= 255
            for dy in range(12):
                for dx in range(8):
                    u,v = (code & 15)*8+dx, (code >> 4)*12+dy
                    packed = vram[v*1024+896+u//4]
                    index = (packed >> ((u%4)*4)) & 15
                    color = vram[clut_y*1024+896+index]
                    if color:
                        result[x+dx,y+dy] = rgb(color)
        x += 8+spacing_x
    return result


def ui_pixels(folder, index, frame_index, ax, ay):
    offsets = struct.unpack('<33I',(folder/'A_FILE.HDT').read_bytes())
    with (folder/'A_FILE.BIN').open('rb') as stream:
        stream.seek(offsets[0]+151552)
        animation = stream.read(5084)
        vram,_ = decode_bank(stream.read(min(1024*1024,offsets[1]-offsets[0]-156636)))
    sections = struct.unpack_from('<5I',animation,16)
    count,offset = struct.unpack_from('<2H',animation,sections[3]+index*4)
    assert frame_index < count
    parts,fx,fy = struct.unpack_from('<H2h',animation,sections[4]+offset+frame_index*10)
    count,offset = struct.unpack_from('<2H',animation,sections[1]+parts*4)
    result = {}
    for part in range(count):
        image,palette,dx,dy,attr = struct.unpack_from('<2H2hB',animation,sections[2]+offset+part*12)
        ix,iy,w,h = struct.unpack_from('<4h',animation,sections[0]+image*8)
        px,py = struct.unpack_from('<2h',animation,sections[0]+palette*8)
        assert not attr&16, 'Selected UI icon oracle expects opaque sprite parts'
        depth = 8 if attr&128 else 4
        scale = 16//depth
        w *= scale
        for row in range(h):
            for col in range(w):
                u = ((ix&63)*scale+(w-1-col if attr&1 else col))&255
                v = ((iy&255)+(h-1-row if attr&2 else row))&255
                packed = vram[(iy//256*256+v)*1024+ix//64*64+u//scale]
                color = vram[py*1024+px+((packed>>((u%scale)*depth))&((1<<depth)-1))]
                if color:
                    result[ax+fx+dx+col,ay+fy+dy+row] = rgb(color)
    return result


def check_pixels(capture, expected):
    frame = bmp(capture)
    assert expected
    for (x,y),color in expected.items():
        assert frame[(y*320+x)*3:(y*320+x+1)*3] == color, (capture.name,x,y)


def check_name_pixels(folder, capture, shifted=False):
    address = 0x80053888 if shifted else 0x800537B2
    expected = text_pixels(folder,exe_words(folder,address,128),47,66,1,2)
    expected.update(ui_pixels(folder,11,1,203 if shifted else 50,72))
    check_pixels(capture,expected)
    frame = bmp(capture)
    def pixel(x,y):
        return frame[(y*320+x)*3:(y*320+x+1)*3]
    # Source window_frame_draw: upper/left highlight, lower/right shadow.
    assert pixel(100,64) == rgb(31 | (29<<5) | (26<<10))
    assert pixel(45,100) == rgb(31 | (29<<5) | (26<<10))
    assert pixel(274,100) == rgb(23 | (17<<5) | (8<<10))
    assert pixel(100,149) == rgb(23 | (17<<5) | (8<<10))
    assert pixel(272,79) != pixel(272,147), 'Window fill must retain its vertical gradient'
    check_question_pixels(folder,capture,0)


def check_question_pixels(folder, capture, question):
    address,capacity = ((0x80051A94,11),(0x80051AAA,15),(0x80051AC8,12),(0x80051AE0,11))[question]
    check_pixels(capture,text_pixels(folder,exe_words(folder,address,capacity),20,20))
    raw = bmp(capture)
    for x,y in ((50,16),(16,24)):
        assert raw[(y*320+x)*3:(y*320+x+1)*3] == rgb(31 | (29<<5) | (26<<10)), 'Question frame missing'


def number_words(value):
    return [0xFFD if value<10 else 0x40+value//10,0x40+value%10,0xFFFF]


def check_number_padding(folder, capture, x, y, blank_x):
    frame=bmp(capture)
    for px,py in text_pixels(folder,[0x40,0xFFFF],x,y):
        assert frame[(py*320+px)*3:(py*320+px+1)*3] == frame[(py*320+blank_x)*3:(py*320+blank_x+1)*3], 'Leading zero must be a space'


def check_review_pixels(folder, capture, choice=None):
    template=exe_words(folder,0x80051AF6,67); words=[]
    for word in template:
        if word in (0x400A,0x400C,0x400D): words.append(0)  # Replay names are one glyph A.
        else: words.append(word)
        if word==0xFFFF: break
    expected=text_pixels(folder,words,76,60,0,1)
    expected.update(text_pixels(folder,[0x71]+number_words(1),188,72))
    expected.update(ui_pixels(folder,43,0,172,72))
    if choice is not None:
        offsets=struct.unpack('<33I',(folder/'A_FILE.HDT').read_bytes())
        member,=exe_words(folder,0x8005597C+6,1)
        with (folder/'A_FILE.BIN').open('rb') as stream:
            stream.seek(offsets[member]+0x3AA*8192+6144); raw=stream.read(128)
        options=struct.unpack('<64H',raw)
        assert options[:2]==(0xFFA,0xFFFE)
        expected.update(text_pixels(folder,options[2:],30,168,0,2))
        expected.update(ui_pixels(folder,11,1,36,174+choice*14))
    check_pixels(capture,expected)
    check_number_padding(folder,capture,196,72,236)
    frame=bmp(capture)
    for x,y in ((100,56),(72,100)):
        assert frame[(y*320+x)*3:(y*320+x+1)*3] == rgb(31 | (29<<5) | (26<<10)), 'Original summary frame missing'


def check_speech_pixels(folder, capture, message, page=0, shown=None):
    offsets = struct.unpack('<33I',(folder/'A_FILE.HDT').read_bytes())
    member, = exe_words(folder,0x8005597C+6,1)
    slot = (0x3A2,0x397,0x3A5,0x3A4,0x3A3)[message]
    with (folder/'A_FILE.BIN').open('rb') as stream:
        stream.seek(offsets[member]+slot*8192); data=stream.read(8192)
    words=list(struct.unpack('<1024H',data[6144:])); words=words[:words.index(0xFFFF)+1]
    if page: words=words[words.index(0xFFB)+2:]
    end = words.index(0xFFC) if 0xFFC in words else words.index(0xFFFF)
    selected = words[:end] if shown is None else words[:shown]
    visible=[w for w in selected if w<=255 or w in (0xFFD,0xFFFE)]+[0xFFFF]
    expected=text_pixels(folder,visible,30,168,0,2)
    vram,_=decode_bank(data[4:6144])
    for y in range(80):
        for x in range(80):
            packed=vram[(175+y)*1024+960+x//2]
            color=vram[490*1024+((packed>>((x%2)*8))&255)]
            if color: expected[26+x,84+y]=rgb(color)
    check_pixels(capture,expected)
    if shown is None:
        check_wait_pixels(folder,capture)
    if shown is not None:
        full=text_pixels(folder,[w for w in words[:end] if w<=255 or w in (0xFFD,0xFFFE)]+[0xFFFF],30,168,0,2)
        frame=bmp(capture)
        assert any((x,y) not in expected and frame[(y*320+x)*3:(y*320+x+1)*3]!=color
                   for (x,y),color in full.items()), 'Speech must remain partially revealed'


def check_wait_pixels(folder, capture):
    offsets = struct.unpack('<33I',(folder/'A_FILE.HDT').read_bytes())
    with (folder/'A_FILE.BIN').open('rb') as stream:
        stream.seek(offsets[0]+151552); animation=stream.read(5084)
    ranges, = struct.unpack_from('<I',animation,28)
    count, = struct.unpack_from('<H',animation,ranges+7*4)
    frame=bmp(capture)
    for index in range(count):
        expected=ui_pixels(folder,7,index,285,217)
        if expected and all(frame[(y*320+x)*3:(y*320+x+1)*3]==color for (x,y),color in expected.items()):
            return
    raise AssertionError('Default dialog wait indicator must use UI range 7')


def setup_replay():
    events = [(0,0),(10,64),(11,0),(32,64),(33,0),(34,1),(35,1),(36,0),(37,16),(38,0)]
    # Early presses during opening must be discarded. Later presses wait for
    # each source-derived close/open cycle; season->day shares the help dialog.
    for tick,mask in [(59,16),(61,64),(63,16),(65,2),
                      (430,16),(432,64),(470,16),(600,16),(606,16),(644,16),
                      (780,16),(782,64),(820,16),(970,16),(972,64),(1010,16),(1050,16)]:
        events += [(tick,mask),(tick+1,0)]
    # Hold to speed up text, release, then press at the page/end wait. No held
    # press may advance both the dialog and the following setup control.
    events += [(80,16),(180,0),(182,16),(282,0),(284,16),(285,0),
               (314,16),(370,0),(372,16),(373,0),
               (500,16),(550,0),(552,16),(553,0),
               (674,16),(730,0),(732,16),(733,0),
               (850,16),(920,0),(922,16),(923,0)]
    return sorted(events)


def check(executable, retail=None):
    with tempfile.TemporaryDirectory(prefix='btn-title-') as temporary:
        root = Path(temporary)
        fixture(root)
        hand = ui_pixels(root,11,1,36,66)
        season = ui_pixels(root,43,0,30,60)
        assert any(xy in season and color!=season[xy] for xy,color in hand.items()), 'Fixture must detect cursor/season ordering'
        source = Path(retail).resolve() if retail else root
        common = ['--title','--data-dir',source,'--headless']
        for args in (['--title'], ['--title','--runtime-test','--data-dir',source],
                     ['--runtime-test','--headless'], ['--title','--data-dir',source,'--output',root/'bad.bmp']):
            run(executable,*args,code=2)
        capture = root/'prompt.bmp'
        run(executable,*common,'--frames','2','--output',capture)
        assert bmp(capture) == reference(source,intro=True,tick=2)
        before = capture.read_bytes()
        run(executable,*common,'--output',capture,code=1)
        assert capture.read_bytes() == before
        wave_capture=root/'wave.bmp'
        run(executable,*common,'--frames','12','--output',wave_capture)
        assert bmp(wave_capture)==reference(source,intro=True,tick=12), 'SDK integer scenery wave differs'
        events = [(0,0),(10,64),(11,0),(32,64),(33,0),(34,16),(35,0),
                  (36,32),(37,0),(38,32),(39,0),(40,64),(41,0)]
        script = root/'menu.input'
        script.write_bytes(b''.join(struct.pack('<II',*event) for event in events))
        capture = root/'menu.bmp'
        output = run(executable,*common,'--frames','43','--input-script',script,'--output',capture)
        assert 'save/load requires phase 07' in output
        assert 'title stopped: ticks=43 page=1 selection=1' in output
        assert bmp(capture) == reference(source,menu=True,tick=43)
        # Original setup order: player, birthday, farm, dog, review, then a
        # typed handoff. Gameplay is not replaced by a successful dummy scene.
        events = setup_replay()
        for frames,message,page,shown in ((86,0,0,9),(180,0,0,None),(282,0,1,None),
                                          (370,1,0,None),(550,2,0,None),(730,3,0,None),(920,4,0,None)):
            script.write_bytes(b''.join(struct.pack('<II',*event) for event in events if event[0]<frames))
            capture = root/f'speech-{frames}.bmp'
            run(executable,*common,'--frames',str(frames),'--input-script',script,'--output',capture)
            check_speech_pixels(source,capture,message,page,shown)
        # Both keyboard pages use retail caption streams and palette 3.
        for shifted in (False,True):
            naming = [event for event in events if event[0]<430]
            if shifted:
                for tick,mask in [(430,4),(432,16)]:
                    naming += [(tick,mask),(tick+1,0)]
            script.write_bytes(b''.join(struct.pack('<II',*event) for event in naming))
            capture = root/('symbols.bmp' if shifted else 'name.bmp')
            run(executable,*common,'--frames','436','--input-script',script,'--output',capture)
            check_name_pixels(source,capture,shifted)
        # Opening and closing use the integer source geometry. Full dialog text
        # is absent while animating; these snapshots are before any choice.
        for frames,edge_y in ((65,193),(434,170)):
            script.write_bytes(b''.join(struct.pack('<II',*event) for event in events if event[0]<frames))
            capture = root/f'box-{frames}.bmp'
            run(executable,*common,'--frames',str(frames),'--input-script',script,'--output',capture)
            raw = bmp(capture)
            assert raw[(edge_y*320+100)*3:(edge_y*320+101)*3] == rgb(31 | (29<<5) | (26<<10))
            assert raw[(164*320+100)*3:(164*320+101)*3] != rgb(31 | (29<<5) | (26<<10))
        for frames in (462,594,604,636):
            script.write_bytes(b''.join(struct.pack('<II',*event) for event in events if event[0]<frames))
            capture = root/f'prompt-{frames}.bmp'
            run(executable,*common,'--frames',str(frames),'--input-script',script,'--output',capture)
            check_question_pixels(source,capture,0 if frames==462 else 1)
            if frames in (462,636):
                offsets = struct.unpack('<33I',(source/'A_FILE.HDT').read_bytes())
                member, = exe_words(source,0x8005597C+6,1)
                slot = 0x3A6 if frames==462 else 0x3A7
                with (source/'A_FILE.BIN').open('rb') as stream:
                    stream.seek(offsets[member]+slot*8192+6144)
                    words = struct.unpack('<64H',stream.read(128))
                assert words[0]==0xFF3
                expected = text_pixels(source,words[1:],30,168)
                expected.update(text_pixels(source,exe_words(source,0x8005175C,16),147,78))
                expected.update(ui_pixels(source,11,1,153,84))
                if frames==636:
                    expected.update(text_pixels(source,exe_words(source,0x80053946,14),173,35))
                    expected.update(text_pixels(source,number_words(1),277,35))
                    expected.update(ui_pixels(source,43,0,253,35))
                check_pixels(capture,expected)
                # The underlying keyboard/calendar is hidden during confirmation.
                raw = bmp(capture)
                y = 64 if frames==462 else 56
                assert raw[(y*320+100)*3:(y*320+101)*3] != rgb(31 | (29<<5) | (26<<10))
            else:
                expected = text_pixels(source,exe_words(source,0x80053946,14),173,35)
                for day in range(30):
                    expected.update(text_pixels(source,number_words(day+1),30+(day%7)*39,88+(day//7)*14))
                for season in range(4):
                    expected.update(ui_pixels(source,43,season,30+season*64,60))
                if frames==604:
                    expected.update(ui_pixels(source,43,0,253,35))
                expected.update(ui_pixels(source,11,1,36,66 if frames==594 else 94))
                check_pixels(capture,expected)
            if frames==636: check_number_padding(source,capture,277,35,299)
        for frames,question in ((780,2),(970,3)):
            script.write_bytes(b''.join(struct.pack('<II',*event) for event in events if event[0]<frames))
            capture = root/f'question-{frames}.bmp'
            run(executable,*common,'--frames',str(frames),'--input-script',script,'--output',capture)
            check_question_pixels(source,capture,question)
        # Moving to No uses the second choice row without accepting the name.
        no_events = [event for event in events if event[0]<466]+[(466,2),(467,0)]
        script.write_bytes(b''.join(struct.pack('<II',*event) for event in no_events))
        capture = root/'name-no.bmp'
        run(executable,*common,'--frames','469','--input-script',script,'--output',capture)
        check_pixels(capture,ui_pixels(source,11,1,153,96))
        script.write_bytes(b''.join(struct.pack('<II',*event) for event in events if event[0]<1044))
        capture=root/'review-ok.bmp'
        run(executable,*common,'--frames','1044','--input-script',script,'--output',capture)
        check_review_pixels(source,capture,0)
        if not retail:
            retry=[event for event in events if event[0]<1044]+[(1044,2),(1045,0)]
            script.write_bytes(b''.join(struct.pack('<II',*event) for event in retry))
            capture=root/'review-retry.bmp'
            run(executable,*common,'--frames','1046','--input-script',script,'--output',capture)
            check_review_pixels(source,capture,1)
            retry += [(1046,16),(1047,0)]
            script.write_bytes(b''.join(struct.pack('<II',*event) for event in retry))
            output=run(executable,*common,'--frames','1080','--input-script',script)
            assert 'title setup: tick=1046 stage=0 length=0 confirming=0' in output
            assert 'title speech: tick=1046 message=1 start=0 shown=0 waiting=0 active=1' in output
            assert 'title handoff:' not in output, 'Try again must not expose New Game data'
        script.write_bytes(b''.join(struct.pack('<II',*event) for event in events))
        output = run(executable,*common,'--frames','1090','--input-script',script,'--output',root/'setup.bmp')
        for stage in (1,2,3,4,5,6,7):
            assert f'stage={stage} ' in output, output[-1500:]
        assert 'tick=35 page=' not in output, 'Held menu direction must not repeat'
        assert 'title stopped: ticks=1090 page=5 selection=0' in output
        for tick in (59,61,63,65):
            assert f'title setup: tick={tick} ' not in output, 'Opening input must be discarded'
        for line in ('tick=75 kind=6 phase=2','tick=432 kind=1 phase=3',
                     'tick=461 kind=3 phase=2','tick=1061 kind=0 phase=0'):
            assert 'title dialog: '+line in output, line
        assert 'New Game setup ready; gameplay requires phase 06' in output
        assert 'message=0 start=49 ' in output, 'Welcome must advance to its second page'
        for message in range(5):
            assert f'message={message} ' in output, 'Missing mayor dialog'
        assert 'title handoff: names=1/1/1 birthday=0/0 flags=00002000' in output
        # The completed setup screen must contain pixels, not an empty capture.
        assert len(set(bmp(root/'setup.bmp'))) > 3
        check_review_pixels(source,root/'setup.bmp')
        for invalid in (b'',b'bad',struct.pack('<II',1,0),struct.pack('<II',0,128),
                        struct.pack('<4I',0,0,0,0),struct.pack('<4I',0,0,10,0)):
            script.write_bytes(invalid)
            assert 'Replay requires' in run(executable,*common,'--frames','10','--input-script',script,code=1)
        # Desktop/dummy device lifecycle, including pause/queue startup and menu presentation.
        run(executable,'--title','--data-dir',source,'--frames','3')
        if not retail:
            exe = root/'SLUS_011.15'
            original = exe.read_bytes()
            for address,payload in ((0x80049044+6,struct.pack('<H',512)),
                                    (0x800537B2,struct.pack('<H',0x400A)),
                                    (0x80051A94,struct.pack('<H',0x400A)),
                                    (0x80051AAA,bytes(30)),
                                    (0x80049052,struct.pack('<H',6)),
                                    (0x80058264,struct.pack('<I',0)),
                                    (0x80058264,struct.pack('<I',1025)),
                                    (0x80058264+4,struct.pack('<I',512)),
                                    (0x80053946,struct.pack('<H',0x400A)),
                                    (0x80051AF6,struct.pack('<H',0x400B)),
                                    (0x80051AF6,bytes(134)),
                                    (0x80053888,bytes(256))):
                broken = bytearray(original)
                offset = 2048+address-0x80010000
                broken[offset:offset+len(payload)] = payload
                exe.write_bytes(broken)
                assert 'incompatible' in run(executable,*common,code=1)
            exe.write_bytes(original)
            exe.write_bytes(b'invalid')
            assert 'incompatible' in run(executable,*common,code=1)
            exe.write_bytes(original)
            archive = root/'A_FILE.BIN'
            data = archive.read_bytes()
            offsets = struct.unpack('<33I',(root/'A_FILE.HDT').read_bytes())
            for offset,payload in ((offsets[5]+0x3A6*8192,bytes(4)),
                                   (offsets[5]+0x3A6*8192+6144,bytes(2)),
                                   (offsets[5]+0x3A7*8192+6146,struct.pack('<H',0x400A)),
                                   (offsets[5]+0x3A7*8192+6146,bytes(126)),
                                   (offsets[5]+0x3A2*8192,bytes(4)),
                                   (offsets[5]+0x3A2*8192+6144,struct.pack('<H',0xF88)),
                                   (offsets[5]+0x397*8192+6144+6,struct.pack('<H',0x400A)),
                                   (offsets[5]+0x3A3*8192+6144,bytes(256)),
                                   (offsets[5]+0x3A2*8192+12,struct.pack('<H',1)),
                                   (offsets[5]+0x3AA*8192+6144,struct.pack('<H',0xFF3)),
                                   (offsets[5]+0x3AA*8192+6144,struct.pack('<8H',0xFFA,0xFFFE,0,0xFFFE,1,0xFFFE,2,0xFFFF)),
                                   (151552+100+11*4,struct.pack('<H',1))):
                broken = bytearray(data)
                broken[offset:offset+len(payload)] = payload
                archive.write_bytes(broken)
                run(executable,*common,code=1)
            archive.write_bytes(data[:-1])
            run(executable,*common,code=1)
    print('native-title: CLI/replay bounds, native menu, unsupported destinations, independent sprite pixels and SDL lifecycle passed'
          + (' (local retail resources; PS1 parity not established)' if retail else ' (synthetic fixtures)'))


if __name__ == '__main__':
    check(Path(sys.argv[1]).resolve(),Path(sys.argv[2]) if len(sys.argv)>2 else None)
