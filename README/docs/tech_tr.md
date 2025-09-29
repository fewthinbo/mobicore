# 🔧 mobi-core - Teknik Yönetim Rehberi

## 📋 Genel Bakış

mobi-core sistemi, MT sunucunuzdaki JSON dosyaları üzerinden köprü sunucuyu yönetmenizi sağlar. Tüm ayarlar gerçek zamanlı olarak güncellenir ve sistem performansı için optimize edilir.

## 🗄️ SQL Veritabanı Yapılandırması

### 🔐 Güvenlik Önceliği
mobi-core, **sadece SELECT yetkisi** olan özel bir SQL kullanıcısı gerektirir. Bu şunları sağlar:
- ✅ Verileriniz tamamen güvende kalır
- ✅ Köprü sunucu yazma/değiştirme izni alamaz
- ✅ Sadece gerekli bilgiler okunabilir
- ✅ Kullanıcı şifreleri okunmaz (Yetki dahilinde değil).

### 📊 Performans Avantajı
Yeni karakterler, hesaplar veya loncalar oluşturulduğunda:
- ✅ MT sunucusundan köprü sunucuya veri aktarımı olmaz
- ✅ Network trafiği minimum seviyede tutulur
- ✅ Yeniden bağlanma durumlarında lag olmadan hızlıca senkronize olur.

### 🛠️ SQL Kullanıcı Kurulumu
- 'account', 'player' ve 'common' veritabanlarından sadece belirli kolonlardaki verileri okuyabilen kullanıcı oluştur.
- 'RANDOM_STRONG_PASS' uzunluğu PASSWORD_MAX_LENGTH(32)'den az olmalıdır.
```sql
CREATE USER 'mobicore'@'YOUR_MOBI_BRIDGE_SERVER_IP' IDENTIFIED BY 'RANDOM_STRONG_PASS';
GRANT SELECT (id, login, empire, email) ON account.account* TO 'mobicore'@'YOUR_MOBI_BRIDGE_SERVER_IP';
GRANT SELECT (mID, mAuthority) ON common.gm_list* TO 'mobicore'@'YOUR_MOBI_BRIDGE_SERVER_IP';
GRANT SELECT ON player.player* TO 'mobicore'@'YOUR_MOBI_BRIDGE_SERVER_IP';
GRANT SELECT ON player.guild* TO 'mobicore'@'YOUR_MOBI_BRIDGE_SERVER_IP';
GRANT SELECT ON player.guild_member* TO 'mobicore'@'YOUR_MOBI_BRIDGE_SERVER_IP';
GRANT SELECT ON player.messenger_list* TO 'mobicore'@'YOUR_MOBI_BRIDGE_SERVER_IP';
FLUSH PRIVILEGES;
```
- "/usr/mobile/info.json" dosyasına bilgileri koyun.

> ⚠️ **Önemli**: Asla tam yetkili SQL kullanıcısı oluşturmayın!

## 📁 Yapılandırma Dosyaları

### 📄 data_config.json
**Konum**: MT sunucunuzun çalıştığı VDS üzerinde  
**Okunma Sıklığı**: Her 5 dakikada bir


#### ⚙️ Sistem Ayarları
```json
{
  "settings": {
    "mobile": {
      "security": {
        "max_active_sessions": 1000000,
        "ip_blocklist": ["***.**.**.***"],
        "heartbeat": {
          "max_missed_heartbeats": 5
        },
        "login_policy": {
          "max_attempts": 5,
          "block_duration_minutes": 0
        },
        "rate_limit": {
          "per_second": 20,
          "max_overflows": 3,
          "block_duration_minutes": 3
        }
      },
      "online_counter": {
        "enabled": true,
        "refresh_interval_seconds": 5
      },
      "maintenance_mode": {
        "enabled": false,
        "message": "Şu anda bakım yapılmaktadır."
      }
    },
    "disabled_packets": {
      "to_mt": [],
      "to_mobile": []
    }
  }
}
```

### 📢 data_notification.json
**Konum**: MT sunucunuzun çalıştığı VDS üzerinde  
**Okunma Sıklığı**: Her 5 dakikada bir  
**Görüntüleme**: Mobil uygulamada ana sayfa

