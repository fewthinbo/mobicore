#!/bin/bash

set -eu

PY=/usr/bin/python3
PY_SCRIPT_JSON_UPDATE=/usr/mobile/scripts/secondary/update-json.py
SETTINGS_FILE=/usr/mobile/settings.json

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

#domaini ogren
while :; do
  printf "Enter your domain for bridge server (example: mobicore-test.com): "
  read DOMAIN_READ
  if ask_yesno "You entered '$DOMAIN_READ'. Confirm?"; then
    DOMAIN_NAME="$DOMAIN_READ"
    break
  fi
done

"$PY" "$PY_SCRIPT_JSON_UPDATE" --file "$SETTINGS_FILE" --field mobile_ws.domain:"$DOMAIN_NAME"

RET=$?
if [ $RET -ne 0 ]; then
  echo "[ERROR] $PY_SCRIPT_JSON_UPDATE failed with exit code $RET"
  echo "You need to adjust $SETTINGS_FILE fields manually"
else
  echo "$SETTINGS_FILE domain field updated with $DOMAIN_NAME"
fi

ACME_SH="/root/.acme.sh/acme.sh"
WEBROOT="/var/www/html"
SSL_DIR="/usr/mobile/ssl"

echo "=== SSL SETUP/RENEW SCRIPT STARTED ==="

# Gerekli paketleri kur
apt-get update
apt-get install -y python3 socat curl

# acme.sh yükle (ilk defaysa)
if [ ! -f "$ACME_SH" ]; then
  echo "acme.sh not found, installing..."
  curl https://get.acme.sh | sh
  set +u
  source /root/.profile
  set -u
fi

# ortam değişkenlerini yükle
set +u
source /root/.profile
set -u

echo "acme.sh is ready"
$ACME_SH --version

# Let's Encrypt kullan
$ACME_SH --set-default-ca --server letsencrypt

while :; do
  printf "Enter your e-mail for registration for lets encrypt(example: banana@gmail.com): "
  read EMAIL_READ
  if ask_yesno "You entered '$EMAIL_READ'. Confirm?"; then
    MY_EMAIL="$EMAIL_READ"
    break
  fi
done

# Hesap kayıtlı mı kontrol et (yoksa kayıt et)
ACCOUNT_PATH="/root/.acme.sh/account.conf"
if [ ! -f "$ACCOUNT_PATH" ]; then
  echo "Registering new account with email $MY_EMAIL"
  $ACME_SH --register-account -m $MY_EMAIL
fi

echo "Checking port 80 availability..."
if command -v netstat >/dev/null 2>&1; then
  netstat -tlnp | grep ':80' && echo "Port 80 in use!" || echo "Port 80 free."
elif command -v ss >/dev/null 2>&1; then
  ss -tlnp | grep ':80' && echo "Port 80 in use!" || echo "Port 80 free."
else
  echo "Neither netstat nor ss found, installing net-tools..."
  apt-get update && apt-get install -y net-tools
  netstat -tlnp | grep ':80' && echo "Port 80 in use!" || echo "Port 80 free."
fi

# Port 80 kullanımda mı kontrol et
PORT_80_IN_USE=false
if command -v ss >/dev/null 2>&1; then
  if ss -tlnp | grep -q ':80 '; then
    PORT_80_IN_USE=true
  fi
elif command -v netstat >/dev/null 2>&1; then
  if netstat -tlnp | grep -q ':80 '; then
    PORT_80_IN_USE=true
  fi
fi

# geçici python http server'ı başlat (sadece port boşsa)
PY_PID=""
if [ "$PORT_80_IN_USE" = false ]; then
  echo "Starting temporary Python HTTP server for domain verification..."
  mkdir -p "$WEBROOT"
  $PY -m http.server 80 --directory "$WEBROOT" &
  PY_PID=$!
  sleep 3
else
  echo "Port 80 is in use, skipping temporary HTTP server (assuming webroot is already served)"
fi

# Sertifika var mı kontrol et
CERT_PATH="/root/.acme.sh/${DOMAIN_NAME}/${DOMAIN_NAME}.cer"
if [ -f "$CERT_PATH" ]; then
  echo "Existing certificate found for $DOMAIN_NAME"
  echo "Certificate already exists, proceeding with installation..."
  CERT_ISSUED=true
else
  echo "No existing certificate found, issuing new one..."
  if $ACME_SH --issue -d "$DOMAIN_NAME" --webroot "$WEBROOT"; then
    echo "Certificate issued successfully."
    CERT_ISSUED=true
  else
    echo "Failed to issue certificate."
    CERT_ISSUED=false
  fi
fi

# sertifikayı kopyala (sadece başarılı olursa)
if [ "$CERT_ISSUED" = true ]; then
  echo "Installing certificate to $SSL_DIR"
  mkdir -p "$SSL_DIR"
  $ACME_SH --install-cert -d "$DOMAIN_NAME" \
    --key-file "$SSL_DIR/$DOMAIN_NAME.key" \
    --fullchain-file "$SSL_DIR/fullchain.cer" \
    --cert-file "$SSL_DIR/$DOMAIN_NAME.cer" \
    --ca-file "$SSL_DIR/ca.cer"
  echo "Certificate installed successfully."
else
  echo "Skipping certificate installation due to previous errors."
fi

# geçici http server'ı kapat (sadece başlattıysak)
if [ -n "$PY_PID" ]; then
  echo "Stopping temporary HTTP server..."
  kill $PY_PID 2>/dev/null || true
else
  echo "No temporary HTTP server to stop."
fi

echo "=== SSL SETUP/RENEW SCRIPT COMPLETED ==="