## Sistem Gereksinimleri 

- CMake 3.20 veya üzeri
- Clang/LLVM veya gcc
- GNU Make veya Ninja
- Derleme için c++17 standardı

## Notlar 📝

**Ayarlar**:
- ~Başlamadan önce CMakeLists.txt dosyasındaki MT_DIRS değişkeninin ayarlandığından emin olun~.
- Bu proje FreeBSD'de bazı sistem kütüphanelerini kullanıyor.
- Bu projenin bağımlılıklarını kontrol edin.
- Eğer MT'nizde aynı kütüphaneleri özel bir yolda kullanıyorsanız, bunları silin ve sistem kütüphanelerini kullanın.

⚠️ **ABI Uyumluluğu**: 
- Bu projenin ve MT projesinin derleyici sürümü aynı olmalıdır.
- Eğer belirli bir clang sürümü kullanıyorsanız, tam yolu CMakeLists.txt dosyasına yazmalısınız:
  ```cmake
  set(CMAKE_C_COMPILER "/specific/clang/path/clang")
  set(CMAKE_CXX_COMPILER "/specific/clang/path/clang++")
  ```
- cmakepresets.json dosyasındaki tüm preset'ler rsync & vs -uzak kurulum- ile kullanılabilir.

**Rehber**:
- Bu repo'yu FreeBSD'nize indirin.
- FreeBSD ortamını ayarlamak için IMPL/bsd/scripts/setup-freebsd.sh dosyasını çalıştırın.
- Uygulama rehberi için IMPL/mt klasörünü kullanın.

## Derleme & Kurulum

İki farklı generator kullanabilirsiniz:

### Ninja generator ile (Tercih edilen):
```bash
cmake --preset bsd-ninja-release
mobi_install_rel
```

Alias'ı henüz eklemediyseniz bu şekilde de yapabilirsiniz.
```bash
cmake --preset bsd-ninja-release
cd out/build/bsd-ninja-release
ninja
ninja install
```

### Unix Makefiles generator ile:
```bash
cmake --preset bsd-gmake-release
cd out/build/bsd-gmake-release
make
make install
```


## Bu kütüphaneyi kullanma

Derleme & kurulum sonrası:
- [SQL kullanıcı ayarlama](./tech_tr.md) belgesini okuyun.
- MT'nizi derleyin ve çalıştırın.
- Eğer bridge server'ınız (mobileServer) başka bir uzak VDS'de çalışıyorsa ve lisansınız varsa, MT'niz otomatik olarak bridgeServer'a bağlanır.
- Her şey otomatik olarak hazır olacaktır.
- Bu kütüphanenin logları her MT-core dosyanızda ('logs/') bulunur; log sistemi ayrıca otomatik yedekleme sistemine sahiptir ('/backups').
- Oyuncularınız App/Play Store'dan mobileApp kullanarak MT hesaplarına giriş yapabilirler.

---

## 📖 Navigasyon

**📚 Dokümantasyon**: [← Önceki](./packets_tr.md) | [← Ana Sayfaya Dön](../tr.md)

**🌐 Dil**: **Türkçe** | [English](./this_en.md)

---

*mobi-core için eksiksiz kurulum ve ayarlama rehberi!* 