# Progres fase 05 — Layar judul native

Diperbarui: 17 Oktober 2026 (Asia/Jakarta). Baseline: `5645d68`.

## Status

**Berjalan. Intro/fade, animasi latar/aktor dan setup New Game tervalidasi
Windows Debug: 19/19 suite CTest lulus, 0 gagal, total 22,89 detik,
termasuk perubahan bingkai, palet teks dan caption keyboard.
Serah-terima data New Game siap dihubungkan ke fase 06. Kesetaraan
tampilan/timing PS1 belum diuji; fase 05 belum dinyatakan lulus penuh.**

## Lanjutan UI — tervalidasi Windows Debug

- Bingkai bevel dan gradasi krem dipindahkan dari `src/ui/window_frame.c`;
  panel nama/kalender mengikuti koordinat source `name_entry*.c`.
- Startup asli memilih palet teks 3 (`runtime/main.c`); preview sebelumnya
  memakai palet terang default. Setup kini membaca baris CLUT palet 3 dari
  executable, sementara API grafik lama mempertahankan default sebelumnya.
- Caption keyboard dibaca dari `0x800537B2` / `0x80053888`, dengan jarak (1,2),
  termasuk label Mark/alphabet, Space dan OK. Latar diredupkan sebagaimana
  `name_entry_step`/`name_sound_step`.
- Tes ditambah untuk glyph kedua halaman keyboard, palet, tepi/gradasi bingkai
  dan penolakan caption tanpa END/kontrol yang tidak didukung. Syntax Python,
  fixture palet serta batas tabel asli diperiksa. Hasil terminal pengguna:
  **19/19 lulus, 0 gagal, 22,89 detik**; mencakup suite C, CLI sintetis dan
  data asli. Exit code build/CTest tidak dilaporkan terpisah.

Empat capture tambahan (huruf 66 tick, simbol 66 tick, kalender 65 tick,
setup selesai 106 tick) berhasil, masing-masing exit code 0. Capture diperiksa
visual: kedua halaman keyboard, bingkai/gradasi, kalender dan ringkasan terbaca.
Handoff tetap `names=1/1/1 birthday=0/0 flags=00002000`. Tidak ada kode C
yang diubah setelah laporan tes. Bukti lokal di
`build/native-validation/phase05-ui-windows-evidence.json` dan
`phase05-ui-{naming,symbols,calendar,ready}.bmp/png`; audit source/data di
`phase05-ui-source-audit.json`. Asset/capture tidak dikomit.

Konfirmasi pengguna: logo yang terpotong pada screenshot adalah animasi turun
yang sedang berlangsung, bukan bug logo. Gambar kedua memperlihatkan panel
native sebelum perubahan UI ini. Uji desktop tersebut belum membuktikan
kesetaraan dengan PS1. Cursor outline, panel konfirmasi/review, ikon musim,
animasi/typing dialog serta timing masih menjadi sisa pekerjaan.

## Lanjutan setelah baseline

- Rekonstruksi gerak masuk logo/copyright/Start, fade mode 1/2, dua lapis
  latar bergerak dan enam grup aktor dari overlay title. State animasi/RNG
  maju pada tick simulasi; presentasi hanya menggambar cache frame.
- Setup New Game: nama pemain, ulang tahun, nama farm, nama anjing, review.
  Grid menggunakan batas baris/kolom serta simbol dari executable; font Inggris
  asli dibaca dari member 0. Batas delapan glyph, END marker, aturan batal,
  kalender 30 hari dan repeat input mengikuti source yang ditelusuri.
- Konfirmasi akhir menyediakan `BtnNewGame` melalui `btn_title_new_game()`
  setelah fade selesai: tiga nama, season/day berbasis nol, empat seed dan
  flag new-game. Tidak ada handler farm palsu. Layar menjelaskan batas fase 06;
  Continue tetap memerlukan fase 07, dan idle attract memerlukan gameplay.
- Tes diperluas untuk alur setup, batas nama/grid/calendar, reset, fade dan
  cache animasi. Oracle Python independen kini menggambar latar/aktor bergerak.
  Suite C/CLI sintetis dan data lokal lulus, termasuk replay setup lengkap.

