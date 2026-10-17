# Acuan video fase 05

Diperiksa: 17 Oktober 2026 (Asia/Jakarta). Baseline native: `d9bc562`.

Pengguna memberikan video **Harvest Moon: Back To Nature Part 1 of 7 -
Walkthrough - No Commentary**, kanal BackGroundGaming Longplays. Pemutar
menampilkan durasi 11:45:24. Bagian awal diperiksa langsung melalui browser;
video tidak diunduh atau dimasukkan ke repo. Teks game yang terlihat berbahasa
Inggris; hash disc, emulator, versi executable dan input rekamannya tidak
diketahui. Karena itu video menjadi acuan observasi, bukan golden piksel/tick.

## Temuan langsung

| Waktu video | Terlihat | Perbandingan baseline native |
|---|---|---|
| [0:46](https://www.youtube.com/watch?v=UGe_1MLec9E&t=46s) | Logo title, aktor dan copyright; prompt masuk dari kanan. | Lapisan dan animasi terkait tersedia. Kesamaan setiap frame belum diukur. |
| [0:56](https://www.youtube.com/watch?v=UGe_1MLec9E&t=56s) | Pilihan New Game/Continue dan tangan pada New Game. | Kedua pilihan tersedia; tujuan Continue dibatasi fase 07. |
| [1:06](https://www.youtube.com/watch?v=UGe_1MLec9E&t=66s) | Portrait wali kota dan dialog pembukaan pertanyaan. | Belum tersedia; native langsung memasuki grid nama. |
| [1:16](https://www.youtube.com/watch?v=UGe_1MLec9E&t=76s) | Panel pertanyaan nama di kiri atas, grid dan petunjuk tombol PS1. | Panel pertanyaan belum tersedia. Native memakai petunjuk keyboard. |
| [1:36](https://www.youtube.com/watch?v=UGe_1MLec9E&t=96s) | Pertanyaan ulang tahun dan konfirmasi Yes/No. | Dialog konfirmasi tersedia; panel pertanyaan hilang dan format header hari berbeda. |
| [1:46](https://www.youtube.com/watch?v=UGe_1MLec9E&t=106s) | Pertanyaan nama farm, nama di kiri dan suffix Farm di kanan. | Urutan/header dasar tersedia; panel pertanyaan hilang. |
| [1:56](https://www.youtube.com/watch?v=UGe_1MLec9E&t=116s) | Portrait wali kota dan teks yang sedang muncul sebagian. | Dialog antarpertanyaan, portrait dan reveal teks belum tersedia. |
| [2:06](https://www.youtube.com/watch?v=UGe_1MLec9E&t=126s) | Pertanyaan nama anjing dan grid. | Grid tersedia; panel pertanyaan hilang. |
| [2:26](https://www.youtube.com/watch?v=UGe_1MLec9E&t=146s) | Layar Now Loading dengan sapi. | Native berhenti pada data handoff; gameplay berada di fase 06. |

Timestamp menunjukkan frame yang diamati saat seek, bukan waktu mulai/akhir
transisi. Tidak ada pengukuran durasi animasi, latensi tombol, fade atau loop
musik. Audio video belum diperiksa. Jangan menyimpulkan kesetaraan dari
kesamaan susunan menu atau dari tes decoder native.

## Perbaikan panel pertanyaan

Status: empat suite title lulus pada Windows Debug, CTest exit 0, 60,15 detik.
Exit code build tidak dilaporkan terpisah. Hasil terarah: `native-title` 0,03
detik, `native-title-cli` 18,78, `native-title-tools` 0,22, `native-title-data`
41,12. Lima belas suite lain tidak diulang; baseline `d9bc562` tetap 19/19
dalam 60,02 detik. Tidak ada kode C yang diubah setelah laporan tes terbaru.

Audit data lokal menemukan fungsi overlay `0x800BA130`: window `(16,16,120,20)`
dan posisi teks `(20,20)`. Fungsi memakai tabel `0x800C4A50`, empat offset relatif
ke textBank `0x8005175C`. Hasilnya alamat `0x80051A94`, `0x80051AAA`,
`0x80051AC8`, `0x80051AE0`; kapasitas termasuk END masing-masing 11,15,12,11
word, cocok dengan `src/text/text_bank.layout.json`.

Native membaca empat stream berbatas dari executable pengguna dan memilihnya
sesuai tahap pemain/ulang tahun/farm/anjing, termasuk ketika konfirmasi aktif.
Panel disembunyikan pada review/handoff native. Tidak ada teks game atau
gambar video yang ditanam ke source. Tes piksel membaca glyph/palet secara
independen untuk keempat tahap; fixture sintetis memakai isi yang berbeda.
Tes input rusak memeriksa kontrol yang tidak didukung dan END yang hilang.

Empat capture tambahan berhasil, masing-masing exit 0: pemain 86 frame,
ulang tahun 154, farm 234 dan anjing 308. Panel dan teks keempatnya diperiksa
visual. Bukti lokal: `build/native-validation/phase05-questions-windows-evidence.json`,
`phase05-questions-audit.json` dan `phase05-questions-{player,birthday,farm,dog}.bmp/png`.
Asset/capture tetap lokal dan tidak dikomit. Platform Windows x64 Debug,
Visual Studio 18 2026, MSVC 19.51.36260.0, SDL 3.2.10.

Lanjutan dialog wali kota/portrait/reveal teks telah divalidasi: empat suite
title lulus (227,58 detik, CTest exit 0), lalu koreksi indikator lulus rebuild
(exit 0) dan dua capture terarah. Lihat
[native-title.md](native-title.md#mayor-conversation-windows-debug-validated).
Format header ulang tahun dan review akhir lulus empat suite title (256,64 detik,
CTest exit 0), ditambah tiga capture data asli untuk kedua pilihan dan retry;
lihat [native-title.md](native-title.md#birthday-header-and-final-review-windows-debug-validated).
Gerbang 5.7 memerlukan
urutan input yang tercatat serta pengukuran pada acuan yang terkontrol.
