# Progres fase 05 — Layar judul native

Diperbarui: 17 Oktober 2026 (Asia/Jakarta). Baseline: `5645d68`.

## Status

**Berjalan. Preview native cabang Start/menu tervalidasi Windows Debug:
19/19 suite CTest lulus, berdasarkan hasil terminal pengguna. Fase 05 belum
lulus karena jalur title lengkap dan perbandingan PS1 masih belum selesai.**

Source `titleStepDispatch` sebelumnya hanya deklarasi/alamat. Audit lokal
menemukan member 27 (`GF_TITLE.Bin`), memetakan dispatch, aset umum dan musik.
Rekonstruksi C saat ini mencakup prompt Start, dua pilihan dengan default
Continue, konfirmasi yang menjelaskan batas fitur, serta kontrol native untuk
kembali. Tidak ada byte MIPS yang dijalankan sebagai fungsi host.

## Checklist terhadap rencana

| ID | Status | Bukti/perubahan | Sisa |
|---|---|---|---|
| 5.1 | Sebagian | Tool audit memverifikasi executable US, overlay, delapan handler, 0E/0D, dan asal pasangan musik. Cabang menu direkonstruksi dalam C. | Body intro/varian latar, attract mode dan name-entry belum direkonstruksi lengkap. |
| 5.2 | Lulus untuk startup preview teruji | Startup SDL, state host, loader berbatas, tanpa CRT0/BIOS; suite sintetis dan data lokal lulus. | Integrasi global yang diperlukan jalur title lengkap. |
| 5.3 | Lulus untuk cabang preview teruji | `btn-native --title` memanggil fungsi C `btn_title_tick`/`btn_title_draw`; byte overlay hanya diperiksa sebagai data identitas struktur. | Dispatch title lengkap dan gerbang transisi gameplay. |
| 5.4 | Lulus untuk reset preview teruji | Resource terpisah dari `BtnTitleState`; tes reset dan replay kembali ke prompt/menu lulus tanpa pointer handler PS1. | Transisi scene penuh ke gameplay/save-load. |
| 5.5 | Sebagian | Layanan renderer, audio, input dan nominal 30 Hz memakai API host; tone SFX eksplisit. | Callback/fade/transisi title penuh, timing NTSC fraksional dan acuan PS1. |
| 5.6 | Sebagian | Sprite logo, copyright, Start, New Game, Continue, cursor berkedip; bank 53 + sequence 54 dan SFX 0/5 terhubung. | Intro, animasi latar/aktor, nama, fade dan kesetaraan audio. |
| 5.7 | Lulus untuk jalur fixture dan data lokal | Tes state, replay deterministik, oracle piksel independen, serta lifecycle SDL dummy lulus; seluruh 19 suite lulus. | Uji desktop/dengar, urutan dan timing terhadap PS1. |
| 5.8 | Sebagian, diagnosis teruji | Log dan panel menjelaskan New Game/Continue yang belum tersedia; tes memeriksa status tersebut. | Verifikasi panel desktop; kedua tombol tujuan tetap belum berfungsi. |

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

Hasil terminal pengguna, 17 Oktober 2026: **19/19 lulus, 0 gagal, total 15,81
detik**. Empat suite title (`native-title`, `native-title-cli`,
`native-title-tools`, `native-title-data`) semuanya lulus. Kelima belas suite
sebelumnya ikut dijalankan dan lulus, termasuk runtime/input, archive, grafik
dan audio. Tidak ada kode C yang diubah setelah laporan tes ini.

| Kelompok | Suite lulus | Jalur |
|---|---:|---|
| Runtime/input | 3 | Core, event adapter, CLI sintetis |
| Arsip/data tools | 5 | Parser, rentang/async, CLI, tools, input lokal |
| Grafik | 3 | Renderer, CLI dan decoder independen data lokal |
| Audio | 4 | Decoder/mixer, tone eksplisit, CLI/tools dan data lokal |
| Title | 4 | State/clock, CLI/replay/oracle, audit tools dan data lokal |

Exit code configure/build/CTest tidak dilaporkan terpisah. Laporan CTest
menyatakan 100% lulus dan executable baru berhasil dijalankan. Smoke capture
lokal tambahan dengan `--title --headless --frames 2` berhasil, exit code 0;
raw framebuffer title diperiksa visual, tanpa membuka window atau audio device.
Ini tidak menggantikan uji desktop/controller atau uji dengar.

Konfigurasi: Windows x64 Debug, generator Visual Studio 18 2026, **MSVC
19.51.36260.0** (metadata compiler CMake), **SDL 3.2.10** (output executable).
Bukti ringkas, hasil suite, hash source dan binary disimpan lokal di
`build/native-validation/phase05-windows-evidence.json`. Capture lokal:
`build/native-validation/phase05-title-native.bmp`. Asset/capture tidak dikomit.

Linux ASan/UBSan dan capture PS1 belum diuji. Rencana asal di direktori induk
tidak ditandai selesai. Background masih warna rata; intro, attract mode,
animasi latar/aktor, fade, input nama dan transisi gameplay belum selesai.

Commit implementasi dicatat di
[Notes fase 05](https://notes.dlands.me/doc/progres-fase-05-layar-judul-native-BwWOSiJ5r9)
setelah commit dibuat; commit ini disimpan lokal, belum dipush.
Panduan pemakaian, format replay dan keterbatasan: [native-title.md](native-title.md).