Panel nama/kalender kini mengikuti bingkai, posisi, caption dan palet dari source;
prompt dan sebagian layout tetap adaptasi native, bukan port lengkap dialog
retail. Trig host menggantikan
kuantisasi `ccos`/`csin`; penjadwalan transisi dan waktu muat juga belum
dibandingkan PS1. Varian attract dan gameplay berada di batas fase 06.
Keterbatasan ini tetap menjadi gerbang kesetaraan, bukan fitur yang diluluskan.

Source `titleStepDispatch` sebelumnya hanya deklarasi/alamat. Audit lokal
menemukan member 27 (`GF_TITLE.Bin`), memetakan dispatch, aset umum dan musik.
Rekonstruksi C mencakup intro, prompt Start, menu dengan default Continue,
setup New Game dan kontrol kembali. Tidak ada byte MIPS yang dijalankan
sebagai fungsi host.

## Checklist terhadap rencana

| ID | Status | Bukti/perubahan | Sisa |
|---|---|---|---|
| 5.1 | Sebagian | Audit overlay, dispatch, gambar/font/musik; intro, latar/aktor dan setup direkonstruksi dalam C. | Varian attract gameplay dan kesetaraan perilaku retail. |
| 5.2 | Lulus untuk jalur native teruji | Startup SDL, state host, loader berbatas, tanpa CRT0/BIOS; suite sintetis dan data lokal lulus. | Integrasi global gameplay fase 06. |
| 5.3 | Lulus sampai handoff setup | `btn_title_tick`/`btn_title_draw` menjalankan C host; `btn_title_new_game()` menyediakan data setelah fade selesai. | Konsumen gameplay fase 06 dan save/load fase 07. |
| 5.4 | Lulus untuk reset teruji | Resource terpisah dari state; reset intro/aktor/setup dan replay lulus tanpa pointer handler PS1. | Perpindahan scene gameplay/save-load. |
| 5.5 | Sebagian | Renderer, audio, input, fade dan animasi memakai tick nominal 30 Hz; tes 30/60/144 FPS lulus. | Timing NTSC fraksional, kuantisasi trig dan acuan PS1. |
| 5.6 | Sebagian | Intro, dua lapis latar, enam grup aktor, teks asli, musik, SFX dan navigasi/setup terhubung. | Window/dialog retail dan kesetaraan visual/audio. |
| 5.7 | Lulus untuk fixture dan data lokal | Seluruh 19 suite lulus; oracle piksel animasi, replay setup, batas nama/calendar dan lifecycle SDL dummy. | Uji desktop/controller/dengar serta input/timing yang sama dengan PS1. |
| 5.8 | Batas fitur teruji | New Game menyelesaikan setup; log/panel menjelaskan gameplay fase 06 dan Continue fase 07. | Tujuan gameplay/save-load belum berfungsi; tidak dihitung lulus. |

## Bukti audit dan input

Tool `tools/audit_native_title.py` berhasil pada input lokal. Laporan hanya
metadata/hash: `build/native-validation/phase05-title-audit.json`, diabaikan Git.
Satu tes unit Python untuk layout dan batas dispatch lulus, exit code 0.
Syntax Python dan fixture oracle piksel sintetis juga diperiksa. Suite C dan
executable native kemudian diuji pengguna, sebagaimana dicatat di bawah.

| Input/resource | SHA-256 |
|---|---|
| A_FILE.HDT | `0cae44d6bdb29e078a182d7cd9a780c21e35dc54f55eb94c2d9ea8deb20c82dd` |
| A_FILE.BIN | `612f9ac8279a364c2fdd19cb75e63df2e892a6f52b6f1d8081ea8560b18decd4` |
| SLUS_011.15 | `34e3b35dcd3fb1cd9a7d4813b8a9ef987915826e3d436f1908ecbe2d4fdf1444` |
| Member 27 | `ea6a7aef3d522bc1c905417cbf641d0a3bfd79b5aa68a95edd06f4d880f84b9e` |
| 0E title | `bcda9442da10ee01868461c34ce62316c18bc06a5563abafc0c1d506926a1999` |
| 0D title | `e49bfa9d8ece82a7531022d13a99e2cc7844804e45a3f4a1a65bf94c20840226` |

