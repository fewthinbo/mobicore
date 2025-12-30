#!/bin/bash

PATH_APP_EXECUTABLE=/usr/mobile/bridge/App_Bridge
PID_FILE="/usr/mobile/bridge/App_Bridge.pid"
ALIAS_BRIDGE_START="bridge_start"
ALIAS_BRIDGE_STOP="bridge_stop"

chmod +x "$PATH_APP_EXECUTABLE"

echo "Updating /root/.bashrc with mobi aliases..."
if ! grep -q "bridge_start" /root/.bashrc; then
cat << 'EOF' >> /root/.bashrc
#App_Bridge aliases
bridge_start() {
  echo "Starting App_Bridge..."
  PID_FILE="/usr/mobile/bridge/App_Bridge.pid"
  if [ -f "$PID_FILE" ]; then
    if kill -0 "$(cat "$PID_FILE")" 2>/dev/null; then
      echo "App_Bridge already running (PID: $(cat "$PID_FILE"))."
      return
    else
      echo "Removing stale PID file."
      rm -f "$PID_FILE"
    fi
  fi
  cd /usr/mobile/bridge/
  nohup ./App_Bridge &
  BRIDGE_PID=$!
  echo $BRIDGE_PID > "$PID_FILE"
  cd - > /dev/null
  echo "App_Bridge started with PID: $BRIDGE_PID"
}

bridge_stop() {
  echo "Stopping App_Bridge..."
  PID_FILE="/usr/mobile/bridge/App_Bridge.pid"
  if [ -f "$PID_FILE" ]; then
    PID=$(cat "$PID_FILE")
    if kill -0 "$PID" 2>/dev/null; then
      echo "Sending SIGTERM to PID $PID..."
      kill "$PID"
      for i in $(seq 1 10); do
        if ! kill -0 "$PID" 2>/dev/null; then
          echo "App_Bridge stopped gracefully."
          rm -f "$PID_FILE"
          return
        fi
        sleep 1
      done
      echo "Process still alive after 10s — forcing kill."
      kill -9 "$PID"
      rm -f "$PID_FILE"
    else
      echo "No process found for PID $PID (might already be stopped)."
      rm -f "$PID_FILE"
    fi
  else
    echo "PID file not found. App_Bridge may not be running."
  fi
}
EOF
else
  echo "Aliases already exist in /root/.bashrc, skipping append."
fi

echo "Reloading /root/.bashrc..."
# Only source .bashrc if running in interactive mode to avoid bind errors
if [ -t 0 ]; then
  set +u
  source /root/.bashrc 2>/dev/null || echo "Note: .bashrc contains interactive-only commands"
  set -u
else
  echo "Non-interactive mode: .bashrc will be loaded on next login"
fi

# Başlangıçta App_Bridge'i otomatik başlat ve PID kaydet
echo "Starting App_Bridge automatically..."
cd /usr/mobile/bridge/
./App_Bridge &
BRIDGE_PID=$!
echo $BRIDGE_PID > "$PID_FILE"
cd - > /dev/null
echo "App_Bridge started with PID: $BRIDGE_PID"

echo "Whenever you want to:"
echo "  start AppBridge: $ALIAS_BRIDGE_START"
echo "  stop AppBridge:  $ALIAS_BRIDGE_STOP"