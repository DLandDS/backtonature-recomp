# Progres fase 05 — Layar judul native

Diperbarui: 17 Oktober 2026 (Asia/Jakarta). Baseline: `5645d68`.

## Status

**Berjalan. Baseline `d9bc562`: intro/fade, animasi, setup, UI dan animasi dialog native
tervalidasi Windows Debug: 19/19 lulus, 0 gagal, 60,02 detik, CTest exit 0.
Lanjutan panel pertanyaan lulus empat suite title: 60,15 detik, CTest exit 0.
Dialog wali kota lulus empat suite title (227,58 detik, CTest exit 0);
koreksi indikator berikutnya lulus rebuild (exit 0) dan dua capture terarah.
Header ulang tahun/review akhir lulus empat suite title (256,64 detik,
CTest exit 0) dan tiga capture data asli tambahan.
Gerak latar integer SDK lulus empat suite title (277,58 detik, CTest exit 0)
dan dua capture seluruh framebuffer.
Serah-terima data New Game siap dihubungkan ke fase 06. Kesetaraan
visual/timing/audio PS1 belum diuji; fase 05 belum dinyatakan lulus penuh.**

## Gerak latar integer SDK — tervalidasi Windows Debug

Lanjutan setelah `a5f60f2` mengganti sinus/cosinus host dengan port C jalur
integer SDK: sudut `frame*4096/120` dibulatkan ke bawah, enam rotasi CORDIC,
koreksi sudut sisa dan lipatan kuadran asli. Enam konstanta sudut dibaca dari
executable `0x80058264`; nol, lebih dari 1024 atau urutan tidak menurun ditolak.
Fungsi asli: `ccos` `0x80036CB4`, `csin` `0x80036F44`, `csincos` `0x80036D84`;
pemanggil overlay `0x800B944C`. Runtime tetap hanya menjalankan C host.

Diagnostik lokal berbatas mencocokkan model dengan kedua fungsi SDK untuk
seluruh 4096 sudut, memakai tabel asli dan sintetis: semua cocok. Perbandingan
120 langkah menemukan 40 dari 240 sampel pergeseran berbeda pada jalur host
lama. Ini bukti aritmetika, bukan pengukuran raster atau timing PS1.

Tes unit menambahkan checksum satu siklus penuh dari eksekusi SDK dengan tabel
sintetis; CLI menambahkan frame 12 dan penolakan tabel rusak. Oracle baru
berhasil menolak framebuffer native lama. Syntax Python, fixture dan audit
data lulus. Hasil pengguna: **4/4 title lulus, 0 gagal, 277,58 detik,
CTest exit 0**. Unit 0,03 detik, CLI 105,92, tools 0,20 dan data asli 171,42.
Exit code build tidak dilaporkan terpisah; skrip hanya menjalankan CTest setelah
build berhasil. Lima belas suite lain tidak diulang.

Dua capture tambahan pada tick 12 berhasil (exit 0): sintetis dan data asli.
Seluruh framebuffer cocok dengan oracle SDK; capture sintetis baru juga berbeda
dari hasil host-trig lama. Capture data asli diperiksa visual. Hash source C/tes
tetap sama dengan bukti sebelum build; hanya dokumentasi diperbarui setelah tes.
Bukti lokal: `build/native-validation/phase05-wave-{source-proof,audit,windows-evidence}.json`
dan `phase05-wave-{synthetic,retail}.bmp/png`. Asset/capture tetap lokal.
Clock, input dan audio tidak diubah. Validasi PS1 terkontrol masih terbuka.

## Header ulang tahun dan review akhir — tervalidasi Windows Debug

Lanjutan setelah `5d7ff69` memakai caption lengkap ulang tahun dari executable
`0x80053946`, termasuk tanda baca/garis miring. Angka hari satu digit memakai
spasi awal, mengikuti `text_format_number`; kalender ikut memakai format ini.

Ringkasan mengikuti fungsi overlay `0x800B9124`: panel `(72,56,176,90)`, teks
`(76,60)` berjarak `(0,1)`, ikon musim `(172,72)` dan tanggal `(188,72)`.
Template 67 word dari `0x80051AF6` diekspansi secara berbatas untuk tiga nama;
insert tak didukung, nama tak valid, END hilang dan overflow ditolak.

Pilihan akhir membaca slot teks `0x3AA` (CHOICE_2) sebagai dua baris asli,
teks `(30,168)` jarak `(0,2)` dan cursor `(36,174 + pilihan*14)`. A/Z
mengonfirmasi; B/X dan Start diabaikan. Arah diproses lebih dulu bila ditekan
bersama A. Pilihan kedua kembali ke pertanyaan nama, tanpa Welcome dan tanpa
handoff; pilihan pertama menunggu kotak/fade selesai sebelum handoff.