```json
{
  "notifications": [
    {
      "title": "Yeni Etkinlik Başladı!",
      "subtitle": "2x EXP Etkinliği",
      "details": "Bu hafta sonu boyunca çifte deneyim kazanın!",
      "admin_name": "OyunMaster"
    },
    {
      "title": "Sunucu Güncellendi",
      "subtitle": "Yeni özellikler eklendi",
      "details": "Detaylar için oyuna giriş yapın...",
      "admin_name": "Yönetici"
    }
  ]
}
```

## Core Yetkilendirme Sistemi

### 🎯 Akıllı Yönetim
- **Tek Core Okuma**: Tüm core'larınıza farklı yetkiler atanır
- **Yetki Dağılımı**: Her core kendi yetkisi dahilindeki görevleri üstlenir

### 🔄 Güncelleme Süreci
1. **JSON Değişikliği**: Dosyayı düzenleyin
2. **Otomatik Algılama**: Sistem 5 dakika içinde okur
3. **Anında Güncelleme**: Tüm ayarlar devreye girer
4. **Mobil Yansıma**: Değişiklikler mobilde görünür

## 🎛️ Yönetilebilir Ayarlar

### 🔒 Güvenlik Kontrolleri
- **Maksimum Bağlantı Sayısı**: Sunucu kapasitesini korur
- **IP Engelleme**: İstenmeyen IP'leri engeller
- **Heartbeat Kontrolü**: Pasif bağlantıları temizler
- **Giriş Politikası**: Brute force saldırılarını engeller
- **Hız Sınırı**: Spam koruması
  > ⚠️ **Önemli**: Varsayılan ayarlar önerilir!

### 📊 Performans Ayarları
- **Online Sayacı**: Etkinleştir/devre dışı ve yenileme sıklığı
- **Bakım Modu**: Anında bakım için
- **Paket Filtresi**: İstenmeyen paketleri devre dışı bırak

## 💡 Pratik Kullanım Örnekleri

### 🎮 Etkinlik Duyurusu
```json
{
  "title": "Şans Kutusu Etkinliği",
  "subtitle": "3 Gün Boyunca!",
  "details": "Bu hafta sonu şans kutularından 2x ödül alın!",
  "admin_name": "EtkinlikYöneticisi"
}
```

### 🛠️ Bakım Modu
```json
{
  "maintenance_mode": {
    "enabled": true,
    "message": "Sunucu güncellemesi yapılıyor. 30 dakika sonra tekrar deneyin."
  }
}
```

### 🔒 Güvenlik Artırma
```json
{
  "security": {
    "max_active_sessions": 5000,
    "rate_limit": {
      "per_second": 10,
      "max_overflows": 1
    }
  }
}
```

## 🔧 Kurulum ve Ayarlama

0. **Lisans**: Lisansınızı almak için aşağıdaki mail adresinden iletişime geçin
1. **MT Uygulaması**: IMPL klasörünü okuyun ve kaynak dosyalarınıza gerekli eklemeleri yaptığınızdan emin olun
2. **SQL Kullanıcısı**: Sadece SELECT yetkisi olan kullanıcı oluşturun
3. **JSON Dosyaları**: VDS'nizde usr/mobile/ klasörüne yerleştirin
4. **Ayarları Yapılandırın**: data_config.json'a SQL bilgilerinizi girin
5. **Sunucunuzu Başlatın**: Her şey otomatik olarak hazır olacak
	> ✅**Not**: Hosting firmalarında altyapı kaynaklı sorunlar nedeniyle internet bağlantı kaybı, sunucu kapanması gibi hoş olmayan durumlar konusunda endişelenmeyin, bağlantılar otomatik olarak yeniden kurulacaktır.

## 📞 Destek

Sorularınız için: **mobicore.io@gmail.com**

---

## 📖 Navigasyon

**📚 Dokümantasyon**: [← Önceki](./logic_tr.md) | [← Ana Sayfaya Dön](../tr.md) | **Sonraki →** [Paket Referansı](./packets_tr.md)

**🌐 Dil**: **Türkçe** | [English](./tech_en.md)

---

*mobi-core ile mobil sunucunuzun tam kontrolü!* 