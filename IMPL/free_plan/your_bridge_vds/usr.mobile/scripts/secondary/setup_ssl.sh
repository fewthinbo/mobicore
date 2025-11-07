#!/bin/sh

set -eu

PY=/usr/local/bin/python3
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
WEBROOT="/usr/local/www/htdocs"
SSL_DIR="/usr/mobile/ssl"

echo "=== SSL SETUP/RENEW SCRIPT STARTED ==="

# Gerekli paketleri kur
#pkg install -y python socat >/dev/null

# acme.sh yükle (ilk defaysa)
if [ ! -f "$ACME_SH" ]; then
  echo "acme.sh not found, installing..."
  curl https://get.acme.sh | sh
  . /root/.profile
fi

# ortam değişkenlerini yükle
. /root/.profile

echo "acme.sh is ready"
$ACME_SH --version

# Let's Encrypt kullan
$ACME_SH --set-default-ca --server letsencrypt

# Hesap kayıtlı mı kontrol et (yoksa kayıt et)
ACCOUNT_PATH="/root/.acme.sh/account.conf"
if [ ! -f "$ACCOUNT_PATH" ]; then
  echo "Registering new account with email mobicore.io@gmail.com"
  $ACME_SH --register-account -m mobicore.io@gmail.com
fi

echo "Checking port 80 availability..."
sockstat -4l | grep ':80' && echo "Port 80 in use!" || echo "Port 80 free."

# geçici python http server'ı başlat
echo "Starting temporary Python HTTP server for domain verification..."
mkdir -p "$WEBROOT"
$PY -m http.server 80 --directory "$WEBROOT" &
PY_PID=$!

sleep 3

# Sertifika var mı kontrol et
CERT_PATH="/root/.acme.sh/${DOMAIN_NAME}/${DOMAIN_NAME}.cer"
if [ -f "$CERT_PATH" ]; then
  echo "Existing certificate found for $DOMAIN_NAME"
  echo "Renewing certificate..."
#zorla yenilemek icin: --force
  $ACME_SH --renew -d "$DOMAIN_NAME" --webroot "$WEBROOT"
else
  echo "No existing certificate found, issuing new one..."
  $ACME_SH --issue -d "$DOMAIN_NAME" --webroot "$WEBROOT"
fi

# sertifikayı kopyala
echo "Installing certificate to $SSL_DIR"
mkdir -p "$SSL_DIR"
$ACME_SH --install-cert -d "$DOMAIN_NAME" \
  --key-file "$SSL_DIR/$DOMAIN_NAME.key" \
  --fullchain-file "$SSL_DIR/fullchain.cer" \
  --cert-file "$SSL_DIR/$DOMAIN_NAME.cer" \
  --ca-file "$SSL_DIR/ca.cer"

# geçici http server'ı kapat
echo "Stopping temporary HTTP server..."
kill $PY_PID 2>/dev/null

echo "=== SSL SETUP/RENEW SCRIPT COMPLETED ==="