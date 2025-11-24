**🌐 Dil**: **Türkçe (seçildi)** | [English](./installation_en.md)
# 🔧 mobi-core - Kurulum Rehberi
- [Formu](https://forms.gle/bPyfbgr4cestykzFA) doldurduktan kendi oyun sunucunuza uygun, ara sunucuda çalıştırılabilir bir dosya alacaksınız.
- Sistem hazır olduğunda bir e-mail alacaksınız ardından [teknik kurulum](#teknik-kurulum)'a geçebilirsiniz..

## Ücretsiz Plan Kurulumu
- Ücretsiz plan kurulumu için önce [buraya tıklayın](./installation_free_tr.md)

## Teknik Kurulum
> 🥳 **Rahatlayın**: Hatasız bir kurulum sizi bekliyor. Tahmini kurulum süresi: **12 dakika**
### Gereklilikler
- Oyun sunucunuzu derlediğiniz FreeBSD VDS (Önerilen Sürüm: 14.0)

### VDS sunucunuzu ayarlayalım
**⏱️ Tahmini süre:** 2 dakika
> ⚠️ **Not**: Kurulum sırasında mt2 sunucunuzun kapalı olması tavsiye edilir.
- VDS'inizi açın.
- VDS'inizde **/usr/mobile/** klasörünü oluşturun.
- **Kaynak klasör**'deki dosyaları **hedef klasör**'e taşıyın.
	| **Kaynak klasör (İndirdiğiniz mobicore projesi)** | **Hangi dosyalar?**                       | **Hedef klasör (VDS sunucunuz)** |
	|---------------------------------------------------|-------------------------------------------|----------------------------------|
	| Ana Dizin/IMPL/your_freebsd_vds/usr.mobile/       | Tüm dosyalar                              | /usr/mobile/                     |
	| Ana Dizin/                                        | Source, CMakeLists.txt, CMakePresets.json | /usr/mobile/src/                 |

- VDS'inizde aşağıdaki komutu çalıştırın:
	```bash
	cd /usr/mobile/scripts && chmod +x setup-mobi.sh && sed -i '' 's/\r$//' setup-mobi.sh && sh setup-mobi.sh
	```
	Birkaç soru sorulacak ve tüm kurulum otomatik olarak tamamlanacaktır.
	> ✅ **Not**: Oluşturulan SQL kullanıcısı şifre gibi gizli kolonları okumaya yetkili değildir, güvenle kullanabilirsiniz.

### Oyununuza eklemeler
**⏱️ Tahmini süre:** 10 dakika
- İndirdiğiniz mobicore projesinin ana dizininde IMPL/mt/ klasörünü açın.
- İçerisindeki tüm eklemeleri kendi oyun projenize yapın.
- Tüm oyun projenizi yeniden derleyin ve VDS sunucunuza atın.
### 🎉 Artık hazırsınız, sunucunuzu çalıştırın
- Oyun sunucunuzu başlatabilirsiniz, her şey otomatik olarak hazır olacak.
- Oyuncularınız mobilden girişlerini kendi oyun içi hesaplarıyla yapabilirler.
	> ✅ **Not**: Hosting firmalarında altyapı kaynaklı sorunlar nedeniyle internet bağlantı kaybı, sunucu kapanması gibi hoş olmayan durumlar konusunda endişelenmeyin, bağlantılar otomatik olarak yeniden kurulacaktır.

## Destek
Sorularınız için: **mobicore.io@gmail.com**

*mobi-core ile mobil sunucunuzun tam kontrolü!* 