Overlay dimuat di `0x800B7E20`, entry `0x800B7EEC`, dispatch `0x800C49F8`.
0E: member 0 offset 8.192, 3.356 byte, 38 range. 0D: offset 11.548,
139.888 byte, 73 blok. Musik dipetakan melalui `sound_request(26)`, bukan
dugaan pasangan berdasarkan kedekatan resource. Definisi efek 0 dan 5 memakai
program 0, masing-masing tone 0 dan 5; adapter eksplisit mencegah pemutaran
seluruh tone program sekaligus.

## Gerbang validasi

Visual Studio MCP melaporkan tidak ada instance dengan extension aktif.
Sesuai aturan sesi, configure/build/CTest yang berpotensi berlog panjang
dijalankan pengguna. Yang diminta hanya status/exit code dan baris error
relevan bila gagal; log penuh tidak diperlukan.

Hasil terminal pengguna, 17 Oktober 2026: **19/19 lulus, 0 gagal, total 22,89
detik**. Empat suite title (`native-title`, `native-title-cli`,
`native-title-tools`, `native-title-data`) semuanya lulus. Kelima belas suite
sebelumnya ikut dijalankan dan lulus, termasuk runtime/input, archive, grafik
dan audio. Hasil ini mencakup intro/fade, animasi, setup dan penyelarasan UI.
Tidak ada kode C yang diubah setelah laporan tes tersebut.

| Kelompok | Suite lulus | Jalur |
|---|---:|---|
| Runtime/input | 3 | Core, event adapter, CLI sintetis |
| Arsip/data tools | 5 | Parser, rentang/async, CLI, tools, input lokal |
| Grafik | 3 | Renderer, CLI dan decoder independen data lokal |
| Audio | 4 | Decoder/mixer, tone eksplisit, CLI/tools dan data lokal |
| Title | 4 | State/clock, CLI/replay/oracle, audit tools dan data lokal |

Exit code configure/build/CTest tidak dilaporkan terpisah. Laporan CTest
menyatakan 100% lulus dan executable baru berhasil dijalankan. Empat capture
lokal tambahan (huruf/simbol 66 tick, kalender 65 tick, setup selesai 106 tick)
berhasil, masing-masing exit code 0. Framebuffer diperiksa visual: keyboard,
kalender dan ringkasan data terbaca, tanpa membuka window atau audio device.
Log akhir mencatat `names=1/1/1 birthday=0/0 flags=00002000`, page 5.
Ini tidak menggantikan uji desktop/controller atau uji dengar.

Konfigurasi: Windows x64 Debug, generator Visual Studio 18 2026, **MSVC
19.51.36260.0** (metadata compiler CMake), **SDL 3.2.10** (output executable).
Bukti ringkas, hasil suite, hash source dan binary disimpan lokal di
`build/native-validation/phase05-ui-windows-evidence.json`. Capture lokal:
`build/native-validation/phase05-ui-{naming,symbols,calendar,ready}.bmp` dan `.png`.
Asset/capture tidak dikomit. Bukti baseline `ad23134` (19/19, 15,81 detik)
tetap terpisah di `phase05-windows-evidence.json`; baseline animasi/setup
`28d9223` (19/19, 19,02 detik) di `phase05-setup-windows-evidence.json`.

Linux ASan/UBSan dan capture PS1 belum diuji. Rencana asal di direktori induk
tidak ditandai selesai. Intro, animasi latar/aktor, fade dan input nama kini
tersedia. Attract gameplay, perpindahan ke farm dan save/load belum tersedia.

Commit implementasi dicatat di
[Notes fase 05](https://notes.dlands.me/doc/progres-fase-05-layar-judul-native-BwWOSiJ5r9)
setelah commit dibuat; commit ini disimpan lokal, belum dipush.
Panduan pemakaian, format replay dan keterbatasan: [native-title.md](native-title.md).
