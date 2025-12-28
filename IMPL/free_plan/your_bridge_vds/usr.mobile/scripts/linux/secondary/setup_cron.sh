#!/bin/bash

ACME_SH="/root/.acme.sh/acme.sh"
ACME_HOME="/root/.acme.sh"

if [ ! -f "$ACME_SH" ]; then
  echo "acme.sh not found, installing..."
  curl https://get.acme.sh | sh
  set +u
  source /root/.profile
  set -u
fi

set +u
source /root/.profile
set -u

echo "acme.sh is ready"
$ACME_SH --version

# Let's Encrypt kullan
$ACME_SH --set-default-ca --server letsencrypt

ACCOUNT_PATH="/root/.acme.sh/account.conf"
if [ ! -f "$ACCOUNT_PATH" ]; then
  echo "Registering new account with email mobicore.io@gmail.com"
  $ACME_SH --register-account -m mobicore.io@gmail.com
fi

# cron zamani her gun 03:00
CRON_MIN=0
CRON_HOUR=3

echo "Setting up daily cron job for acme.sh (at ${CRON_HOUR}:${CRON_MIN})..."

CRON_CMD="$CRON_MIN $CRON_HOUR * * * \"$ACME_SH\" --cron --home \"$ACME_HOME\" > /dev/null"

TMPFILE=$(mktemp /tmp/acme_cron.XXXXXX)
crontab -l 2>/dev/null > "$TMPFILE" || true

# zaten ekli mi kontrol et
if grep -Fq -- "$ACME_SH --cron --home" "$TMPFILE"; then
  echo "Cron job already exists in acme.sh, skipping"
else
  echo "$CRON_CMD" >> "$TMPFILE"
  crontab "$TMPFILE"
  echo "Cron job added: $CRON_CMD"
fi

rm -f "$TMPFILE"

echo "Cron setup completed."