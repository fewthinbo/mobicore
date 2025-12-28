#!/bin/bash

set -eu

DB_NAME="ws_database"
PY=/usr/bin/python3
PY_SCRIPT_JSON_UPDATE=/usr/mobile/scripts/secondary/update-json.py
JSON_FILE=/usr/mobile/config.json

# helper: simple yes/no prompt. returns 0 for yes, 1 for no
ask_yesno() {
  prompt="$1"
  while :; do
    printf "%s [y/n]: " "$prompt"
    read ans
    case "$ans" in
      [Yy]|[Yy][Ee][Ss]) return 0 ;;
      [Nn]|[Nn][Oo]) return 1 ;;
      *) printf "Please answer y or n.\n" ;;
    esac
  done
}

# helper: escape single quotes for SQL literal via doubling (MySQL accepts '' as quote)
sql_escape() {
  printf '%s' "$1" | sed "s/'/''/g"
}

generate_pass() {
  # prefer openssl if available
  if command -v openssl >/dev/null 2>&1; then
    openssl rand -base64 18
  else
    # fallback: dd + base64
    dd if=/dev/urandom bs=1 count=24 2>/dev/null | base64 | tr -d '/+' | cut -c1-24
  fi
}

echo "Updating package lists..."
apt-get update

echo "Installing mariadb server"
apt-get install -y mariadb-server

# Konfigürasyon dizini varsa değiştir
CONF_DIR="/etc/mysql/mariadb.conf.d"
mkdir -p "$CONF_DIR"
cat > "$CONF_DIR/50-server.cnf" <<EOF
[mysqld]
bind-address = 0.0.0.0
skip-networking=0
EOF

# systemd servisini başlat ve enable et
systemctl enable mariadb
systemctl start mariadb
echo "Waiting 3s for MySQL to start properly..."
sleep 3

# Tüm default user'ları kaldır (root hariç)
mysql -u root -N -B -e "
SELECT CONCAT('DROP USER IF EXISTS \\'', user, '\\'@\\'', host, '\\';')
FROM mysql.user
WHERE user NOT IN ('root') AND user NOT LIKE 'mysql.%';
" > /root/drop_users.sql

mysql -u root < /root/drop_users.sql
mysql -u root -e "FLUSH PRIVILEGES;"
echo "Removed default MySQL users."

while :; do
  RANDOM_STRONG_PASS="$(generate_pass)"
  printf "Generated strong password: %s\n" "$RANDOM_STRONG_PASS"
  if ask_yesno "Use this password?"; then
    break
  fi
done

echo "Setting MySQL root password..."
mysql -u root <<SQL
ALTER USER 'root'@'localhost' IDENTIFIED BY '${RANDOM_STRONG_PASS}';
FLUSH PRIVILEGES;
SQL

# Root şifresini doğrula
if mysql -u root -p"${RANDOM_STRONG_PASS}" -e "SELECT 1;" >/dev/null 2>&1; then
  echo "Root password set successfully."
else
  echo "Failed to authenticate with new root password!"
  exit 1
fi

while :; do
  printf "Testing MySQL root authentication...\n"
  if mysql -u root -p"$RANDOM_STRONG_PASS" -e "SELECT 1;" >/dev/null 2>&1; then
    echo "MySQL root auth OK."
    break
  else
    echo "Authentication failed. Try again."
  fi
done

# Veritabanını oluştur
echo "Creating database '$DB_NAME'..."
mysql -u root -p"$RANDOM_STRONG_PASS" <<SQL
CREATE DATABASE IF NOT EXISTS \`${DB_NAME}\`;
FLUSH PRIVILEGES;
SQL

echo "Database '$DB_NAME' created successfully."

# ensure python script is present and executable
if [ ! -f "$PY_SCRIPT_JSON_UPDATE" ]; then
  echo "Please create $PY_SCRIPT_JSON_UPDATE (put the python code there) and make it executable."
  exit 1
fi
chmod +x "$PY_SCRIPT_JSON_UPDATE"

# call python updater
"$PY" "$PY_SCRIPT_JSON_UPDATE" --file "$JSON_FILE" --field db.host:"localhost:3306" --field db.user:"root" --field db.password:"$RANDOM_STRONG_PASS"
RET=$?
if [ $RET -ne 0 ]; then
  echo "[ERROR] $PY_SCRIPT_JSON_UPDATE failed with exit code $RET"
  echo "You need to adjust $JSON_FILE 'db' fields manually"
fi

echo "=== MariaDB setup completed successfully ==="
echo "You can take a note of root password: $RANDOM_STRONG_PASS"