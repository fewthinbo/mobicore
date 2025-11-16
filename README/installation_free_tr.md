**🌐 Dil**: **Türkçe (seçildi)** | [English](./installation_free_en.md) 
# 🔧 mobi-core - Ücretsiz Plan Kurulumu
- [Formu](https://forms.gle/EFNjrpuXF558PkKL8) doldurun, ücretsiz plan için kaydınız yapılacaktır.
- Ücretsiz plandaki tek kısıtlama hesap sayısı limitidir (şimdilik 130).

| **Gereklilikler**                                 |
|--------------------------------------------------|
| FreeBSD 14.0 amd64 VDS                           |
| Alan adı, örneğin: mobicore-test.com     |

## Teknik Kurulum
> 🥳 **Rahatlayın**: Hatasız bir kurulum sizi bekliyor. Tahmini kurulum süresi: **6 dakika**

### Alan adınızı ayarlayalım
**⏱️ Tahmini süre:** 1 dakika
- Alan adını satın aldığınız siteye girin(örn: GoDaddy) ve alan adı ayarlarınızı bulun.
- Alan adınızın adres 'A' kaydını yeni satın aldığınız vds'inizin ip'si ile değiştirin.

### VDS sunucunuzu ayarlayalım
**⏱️ Tahmini süre:** 5 dakika
- VDS sunucunuzu açın.
- VDS'inizde **/usr/mobile/** klasörünü oluşturun.
- Move files from **source folder** to **target folder**.
	| **Kaynak klasör (İndirdiğiniz mobicore projesi)**            | **Hangi dosyalar?**                                                            | **Hedef klasör (VDS sunucunuz)** |
	|--------------------------------------------------------------|--------------------------------------------------------------------------------|-------------------------------------|
	| Ana Dizin/IMPL/free_plan/your_freebsd_vds/usr.mobile/        | Tüm dosyalar                                                                   | /usr/mobile/                        |
	| [Tıkla](https://github.com/fewthinbo/mobicore/releases) | En son yayınlanan App_Bridge sürümü -mt sunucunuza uygun olanı seçin-          | /usr/mobile/bridge/                    |

- VDS'inizde aşağıdaki komutu çalıştırın:
	```bash
	cd /usr/mobile/scripts && chmod +x setup-bridge.sh && sed -i '' 's/\r$//' setup-bridge.sh && sh setup-bridge.sh
	```
	Birkaç soru sorulacak ve tüm kurulum otomatik olarak tamamlanacaktır.

### 🎉 Artık hazırsınız, ara sunucunuzu başlatın.
- Son bir adım kaldı [buraya tıkla](./installation_tr.md#teknik-kurulum)
- Son adımı tamamladıktan sonra oyuncularınız kendi oyun içi hesaplarıyla mobilden giriş yapabilecekler.

## Destek
Sorularınız için: **mobicore.io@gmail.com**

*mobi-core ile mobil sunucunuzun tam kontrolü!* 