Tes C dan CLI diperluas untuk pemformatan, kedua pilihan/row cursor, reset
Try again serta input/resource rusak. Audit sumber/data, syntax Python dan
layout fixture lulus; oracle ringkasan baru menolak capture native lama.
Hasil pengguna: **4/4 suite title lulus, 256,64 detik, CTest exit 0**.
Unit 0,03 detik, CLI 84,27, tools 0,14, data asli 172,18. Exit code build
tidak dilaporkan terpisah; skrip menjalankan CTest hanya setelah build berhasil.
Lima belas suite lain tidak diulang.

Tiga capture data asli tambahan berhasil (exit 0): pilihan pertama 1044 frame,
pilihan kedua 1046 dan pembukaan ulang pertanyaan nama 1080. Pemeriksaan piksel
ringkasan/teks/cursor kedua pilihan lulus; log retry membuktikan reset nama,
aktivasi pertanyaan nama dan tidak ada handoff. Ketiganya diperiksa visual.
Bukti lokal: `build/native-validation/phase05-review-windows-evidence.json`,
audit `phase05-review-audit.json`, capture `phase05-review-{ok,retry,name-again}.bmp/png`.
Asset dan capture tetap lokal. Kesetaraan timing/input/audio PS1 masih terbuka.

## Dialog wali kota — tervalidasi dengan koreksi indikator terarah

Lanjutan setelah `75786a5` menambahkan dua pesan pembuka dan pertanyaan
sebelum nama pemain, ulang tahun, farm dan anjing. Slot asli berurutan:
`0x3A2`, `0x397`, `0x3A5`, `0x3A4`, `0x3A3`; Welcome memiliki dua halaman.
Kelima bank portrait halaman 104 identik dan dibaca dari slot +4. Tidak ada
asset/teks game yang ditanam ke source. Audit membuktikan persegi VRAM tidak
bertumpuk dengan title, shared UI atau font.

Model menghubungkan reveal teks, jeda halaman/akhir, tahan A/B untuk speed 0,
portrait `(26,84,80,80)`, indikator tunggu UI range 7 dan SFX teks 4.
Grid/setup input diblokir selama percakapan; handoff juga memerlukan speech
nonaktif. Panel memakai animasi buka/tutup yang sudah ada. Teks dibatasi 128
word, dua halaman dan empat baris per halaman; kontrol lain ditolak.

Tes C dan oracle piksel CLI diperluas, termasuk gambar portrait rusak,
kontrol tak didukung, END hilang dan tombol tahan saat menunggu. Replay baru
berakhir pada frame 1090. Audit sumber/data, decoder fixture sintetis, urutan
event dan syntax Python lulus. Hasil pengguna: **4/4 suite title lulus,
227,58 detik, CTest exit 0**. Unit 0,03 detik, CLI 64,91, tools 0,15,
data asli 162,48. Exit code build tidak dilaporkan terpisah; 15 suite lain
tidak diulang.

Lima capture tambahan berhasil (exit 0): teks parsial 86 frame, Welcome 180,
halaman kedua 282, pertanyaan ulang tahun 550 dan handoff 1090. Portrait,
teks dan handoff terlihat benar; log tetap
`names=1/1/1 birthday=0/0 flags=00002000`.

Pemeriksaan visual menemukan indikator tunggu salah: range 1 menggambar hati.
Source `DrawDialogUiLabelFromArg` memilih range 7 saat textWaitFrames nol.
Kode dan audit diperbaiki; fixture kini membedakan kedua range dan oracle
indikator menolak capture lama. Pengguna melaporkan **rebuild btn-native
exit 0**. Dua capture 180 frame setelah koreksi, sintetis dan data asli,
berhasil (exit 0) dan lulus oracle portrait/teks/indikator range 7. Hasil data
asli diperiksa visual dan menampilkan jam pasir. CTest tidak diulang setelah
koreksi range; hasil empat suite di atas berlaku sebelum koreksi tersebut.
Tidak ada perubahan C setelah rebuild yang berhasil.

