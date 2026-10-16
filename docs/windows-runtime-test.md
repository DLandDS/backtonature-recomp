# Uji runtime SDL3 di Windows

Paket ini berisi source runtime sintetis fase 1. Belum ada gameplay BTN, audio,
atau save/load game. Asset game dan SDK PS1 tidak diperlukan.

## 1. Siapkan dan buka source

Pasang Visual Studio 2022 atau Build Tools 2022 dengan workload **Desktop
development with C++**, termasuk MSVC dan Windows SDK. Pastikan CMake 3.21+
dan Python 3.10+ tersedia di PATH.

Ekstrak `btn-runtime-windows-source.zip`. Buka **Developer PowerShell for
VS 2022** di folder `btn-runtime-source` yang berisi `CMakeLists.txt`.

ZIP ini berisi file yang diperlukan untuk target native dan pengujiannya.
Build matching PS1 serta vendored agent skills tidak disertakan. SDL 3.2.10
diunduh saat configure pertama; koneksi HTTPS ke `codeload.github.com` diperlukan.
Simpan file `SOURCE-MANIFEST.json` untuk mengidentifikasi source yang diuji.

## 2. Build dan jalankan tes otomatis

Jalankan satu per satu. Jika sebuah perintah gagal, berhenti pada error itu.
`Start-Transcript` mencatat output terminal selama sesi ini.

```powershell
New-Item -ItemType Directory -Force build/windows-report | Out-Null
Start-Transcript -Path build/windows-report/session.txt
cmake --version
python --version
cmake -S . -B build/native-windows -G "Visual Studio 17 2022" -A x64 -DBTN_BUILD_NATIVE=ON -DBTN_BUILD_MATCHING=OFF
cmake --build build/native-windows --config Debug --parallel 4
ctest --test-dir build/native-windows -C Debug --output-on-failure
```

Hasil yang diharapkan: build selesai dan CTest melaporkan **3/3 tes lulus**:
`native-core`, `native-input`, dan `native-sdl`. Tes otomatis menggunakan dummy
video dan controller virtual. Tes desktop/perangkat fisik di bawah tetap perlu.

## 3. Uji desktop dan input

Jalankan:

```powershell
.\build\native-windows\native\Debug\btn-native.exe --runtime-test --save-dir build/windows-report/saves
```

Yang muncul: pola kotak 320x240 yang diperbesar, kotak hijau yang bisa digerakkan,
angka tick, dan mask input. Belum ada grafik atau suara dari game.

| Uji | Hasil yang diharapkan |
|---|---|
| Buka/tutup | Window muncul; tombol X dan Escape menutup aplikasi tanpa crash. |
| Resize dan F11 | Pola tetap proporsional dengan letterbox; fullscreen bisa masuk/keluar. |
| Keyboard | Panah menggerakkan kotak. Z mengubah kotak menjadi kuning; X dan Return mengubah mask input. Setelah dilepas, input kembali nol. |
| Fokus/minimize | Tahan panah, Alt+Tab, lalu lepas tombol di luar aplikasi. Saat kembali, kotak tidak bergerak sendiri atau mengejar waktu selama ditinggalkan. Ulangi dengan minimize/restore. |
| Controller | D-pad/left stick menggerakkan kotak; tombol south/east/Start mengubah input. Lepas controller saat arah ditahan: input controller harus bersih. Sambungkan lagi dan periksa input kembali bekerja. Jika tidak ada controller, catat belum diuji. |

Tutup aplikasi sebelum mencoba konfigurasi berikut. Ulangi pengamatan gerak pada
30 dan 144 FPS; frekuensi tick sintetis tetap sekitar 60 per detik selama window
aktif. Jangan membandingkan jumlah tick setelah durasi bermain yang berbeda.

```powershell
.\build\native-windows\native\Debug\btn-native.exe --runtime-test --fps 30
.\build\native-windows\native\Debug\btn-native.exe --runtime-test --fps 144
.\build\native-windows\native\Debug\btn-native.exe --runtime-test --bind up=W --bind down=S --bind left=A --bind right=D
.\build\native-windows\native\Debug\btn-native.exe --runtime-test --data-dir .\folder-data-tidak-ada
```

WASD harus menggantikan tombol panah pada contoh remapping. Lokasi data yang
tidak ada harus menghasilkan pesan `Required data file missing or empty` dan
keluar dengan status nonzero. Pengujian determinisme numerik FPS sudah berada
di `native-core`; uji manual memeriksa perilaku presentasi nyata.

## 4. Simpan laporan

Setelah semua aplikasi uji ditutup:

```powershell
Copy-Item build/native-windows/Testing/Temporary/LastTest.log build/windows-report/ctest-last.txt
Copy-Item SOURCE-MANIFEST.json build/windows-report/source-manifest.json
Stop-Transcript
```

Jika build/tes gagal sebelum `LastTest.log` dibuat, cukup simpan `session.txt`
dan baris error pertama. Pada checkout repository tanpa paket ZIP, file
`SOURCE-MANIFEST.json` tidak ada; gunakan identitas commit dan diff yang diuji.

Isi laporan singkat ini; jangan tandai pengujian yang tidak dilakukan sebagai
lulus:

```text
Windows (versi/build):
GPU dan nama controller (jika ada):
Build: lulus / gagal
CTest: .../3 lulus
Window, resize, fullscreen: lulus / gagal / belum diuji
Keyboard, fokus, minimize, remapping: lulus / gagal / belum diuji
Controller dan lepas/sambung: lulus / gagal / belum diuji
Presentasi 30/144 FPS dan pesan lokasi data: lulus / gagal / belum diuji
Error pertama atau perilaku yang berbeda:
```

Bukti ini dapat menutup verifikasi Windows serta perangkat yang benar-benar
diuji. Pengukuran timing BTN asli, matching PS1, dan verifikasi Linux desktop
tetap merupakan pemeriksaan terpisah.
