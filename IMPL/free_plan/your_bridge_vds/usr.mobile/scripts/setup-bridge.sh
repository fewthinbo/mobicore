#!/bin/sh
set -eu

SCRIPT_SEC="secondary"
CMD_BRIDGE_START=./usr/mobile/bridge/App_Bridge
ALIAS_BRIDGE_START="bridge_start"
ALIAS_CFG_ENCRYPT="bridge_cfg_encrypt"

SCRIPTS=(
  "$SCRIPT_SEC/setup_env.sh"
  "$SCRIPT_SEC/setup_cfg.sh"
  "$SCRIPT_SEC/setup_ssl.sh"
  "$SCRIPT_SEC/setup_cron.sh"
  "$SCRIPT_SEC/setup_sql.sh"
  "$SCRIPT_SEC/setup_ipfw.sh"
)

echo "Updating /root/.shrc with mobi aliases..."
if ! grep -q "bridge_start" /root/.shrc; then
cat << 'EOF' >> /root/.shrc
#App_Bridge aliases
alias $ALIAS_BRIDGE_START="$CMD_BRIDGE_START &"
alias $ALIAS_CFG_ENCRYPT="sh /usr/mobile/scripts/secondary/setup_cfg.sh"
EOF
else
	echo "Aliases already exist in /root/.shrc, skipping append."
fi

echo "Reloading /root/.shrc..."
. /root/.shrc

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

$CMD_BRIDGE_START &
echo "Whenever you want to:"
echo "  start AppBridge: $ALIAS_BRIDGE_START"
echo "  encrypt config.json file: $ALIAS_CFG_ENCRYPT"
