#!/bin/bash

set -eu

if [ "$(id -u)" -ne 0 ]; then
  echo "You must be root."
  exit 1
fi

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

#update mt ip for ipfw rules etc.
PY=/usr/bin/python3
PY_SCRIPT_JSON_UPDATE=/usr/mobile/scripts/secondary/update-json.py
SETTINGS_FILE=/usr/mobile/settings.json

while :; do
  printf "Enter your mt server vds ip (example: xx.xx.xxx.xxx): "
  read READ_MT_HOST
  if ask_yesno "You entered '$READ_MT_HOST'. Confirm?"; then
    MT_HOST="$READ_MT_HOST"
    break
  fi
done

"$PY" "$PY_SCRIPT_JSON_UPDATE" --file "$SETTINGS_FILE" --field mt.remote_host:"$MT_HOST"


SCRIPT_SEC="secondary"

SCRIPTS="$SCRIPT_SEC/setup_env.sh \
$SCRIPT_SEC/setup_ssl.sh \
$SCRIPT_SEC/setup_cron.sh \
$SCRIPT_SEC/setup_sql.sh \
$SCRIPT_SEC/setup_iptables.sh \
$SCRIPT_SEC/setup_alias.sh"

echo "=== Setup sequence started ==="

for SCRIPT in $SCRIPTS; do
  NAME=$(basename "$SCRIPT")

  echo "[$NAME] running..."
  if bash "$SCRIPT"; then
    echo "[$NAME] completed successfully."
  else
    echo "[$NAME] failed! Exiting..."
    exit 1
  fi

  echo "-------------------------------"
done

echo "=== All setup steps completed successfully ==="