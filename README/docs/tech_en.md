# 🔧 mobi-core - Technical Management Guide

## 📋 Overview

The mobi-core system allows you to manage the bridge server through JSON files on your MT server. All settings are updated in real-time and optimized for system performance.

## 🗄️ SQL Database Configuration

### 🔐 Security Priority
mobi-core requires a special SQL user with **SELECT privileges only**. This ensures:
- ✅ Your data remains completely secure
- ✅ Bridge server cannot get write/modify permissions
- ✅ Only necessary information can be read

### 📊 Performance Advantage
When new characters, accounts or guilds are created:
- ✅ No data transfer from MT server to bridge server
- ✅ Network traffic kept at minimum level

### 🛠️ SQL User Setup
- Create a user which can run select queries on 'account.account' and 'player.' tables.
- 'RANDOM_STRONG_PASS' length should be lower than PASSWORD_MAX_LENGTH(32)
```sql
CREATE USER 'mobicore'@'YOUR_MOBI_BRIDGE_SERVER_IP' IDENTIFIED BY 'RANDOM_STRONG_PASS';
GRANT SELECT ON account.account* TO 'mobicore'@'YOUR_MOBI_BRIDGE_SERVER_IP';
GRANT SELECT ON player.* TO 'mobicore'@'YOUR_MOBI_BRIDGE_SERVER_IP';
FLUSH PRIVILEGES;
```
- After that put the infos to db fields in "/usr/mobile/info.json"

> ⚠️ **Important**: Never create full privilege SQL user!

## 📁 Configuration Files

### 📄 data_config.json
**Location**: On your VDS where MT server runs  
**Reading Frequency**: Every 5 minutes

#### 🔌 Database Connection
```json
{
  "db": {
    "host": "localhost:3306",
    "user": "mobi_readonly",
    "password": "secure_password"
  }
}
```

#### ⚙️ System Settings
```json
{
  "settings": {
    "mobile": {
      "security": {
        "max_active_sessions": 1000000,
        "ip_blocklist": ["***.**.**.***"],
        "heartbeat": {
          "interval_seconds": 60,
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
        "message": "Currently under maintenance."
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
**Location**: On your VDS where MT server runs  
**Reading Frequency**: Every 5 minutes  
**Display**: Main page in mobile application

```json
{
  "notifications": [
    {
      "title": "New Event Started!",
      "subtitle": "Double EXP Event",
      "details": "Gain double experience throughout this weekend!",
      "admin_name": "GameMaster"
    },
    {
      "title": "Server Updated",
      "subtitle": "New features added",
      "details": "Login to the game for details...",
      "admin_name": "Admin"
    }
  ]
}
```

## 🏗️ Core Authorization System

### 🎯 Smart Management
- **Single Core Reading**: Different authorities are assigned to all your cores.
- **Authority Distribution**: Each core handles tasks within its authority

### 🔄 Update Process
1. **JSON Modification**: Edit the file
2. **Automatic Detection**: System reads within 5 minutes
3. **Instant Update**: All settings take effect
4. **Mobile Reflection**: Changes appear on mobile

## 🎛️ Manageable Settings

### 🔒 Security Controls
- **Maximum Connection Count**: Protects server capacity
- **IP Blocking**: Blocks unwanted IPs
- **Heartbeat Control**: Cleans inactive connections
- **Login Policy**: Prevents brute force attacks
- **Rate Limiting**: Spam protection
  > ⚠️ **Important**: Default settings are recommended!

### 📊 Performance Settings
- **Online Counter**: Enable/disable and refresh frequency
- **Maintenance Mode**: For instant maintenance
- **Packet Filter**: Disable unwanted packets

## 💡 Practical Usage Examples

### 🎮 Event Announcement
```json
{
  "title": "Lucky Box Event",
  "subtitle": "For 3 Days!",
  "details": "Get 2x rewards from lucky boxes this weekend!",
  "admin_name": "EventManager"
}
```

### 🛠️ Maintenance Mode
```json
{
  "maintenance_mode": {
    "enabled": true,
    "message": "Server update in progress. Please try again in 30 minutes."
  }
}
```

### 🔒 Security Enhancement
```json
{
  "security": {
    "max_active_sessions": 5000,
    "rate_limit": {
      "per_second": 10,
      "max_overflows": 2
    }
  }
}
```

## 🔧 Installation and Setup

0. **License**: Contact the mail address below to get your license
1. **MT Implementation**: Read IMPL folder and make sure you have made the necessary additions to your source files
2. **SQL User**: Create user with SELECT privileges only
3. **JSON Files**: Place in usr/mobile/ folder on your VDS
4. **Configure Settings**: Enter your SQL information in data_config.json
5. **Start Your Server**: Everything will be ready automatically
	> ✅**Note**: Don't worry about unpleasant situations such as internet connection loss, server shutdown due to infrastructure-related issues at hosting companies, connections will be automatically re-established.

## 📞 Support

For any questions: **mobicore.io@gmail.com**

---

## 📖 Navigation

**📚 Documentation**: [← Previous](./logic_en.md) | [← Back to Main](../en.md) | **Next →** [Packet Reference](./packets_en.md)

**🌐 Language**: [Türkçe](./tech_tr.md) | **English**

---

*Complete control of your mobile server with mobi-core!* 