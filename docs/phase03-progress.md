# Progres fase 03 — Grafik, GTE, dan asset viewer

Diperbarui: 17 Oktober 2026 (Asia/Jakarta).

## Status

**Berjalan. Implementasi viewer dan pengujian Windows sudah lulus; syarat lulus
seluruh fase belum terpenuhi.** Build Debug dan CTest terakhir menghasilkan
exit code 0 berdasarkan hasil terminal pengguna. Konfigurasi memiliki 11 suite.
Tidak ada log penuh yang diambil. Identitas commit implementasi dicatat pada
[Notes fase 03](https://notes.dlands.me/doc/progres-fase-03-grafik-gte-dan-asset-viewer-moyy7a0Y4L)
dan bukti lokal di bawah `build/native-validation/`.

**Keputusan pengguna:** pengujian Linux ASan/UBSan ditunda. Instalasi WSL
belum dilakukan karena ruang disk C pada laptop terbatas. Opsi sanitizer sudah
tersedia; yang ditunda adalah eksekusi pengujiannya. Tidak ada instalasi WSL
atau penambahan kebutuhan ruang disk untuk Linux pada sesi ini.

Prasyarat fase 02 dikonfirmasi dari
[catatan pembaca data](https://notes.dlands.me/doc/progres-fase-02-pembaca-data-wgaFUtEklQ):
8/8 suite lulus, 32/32 span arsip cocok dengan pembacaan independen, bank teks
terverifikasi, dan commit lokal
`10df9412c92c0e64b8d0d92f6ee78b67f2ec28ba`.

## Checklist terhadap rencana

Checkbox berikut mencatat implementasi yang telah diuji dalam cakupan viewer.
Perbandingan PS1 dan kebutuhan scene title tetap menjadi gerbang kelulusan fase.

| ID | Status | Perubahan dan bukti | Sisa |
|---|---|---|---|
| 3.1 | Lulus | Resource 0D `gf`/wajah/font dipilih dari manifest; offset dan rectangle diperiksa. | Audit resource lain saat dipilih. |
| 3.2 | Lulus untuk resource pilihan | Decoder raw/RLE baris/kolom, CLUT dan indeks warna; 8 gambar input cocok per piksel dengan decoder Python independen. | TIM/format lain belum dibutuhkan pada jalur pilihan. |
| 3.3 | Lulus untuk API host | VRAM 1024x512, upload/read/copy dengan batas kapasitas; tes rentang dan overlap. | Semantik wrapping/masking transfer GPU dan overlap perangkat keras belum dibandingkan. |
| 3.4 | Lulus untuk adegan uji | Sprite/tile, line, flat/Gouraud triangle/quad; tes clipping dan tepi quad. | Golden rasterisasi PS1. |
| 3.5 | Lulus untuk jalur native | OT memakai indeks arena, descending bucket/head insertion; tes urutan, kapasitas dan link/cycle pada executable x64. | Source matching tetap memakai tag PS1; belum ditautkan ke scene host. |
| 3.6 | Lulus pada harness | Rasterizer CPU dan framebuffer terpisah dari presentasi SDL3; CTest dummy-video lulus. | Kontrol desktop/fullscreen belum diperiksa manual; backend GPU ditunda sesuai rencana. |
| 3.7 | Lulus untuk harness | Clip, draw offset, texture page/window, transparansi, empat blend mode, masks dan dithering tersedia. | Catatan perbedaan/rounding PS1 di panduan. |
| 3.8 | Sebagian | C RTPS/RTPT, MVMVA, NCLIP, AVSZ3/4, UNR, FIFO, fixed-point, wrap, saturasi dan proyeksi batas diuji. | Audit operasi yang dibutuhkan title; lighting/color GTE dan uji acuan PS1. |
| 3.9 | Sebagian | Model kubus berindeks, kamera, transformasi/animasi deterministik dan 6 frame animasi 0E asli diuji; pemetaan affine. | Pemilihan/integrasi model TMD retail dan scene title. |
| 3.10 | Sebagian | Glyph Inggris 8x12 dari `gf` blok 9, CLUT y=254, space/newline/END; piksel teks cocok dengan acuan decoder independen. | Kebutuhan kontrol teks title belum ditentukan; kontrol lain ditolak eksplisit. |
| 3.11 | Lulus | `btn-graphics --asset-viewer` dan `--graphics-test`, pemilihan member/offset/image/palette/depth/range/frame dan overlay diagnosis. | Uji interaktif desktop. |
| 3.12 | Sebagian | Capture PPM deterministik, pembanding diff/metrik/hash, identitas resource/tick dan daftar perbedaan tersedia. | Capture PS1 dengan kondisi sama. Linux ASan/UBSan ditunda atas keputusan pengguna. |

**Fase ini belum ditandai lulus.** Kecocokan decoder Python membuktikan konsistensi
terhadap input lokal, bukan identitas visual terhadap perangkat keras PS1.
Tidak ada gameplay/overlay title yang diklaim berjalan.

## Hasil pengujian

Platform: Windows x64 Debug, generator Visual Studio 18 2026, SDL **3.2.10**
(`SDL_GetVersion()=3002010`), pointer host **64 bit**, baseline commit fase 02
di atas. CMake compiler metadata mencatat **MSVC 19.51.36260.0**; waktu CTest
akhir tidak dilaporkan.

| Bukti | Hasil |
|---|---|
| Configure awal | Exit code 0, dilaporkan pengguna. |
| Build setelah perbaikan font/overlay | Exit code 0, dilaporkan pengguna. |
| CTest akhir | Exit code 0, dilaporkan pengguna; 11 suite terdaftar pada konfigurasi data asli. |
| CTest awal | 10/11 lulus; satu fixture rotasi memilih sudut simetris 90°. Diganti 45°, pengulangan terfokus lulus. |
| Pembanding gambar Python | 1 tes unit lulus; metrik identik/berbeda dan input rusak/ukuran berbeda. |
| CLI terfokus | Lulus; capture deterministik, perubahan rotasi, proteksi output dan malformed data. |
| Data asli | 8 gambar, 6 frame animasi dan area glyph Inggris dibandingkan dengan decoder/table reader Python independen melalui suite data. |
| Pratinjau native akhir | Scene tick 8, wajah member 1 dan frame 0 Status member 14 berhasil, GTE flags 0, hasil PNG diperiksa visual. |
| Pemeriksaan patch | `git diff --check` lulus. |
| Linux ASan/UBSan | Belum dijalankan; ditunda atas keputusan pengguna karena kapasitas disk C untuk WSL terbatas. |

Perbaikan atlas font didasarkan pada hasil visual dan source: atlas Jepang pada
member 7 bukan page Inggris untuk `text_glyph_sprite`. Bank `gf` memiliki atlas
16 kolom 8x12 pada blok 9; palette default y=254 dikonfirmasi dari
`textClutRows[0]` executable. Tes area glyph kini memeriksa pemilihan bank dan
palette ini, bukan hanya keberhasilan membuka window.

Suite terdaftar: `native-core`, `native-input`, `native-sdl`, `native-archive`,
`native-archive-cli`, `native-archive-load`, `native-data-tools`,
`native-graphics`, `native-graphics-cli`, `native-archive-data`,
`native-graphics-data`.

## Identitas input dan artefak lokal

Asset berada di `disc/btn-assets`, diabaikan Git. Identitas yang diteruskan dari
audit fase 02:

| File | Byte | SHA-256 |
|---|---:|---|
| A_FILE.HDT | 132 | `0cae44d6bdb29e078a182d7cd9a780c21e35dc54f55eb94c2d9ea8deb20c82dd` |
| A_FILE.BIN | 133935104 | `612f9ac8279a364c2fdd19cb75e63df2e892a6f52b6f1d8081ea8560b18decd4` |
| SLUS_011.15 | 323584 | `34e3b35dcd3fb1cd9a7d4813b8a9ef987915826e3d436f1908ecbe2d4fdf1444` |

File lokal di bawah `build/native-validation/`:

- `phase03-windows-evidence.json`: sumber bukti, status, hash source/executable dan capture.
- `phase03-scene-verified.ppm` / `.png`: scene sintetis tick 8 dengan font Inggris.
- `phase03-portrait.ppm` / `.png`: member 1, image 1, palette 0, tick 0.
- `phase03-status.ppm` / `.png`: member 14, bank offset 11636, animasi offset 0, range/frame 0.

Source, perintah viewer, format data, lokasi bank animasi, reproduksi tes dan
perbandingan gambar dijelaskan di [native-graphics.md](native-graphics.md).
Workflow CI native yang sudah ada otomatis mencakup target dan tes baru serta
opsi sanitizer Linux; workflow itu belum dijalankan dalam sesi ini.

## Langkah berikutnya

1. Jalankan viewer desktop untuk memeriksa kontrol, resize dan fullscreen.
2. Tentukan asset/model/operasi GTE/kontrol teks yang benar-benar dipakai title,
   lalu lengkapi consumer tersebut tanpa mengeksekusi overlay PS1 sebagai kode host.
3. Sediakan capture PS1 untuk input/camera/tick/display crop yang sama; jalankan
   `tools/compare_graphics.py` dan catat setiap perbedaan yang masih tersisa.
4. **Ditunda:** jalankan konfigurasi Linux dengan `BTN_SANITIZERS=ON` setelah
   lingkungan Linux tersedia. Tidak perlu memasang WSL sekarang.

Panduan teknis mengacu pada source repo, [spesifikasi GPU](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/),
[spesifikasi GTE](https://psx-spx.consoledev.net/geometrytransformationenginegte/),
dan [presentasi texture SDL3](https://wiki.libsdl.org/SDL3/SDL_UpdateTexture).
