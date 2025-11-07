#!/bin/sh

set -eu

if [ "$(id -u)" -ne 0 ]; then
  echo "You must be root."
  exit 1
fi


#update mt ip for ipfw rules etc.
PY=/usr/local/bin/python3
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
$SCRIPT_SEC/setup_ipfw.sh"

echo "=== Setup sequence started ==="

for SCRIPT in $SCRIPTS; do
  NAME=$(basename "$SCRIPT")

  echo "[$NAME] running..."
  if sh "$SCRIPT"; then
    echo "[$NAME] completed successfully."
  else
    echo "[$NAME] failed! Exiting..."
    exit 1
  fi

  echo "-------------------------------"
done

echo "=== All setup steps completed successfully ==="


PATH_APP_EXECUTABLE=/usr/mobile/bridge/App_Bridge
CMD_BRIDGE_START= "cd /usr/mobile/bridge/ && ./App_Bridge"
PID_FILE="/usr/mobile/bridge/App_Bridge.pid"
ALIAS_BRIDGE_START="bridge_start"
ALIAS_BRIDGE_STOP="bridge_stop"

chmod +x "$PATH_APP_EXECUTABLE"

echo "Updating /root/.shrc with mobi aliases..."
if ! grep -q "bridge_start" /root/.shrc; then
cat << EOF >> /root/.shrc
#App_Bridge aliases
$ALIAS_BRIDGE_START() {
  echo "Starting App_Bridge..."
  if [ -f "$PID_FILE" ]; then
    if kill -0 "\$(cat "$PID_FILE")" 2>/dev/null; then
      echo "App_Bridge already running (PID: \$(cat "$PID_FILE"))."
      return
    else
      echo "Removing stale PID file."
      rm -f "$PID_FILE"
    fi
  fi
  $CMD_BRIDGE_START & echo \$! > "$PID_FILE"
  echo "App_Bridge started with PID: \$(cat "$PID_FILE")"
}

$ALIAS_BRIDGE_STOP() {
  echo "Stopping App_Bridge..."
  if [ -f "$PID_FILE" ]; then
    PID=\$(cat "$PID_FILE")
    if kill -0 "\$PID" 2>/dev/null; then
      echo "Sending SIGTERM to PID \$PID..."
      kill "\$PID"
      for i in \$(seq 1 10); do
        if ! kill -0 "\$PID" 2>/dev/null; then
          echo "App_Bridge stopped gracefully."
          rm -f "$PID_FILE"
          return
        fi
        sleep 1
      done
      echo "Process still alive after 10s — forcing kill."
      kill -9 "\$PID"
      rm -f "$PID_FILE"
    else
      echo "No process found for PID \$PID (might already be stopped)."
      rm -f "$PID_FILE"
    fi
  else
    echo "PID file not found. App_Bridge may not be running."
  fi
}
EOF
else
  echo "Aliases already exist in /root/.shrc, skipping append."
fi

echo "Reloading /root/.shrc..."
# Only source .shrc if running in interactive mode to avoid bind errors
if [ -t 0 ]; then
  . /root/.shrc 2>/dev/null || echo "Note: .shrc contains interactive-only commands"
else
  echo "Non-interactive mode: .shrc will be loaded on next login"
fi

# Başlangıçta App_Bridge’i otomatik başlat ve PID kaydet
echo "Starting App_Bridge automatically..."
$CMD_BRIDGE_START & echo $! > "$PID_FILE"
echo "App_Bridge started (PID: $(cat "$PID_FILE"))."

echo "Whenever you want to:"
echo "  start AppBridge: $ALIAS_BRIDGE_START"
echo "  stop AppBridge:  $ALIAS_BRIDGE_STOP"
