# 📦 mobi-core - Packet Reference Guide

## 📋 Overview

This document contains packet IDs and descriptions that can be used in the `disabled_packets` fields (`to_mt` and `to_mobile`) in the `data_config.json` file. This allows you to disable unwanted packets.

## 🔧 Usage

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

## 📱 Packets to Mobile (to_mobile)

These packets are sent from bridge server to mobile application:

### 🎮 MT Game Packets
| ID | Description |
|---|---|
| 10 | Character information |
| 11 | Player online/offline status |
| 12 | In-game general messages |
| 13 | Admin announcements |
| 14 | MT online counter |
| 15 | Player mob kill |

### 💬 Private Messaging (PM)
| ID | Description |
|---|---|
| 23 | Private message from MT |
| 24 | Private message room deleted |
| 25 | Private message room information |

### 📨 Messenger System
| ID | Description |
|---|---|
| 32 | Mobile to mobile message |
| 33 | Messenger room information |
| 34 | Messenger room deleted |
| 35 | Member added to room |
| 36 | Member left room |

### 🏛️ Guild System
| ID | Description |
|---|---|
| 39 | Guild information |
| 40 | Guild war list |
| 41 | Guild war started |
| 42 | Guild war ended |
| 43 | Guild war kill |
| 44 | Joined guild war |
| 45 | Left guild war |
| 46 | Guild war position update |
| 47 | Guild war notification |

## 📤 Packets to MT (to_mt)

These packets are sent from bridge server to MT server:

| ID | Description |
|---|---|
| 3 | Mobile to game message |
| 7 | Mobile user login notification |
| 8 | Mobile user logout notification |

> ⚠️**Note**: For system integrity, it would be better not to disable any packets.

## 🛠️ Troubleshooting

If you experience issues after disabling a packet:

1. Remove the related packet ID from **data_config.json** file
2. Wait **5 minutes** (system will automatically re-read)

---

## 📖 Navigation

**📚 Documentation**: [← Previous](./tech_en.md) | [← Back to Main](../en.md) | **Next →** [Installation Guide](./this_en.md)

**🌐 Language**: [Türkçe](./packets_tr.md) | **English**

---

*Complete control of your mobile server with mobi-core!* 