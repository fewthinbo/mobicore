#!/bin/sh
set -eu

if [ "$(id -u)" -ne 0 ]; then
  echo "You must be root."
  exit 1
fi

SCRIPT_SEC="secondary"

SCRIPTS=(
  "$SCRIPT_SEC/setup_env.sh"
  "$SCRIPT_SEC/setup_ssl.sh"
  "$SCRIPT_SEC/setup_cron.sh"
  "$SCRIPT_SEC/setup_sql.sh"
  "$SCRIPT_SEC/setup_ipfw.sh"
)

echo "=== Setup sequence started ==="

for SCRIPT in "${SCRIPTS[@]}"; do
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


CMD_BRIDGE_START=./usr/mobile/bridge/App_Bridge
PID_FILE="/usr/mobile/bridge/App_Bridge.pid"
ALIAS_BRIDGE_START="bridge_start"
ALIAS_BRIDGE_STOP="bridge_stop"

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
. /root/.shrc

# Başlangıçta App_Bridge’i otomatik başlat ve PID kaydet
echo "Starting App_Bridge automatically..."
$CMD_BRIDGE_START & echo $! > "$PID_FILE"
echo "App_Bridge started (PID: $(cat "$PID_FILE"))."

echo "Whenever you want to:"
echo "  start AppBridge: $ALIAS_BRIDGE_START"
echo "  stop AppBridge:  $ALIAS_BRIDGE_STOP"
