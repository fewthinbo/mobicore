# 📦 mobi-core - Paket Referans Rehberi

## 📋 Genel Bakış

Bu belge, `data_config.json` dosyasındaki `disabled_packets` alanlarında (`to_mt` ve `to_mobile`) kullanılabilecek paket ID'lerini ve açıklamalarını içerir. Bu sayede istenmeyen paketleri devre dışı bırakabilirsiniz.

## 🔧 Kullanım

```json
{
  "settings": {
    "disabled_packets": {
      "to_mt": [3, 7, 8],
      "to_mobile": [4, 5, 25]
    }
  }
}
```

## 📱 Mobil'e Giden Paketler (to_mobile)

Bu paketler köprü sunucudan mobil uygulamaya gönderilir:

### 🎮 MT Oyun Paketleri
| ID | Açıklama |
|---|---|
| 10 | Karakter bilgileri |
| 11 | Oyuncu çevrimiçi/çevrimdışı durumu |
| 12 | Oyun içi genel mesajlar |
| 13 | Yönetici duyuruları |
| 14 | MT çevrimiçi sayacı |
| 15 | Oyuncu mob öldürme |

### 💬 Özel Mesajlaşma (PM)
| ID | Açıklama |
|---|---|
| 23 | MT'den özel mesaj |
| 24 | Özel mesaj odası silindi |
| 25 | Özel mesaj odası bilgileri |

### 📨 Messenger Sistemi
| ID | Açıklama |
|---|---|
| 32 | Mobil'den mobil'e mesaj |
| 33 | Messenger odası bilgileri |
| 34 | Messenger odası silindi |
| 35 | Odaya üye eklendi |
| 36 | Üye odadan ayrıldı |

### 🏛️ Lonca Sistemi
| ID | Açıklama |
|---|---|
| 39 | Lonca bilgileri |
| 40 | Lonca savaş listesi |
| 41 | Lonca savaşı başladı |
| 42 | Lonca savaşı bitti |
| 43 | Lonca savaş öldürme |
| 44 | Lonca savaşına katıldı |
| 45 | Lonca savaşından ayrıldı |
| 46 | Lonca savaş pozisyon güncelleme |
| 47 | Lonca savaş bildirimi |

## 📤 MT'ye Giden Paketler (to_mt)

Bu paketler köprü sunucudan MT sunucusuna gönderilir:

| ID | Açıklama |
|---|---|
| 3 | Mobil'den oyuna mesaj |
| 7 | Mobil kullanıcı giriş bildirimi |
| 8 | Mobil kullanıcı çıkış bildirimi |

> ⚠️**Not**: Sistem bütünlüğü için hiçbir paketi devre dışı bırakmamanız daha iyi olacaktır.

## 🛠️ Sorun Giderme

Bir paketi devre dışı bıraktıktan sonra sorun yaşarsanız:

1. **data_config.json** dosyasından ilgili paket ID'sini kaldırın
2. **5 dakika** bekleyin (sistem otomatik olarak yeniden okur)

---

## 📖 Navigasyon

**📚 Dokümantasyon**: [← Önceki](./tech_tr.md) | [← Ana Sayfaya Dön](../tr.md) | **Sonraki →** [Kurulum Rehberi](./this_tr.md)

**🌐 Dil**: **Türkçe** | [English](./packets_en.md)

---

*mobi-core ile mobil sunucunuzun tam kontrolü!* 