Bukti akhir lokal: `build/native-validation/phase05-mayor-windows-evidence.json`,
`phase05-mayor-initial-test.json`, `phase05-mayor-wait-audit.json`, dan
`phase05-mayor-wait-{synthetic,retail}.bmp/png`. Asset/capture tidak dikomit.
Timing dibanding PS1, latensi I/O, format header ulang tahun dan review akhir
native (slot retail `0x3AA` belum diport) tetap terbuka. Lihat juga
[native-title.md](native-title.md#mayor-conversation-windows-debug-validated).

## Acuan video dan panel pertanyaan — tervalidasi Windows Debug

Video pengguna berhasil diperiksa melalui browser. Temuan bertimestamp dan
batas buktinya tercatat di [phase05-reference.md](phase05-reference.md).
Dialog wali kota/portrait, panel pertanyaan dan format header ulang tahun
memiliki perbedaan yang terlihat terhadap baseline native. Audio dan timing
per frame belum diperiksa; video tidak membuktikan hash/versi game atau input.

Panel pertanyaan pemain/ulang tahun/farm/anjing kini ditulis memakai empat
stream asli executable, window `(16,16,120,20)` dan teks `(20,20)` dari fungsi
overlay `0x800BA130`. Loader memakai kapasitas masing-masing termasuk END.
Audit sumber/data dan syntax Python lulus. Hasil terminal pengguna: **4/4 suite
title lulus, 0 gagal, 60,15 detik, CTest exit 0**. Rincian: unit 0,03 detik,
CLI sintetis 18,78, tools 0,22, data asli 41,12. Exit code build tidak dilaporkan
terpisah. Lima belas suite lain tidak diulang; hasil 19/19 baseline tetap
terpisah. Tidak ada perubahan kode C setelah hasil ini.

Empat capture tambahan berhasil dan diperiksa visual, masing-masing exit 0:
pemain 86 frame, ulang tahun 154, farm 234, anjing 308. Bukti lokal:
`build/native-validation/phase05-questions-windows-evidence.json`,
`phase05-questions-audit.json`, dan `phase05-questions-{player,birthday,farm,dog}.bmp/png`.
Asset/capture tidak dikomit. Dialog wali kota/portrait/reveal teks, format
header ulang tahun, review native dan pengukuran acuan PS1 masih terbuka.

## Lanjutan animasi dialog — tervalidasi Windows Debug

- Controller memakai geometri bilangan bulat `menu_box_animate`: parameter
  durasi 12 untuk buka, 6 untuk tutup, enam langkah lebar dan skip frame dengan
  tinggi nol. Model memerlukan 18 tick buka dan 12 tick tutup; frame awal
  dengan lebar nol tidak digambar.
- Dialog lama menutup sebelum pesan baru membuka. Teks/pilihan tampil saat
  siap; input selama transisi dibuang. Repeat pad terus dihitung, dan handoff
  menunggu penutupan dialog terakhir. Model maju pada tick simulasi.
- Audit ulang `RenderDialogTextPage` menunjukkan NO_SCROLL (`0x0FF3`) mengisi
  textIndex hingga END. Pesan konfirmasi setup asli muncul sekaligus; efek
  ketik tidak ditambahkan pada pesan ini. Engine dialog umum/portrait/scroll
  tidak diklaim sudah tersedia.
- Tes C ditambah untuk geometri, jumlah tick, input/held-repeat saat transisi,
  reset dan batas handoff. Replay lengkap kini berakhir pada tick 426; capture
  parsial membuka/menutup dan teks lengkap saat ready ditambahkan.

Hasil terminal pengguna, 17 Oktober 2026: **19/19 lulus, 0 gagal, 60,02 detik,
CTest exit 0**. Seluruh suite, termasuk CLI sintetis (14,12 detik) dan data
asli title (30,74 detik), lulus pada run penuh terbaru. Exit code build tidak
dilaporkan terpisah. Tidak ada perubahan kode C setelah hasil tersebut.

Tujuh capture tambahan berhasil, masing-masing exit 0: buka 65 frame, tutup
84, nama 86, konfirmasi nama 112, kalender 154, konfirmasi ulang tahun 192 dan
setup selesai 426. Pemeriksaan visual selesai; log handoff tetap
`names=1/1/1 birthday=0/0 flags=00002000`. Audit sumber/data juga berhasil,
exit 0. Capture headless tidak menggantikan uji desktop/controller/dengar.

Bukti lokal: `build/native-validation/phase05-dialog-windows-evidence.json`,
`phase05-dialog-audit.json`, dan `phase05-dialog-{opening,closing,name,confirm,calendar,birthday,ready}.bmp/png`.
Asset/capture tidak dikomit. Platform Windows x64 Debug, Visual Studio 18 2026,
MSVC 19.51.36260.0, SDL 3.2.10.

Timing I/O asinkron, penjadwalan terhadap scene/fade, raster dan urutan input
fisik terhadap PS1 tetap perlu dibandingkan. Review handoff dan bantuan tombol
masih merupakan adaptasi native.

## Lanjutan cursor/ikon dan konfirmasi — tervalidasi dengan ulang terarah

- Aset UI bersama: member 0, animasi offset 151.552 (5.084 byte, 44 range),
  bank gambar offset 156.636 (16.152 byte, 140 blok). Seluruh persegi image
  dan palette cocok dengan bank terpilih; tidak bertumpuk dengan title/font.
- Cursor tangan memakai range 11 frame 1; ikon musim range 43 frame 0..3.
  Posisi mengikuti `name_marker_draw`, `name_sound_draw` dan
  `name_title_build_display`. Cursor menggantikan outline kuning.
- Konfirmasi nama/ulang tahun membaca Yes/No executable dan teks asli slot
  `0x3A6`/`0x3A7` dari member yang dipetakan `menuWindowValues[3]` (5 pada US).
  Kotak pilihan mengikuti `name_prompt_draw`; grid/kalender disembunyikan
  selama konfirmasi. Input dan data handoff tidak diubah.
- Audit source/data lulus: `build/native-validation/phase05-icons-audit.json`.
  Lima capture viewer aset (cursor dan empat musim) berhasil, exit code 0;
  pikselnya cocok dengan decoder Python independen. Fixture dan syntax Python
  diperiksa. Validasi C/CLI terbaru dicatat di bawah.

Tes diperluas untuk kedua pilihan, piksel ikon/cursor/prompt, grid tersembunyi,
slot pesan rusak dan frame cursor yang hilang. Hanya slot teks dengan prefix
NO_SCROLL dan END berbatas yang didukung. Pada baseline ini animasi kotak,
review native dan perbandingan PS1 masih tersisa. Audit lanjutan di atas
menjelaskan mengapa pesan NO_SCROLL tidak memerlukan efek ketik.

Validasi pertama lanjutan ini: **18/19 lulus**, total 26,91 detik, CTest exit 8.
`native-title-data` gagal pada `prompt-65.bmp`, piksel (34,66). Capture terarah
mengonfirmasi RGB aktual (123,197,32) sama dengan ikon Spring; cursor seharusnya
(90,90,90). Cursor dan musim masuk bucket yang sama sehingga musim menimpa
cursor. Perbaikan memisahkan bucket cursor (0) dari musim/teks (1), mengikuti
slot OT +0x60 / +0x64 pada `name_sound_draw`. Fixture sintetis kini sengaja
membuat warna cursor/musim berbeda dan saling bertumpuk agar bug ini tidak
hanya terdeteksi data asli. Perbaikan kemudian dibuild dan diuji ulang pada
dua suite CLI/data title; kode core/shared tidak diubah oleh perbaikan ini.

Ulang terarah: `native-title-data` lulus (13,66 detik), sementara
`native-title-cli` gagal pada fixture, piksel (88,65); total 16,58 detik,
CTest exit 8. Piksel aktual (8,16,24) adalah teks Spring, sedangkan oracle
mengharapkan ikon Summer contoh (0,255,0). Ikon sintetis terlalu lebar dan
ber-offset negatif sehingga menabrak label musim sebelumnya. Fixture dipersempit
menjadi delapan piksel; benturan cursor/musim dengan warna berbeda dipertahankan.
Capture kalender sintetis terarah kemudian lulus. Tidak ada perubahan C lagi.
Ulang terakhir `native-title-cli` **lulus, 6,01 detik, CTest exit 0**. Hasil
akhir kedua suite terarah lulus; 17 suite lain tidak diulang karena perbaikan
hanya menyentuh urutan gambar title dan fixture Python. Tidak ada satu run
penuh baru yang diklaim 19/19 setelah koreksi ini.

Tujuh capture title tambahan (nama, konfirmasi Yes/No, kalender musim/hari,
konfirmasi ulang tahun, setup selesai) berhasil, masing-masing exit code 0.
Tampilan diperiksa visual; handoff tetap
`names=1/1/1 birthday=0/0 flags=00002000`. Bukti lokal:
`build/native-validation/phase05-icons-windows-evidence.json`, riwayat diagnosis
di `phase05-icons-failure-evidence.json`, dan capture
`phase05-icons-{name,confirm-name,confirm-no,calendar,day,confirm-birthday,ready}.bmp/png`.
Tidak ada kode C yang diubah setelah tes data asli lulus. Asset/capture tidak
dikomit. Animasi/typing dialog dan perbandingan PS1 tetap belum lulus.

## Baseline UI 77ee140 — tervalidasi Windows Debug

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
kesetaraan dengan PS1. Pada baseline ini, cursor outline, panel konfirmasi/review,
ikon musim, animasi/typing dialog dan timing masih menjadi sisa pekerjaan;
lanjutan cursor/ikon/konfirmasi dijelaskan pada bagian terbaru di atas.

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
| 5.5 | Sebagian | Renderer, audio, input, fade dan animasi memakai tick nominal 30 Hz; tes 30/60/144 FPS lulus. Trig integer SDK lulus empat suite title dan dua capture. | Timing NTSC fraksional dan acuan PS1 terkontrol. |
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
Hasil ini berlaku untuk `77ee140`; perubahan cursor/ikon dan konfirmasi
setelahnya memerlukan hasil build/tes baru.

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
