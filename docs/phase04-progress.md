# Progres fase 04 — Audio dan musik

Diperbarui: 17 Oktober 2026 (Asia/Jakarta).

## Status

**Berjalan. Pemutar uji dan empat suite audio Windows Debug lulus, exit code 0,
berdasarkan hasil terminal pengguna. Seluruh fase belum dinyatakan lulus.**

Asset lokal mencakup `A_FILE.HDT`, `A_FILE.BIN`, dan `SLUS_011.15`, tanpa empat
file XA. Ekspor lokal berhasil: **39 bank VAB v7 dan 36 sequence**. Pemeriksaan
struktur Python terhadap 39 bank tidak menemukan masalah; satu tes unit batas
header/body exporter lulus. Suite data C yang kemudian dijalankan pengguna juga
mem-parsing 39 bank, membandingkan prefix PCM sample pertama setiap bank dengan
decoder Python independen, dan merender satu pasangan bank/sequence pilihan.

## Checklist terhadap rencana

| ID | Status | Perubahan/bukti | Sisa |
|---|---|---|---|
| 4.1 | Inventaris terverifikasi untuk input lokal | 75 record member 9, 78 sound definitions, 26 track XA; nama `STR/01.XA` sampai `STR/04.XA` dari executable tervalidasi hash. | Empat file XA belum tersedia. |
| 4.2 | Lulus pada fixture | Pembacaan sektor utuh 2352/2336 byte, subheader ganda dan filter file/channel diuji. Jalur 2048 byte ditolak. | Validasi media asli; EDC dan pemulihan sektor belum tersedia. |
| 4.3 | Sebagian | Batas VAB, ukuran sample, program/tone, pitch, pan, repeat, ADSR dan flag reverb diaudit; controller sequence ditelusuri. | Pemetaan bank title dan semantik SDK/PS1 yang tepat. |
| 4.4 | Lulus untuk jalur preview teruji | Decoder SPU ADPCM, XA sintetis 4-bit mono/stereo, PCM SDL3 dummy dan WAV lulus. Prefix PCM 39 sample asli cocok dengan decoder independen. | XA asli dan seluruh sample; interpolasi hardware belum tersedia. |
| 4.5 | Sebagian, suite terkait lulus | Sequencer dengan delta/running status, program/note, tempo Sony, volume/pan, bend dan repeat NRPN; chunk/timing diuji dan satu pasangan asli dirender. | Uji repeat NRPN/bend terhadap acuan, reverb, modulasi dan adapter scene; bukan pengganti SPU lengkap. |
| 4.6 | Lulus untuk fixture preview | 24 voice, prioritas/steal, start/stop/release, loop, volume, fade dan stereo tersedia; fixture menguji steal/clipping/pan/release/loop/fade. | Uji prioritas berbeda dan perbandingan penjadwalan/volume/envelope SDK asli. |
| 4.7 | Lulus pada dummy SDL | Producer/main thread melakukan I/O/mix; SDL device thread hanya mengonsumsi PCM. Prebuffer, batas queue dan hitungan underrun; smoke test queue/drain lulus. | Uji dengar dan beban desktop. |
| 4.8 | Lulus untuk CLI teruji | `btn-audio --audio-test`, sample, sequence/XA eksplisit dan ekspor WAV lulus; kontrol desktop tersedia. | Uji dengar dan kontrol interaktif manual. |
| 4.9 | Belum lulus | Format pencatatan perbandingan PS1 dijelaskan di panduan. | Capture PS1 yang cocok dan perbandingan pitch/durasi/loop/fade. |

## Identitas input

| File | SHA-256 |
|---|---|
| A_FILE.HDT | `0cae44d6bdb29e078a182d7cd9a780c21e35dc54f55eb94c2d9ea8deb20c82dd` |
| A_FILE.BIN | `612f9ac8279a364c2fdd19cb75e63df2e892a6f52b6f1d8081ea8560b18decd4` |
| SLUS_011.15 | `34e3b35dcd3fb1cd9a7d4813b8a9ef987915826e3d436f1908ecbe2d4fdf1444` |

Output exporter berada di `build/native-audio/`, diabaikan Git. Tidak ada asset
atau capture audio game yang dimasukkan ke repository. Dokumen rencana asal di
direktori induk tidak ditandai selesai; catatan implementasi repo ada di sini.

## Validasi dan batas

Visual Studio MCP melaporkan tidak ada instance dengan extension aktif. Sesuai
instruksi pengguna, perintah configure/build/CTest diserahkan kepada pengguna;
yang diminta hanya exit code dan baris error relevan, tanpa log penuh.
Suite CTest baru: `native-audio`, `native-audio-cli`, `native-audio-tools`, dan
`native-audio-data` jika direktori data asli dikonfigurasi.

Hasil pengguna pada 17 Oktober 2026:

| Suite | Hasil | Durasi |
|---|---|---|
| native-audio | Lulus | 0,04 detik |
| native-audio-cli | Lulus | 1,89 detik |
| native-audio-tools | Lulus | 0,17 detik |
| native-audio-data | Lulus | 3,66 detik |

**4/4 lulus; total 5,77 detik; CTest exit code 0.** Perintah menggunakan
`-R '^native-audio'`, jadi 11 suite sebelumnya tidak dijalankan ulang pada
validasi ini. Tidak ada log penuh yang dibaca. Exit code configure/build tidak
dilaporkan terpisah; executable baru tersedia dan keempat suite berhasil
dijalankan. Konfigurasi: Windows x64 Debug, generator Visual Studio 18 2026,
SDL 3.2.10; identitas compiler dicatat dari metadata CMake pada bukti lokal.
Baseline sebelum perubahan: `f8d57a3`.

Bukti ringkas dan hash source/executable disimpan lokal dalam
`build/native-validation/phase04-windows-evidence.json`. Compiler:
**MSVC 19.51.36260.0**. Commit implementasi dicatat pada
[Notes fase 04](https://notes.dlands.me/doc/progres-fase-04-audio-dan-musik-lU2FxNWNmA).

Linux ASan/UBSan tetap ditunda mengikuti keputusan pada fase 03; tidak ada
instalasi WSL. Reverb belum diimplementasikan (preview kering dengan diagnosis),
ADSR dan pitch/pan belum identik PS1. Title native belum dihubungkan. Panduan
format, pemakaian dan tes: [native-audio.md](native-audio.md).
