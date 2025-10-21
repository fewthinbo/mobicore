#!/bin/sh
set -eu

#constants
DB_USER=mobicore
SCRIPT=/usr/mobile/scripts/secondary/update-json.py
JSON_FILE=/usr/mobile/info.json
PY=/usr/local/bin/python3

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

# helper: validate IPv4-ish simple (not full strict) - accept IPv4 and IPv6-ish formats minimally
validate_ip() {
  ip="$1"
  # allow IPv4 dotted quad OR contains letters/digits/: for IPv6 or hostname
  # We'll accept hostnames too (e.g. example.com) - but if you want only IPv4 uncomment next line
  # printf '%s\n' "$ip" | grep -Eq '^([0-9]{1,3}\.){3}[0-9]{1,3}$'
  [ -n "$ip" ]
}

# helper: validate DB name (only letters, numbers, underscore)
validate_dbname() {
  name="$1"
  if printf '%s\n' "$name" | grep -Eq '^[A-Za-z0-9_]+$'; then
    return 0
  else
    return 1
  fi
}

# helper: escape single quotes for SQL literal via doubling (MySQL accepts '' as quote)
sql_escape() {
  printf '%s' "$1" | sed "s/'/''/g"
}

# 1) Bridge IP (with confirmation)
while :; do
  printf "Enter your bridge server IP (example: 123.123.123.123): "
  read BRIDGE_IP
  if ! validate_ip "$BRIDGE_IP"; then
    echo "Invalid input, try again."
    continue
  fi
  if ask_yesno "You entered '$BRIDGE_IP'. Confirm?"; then
    YOUR_MOBI_BRIDGE_SERVER_IP="$BRIDGE_IP"
    break
  fi
done

# 2) Strong password generation + confirmation (2-step)
generate_pass() {
  # prefer openssl if available
  if command -v openssl >/dev/null 2>&1; then
    openssl rand -base64 18
  else
    # fallback: dd + base64 (FreeBSD should have base64 via base64 tool)
    dd if=/dev/urandom bs=1 count=24 2>/dev/null | base64 | tr -d '/+' | cut -c1-24
  fi
}

while :; do
  RANDOM_STRONG_PASS="$(generate_pass)"
  printf "Generated strong password: %s\n" "$RANDOM_STRONG_PASS"
  if ask_yesno "Use this password?"; then
    break
  fi
done

# 3) Ask three DB names (account, player, common) with confirmation and validation
while :; do
  printf "What's your account database name? "
  read DB_ACCOUNT
  if ! validate_dbname "$DB_ACCOUNT"; then
    echo "DB name must contain only letters, numbers or underscore. Try again."
    continue
  fi
  if ask_yesno "You entered '$DB_ACCOUNT'. Confirm?"; then break; fi
done

while :; do
  printf "What's your player database name? "
  read DB_PLAYER
  if ! validate_dbname "$DB_PLAYER"; then
    echo "DB name must contain only letters, numbers or underscore. Try again."
    continue
  fi
  if ask_yesno "You entered '$DB_PLAYER'. Confirm?"; then break; fi
done

while :; do
  printf "What's your common database name? "
  read DB_COMMON
  if ! validate_dbname "$DB_COMMON"; then
    echo "DB name must contain only letters, numbers or underscore. Try again."
    continue
  fi
  if ask_yesno "You entered '$DB_COMMON'. Confirm?"; then break; fi
done

#ask db port with default value
while :; do
  printf "Enter your database port (for default, type: 3306): "
  read DB_PORT
  if ask_yesno "You entered '$DB_PORT'. Confirm?"; then
    DB_PORT="$DB_PORT"
    break
  fi
done

# show summary
echo "Summary:"
echo "  Bridge IP: $YOUR_MOBI_BRIDGE_SERVER_IP"
echo "  SQL user $DB_USER password: $RANDOM_STRONG_PASS"
echo "  DB_ACCOUNT: $DB_ACCOUNT"
echo "  DB_PLAYER: $DB_PLAYER"
echo "  DB_COMMON: $DB_COMMON"
echo "  DB_PORT: $DB_PORT"

# 4) Prompt for MySQL root password and keep trying until it works
while :; do
  printf "Enter MySQL root password: "
  # hide input
  stty -echo || true
  read MYSQL_ROOT_PW
  stty echo || true
  printf "\n"
  # quick test
  if mysql -u root -p"$MYSQL_ROOT_PW" -e "SELECT 1;" >/dev/null 2>&1; then
    echo "MySQL root auth OK."
    break
  else
    echo "Authentication failed. Try again."
  fi
done

# 5) Prepare escaped variables for SQL
ESC_PASS="$(sql_escape "$RANDOM_STRONG_PASS")"
ESC_BRIDGE="$(sql_escape "$YOUR_MOBI_BRIDGE_SERVER_IP")"
# db names are validated to safe chars; still protect by surrounding with backticks
DB_ACCOUNT_BQ="\`$DB_ACCOUNT\`"
DB_PLAYER_BQ="\`$DB_PLAYER\`"
DB_COMMON_BQ="\`$DB_COMMON\`"


# 6) Run SQL commands
echo "Creating user and applying grants..."
mysql -u root -p"$MYSQL_ROOT_PW" <<SQL
CREATE USER IF NOT EXISTS '$DB_USER'@'$YOUR_MOBI_BRIDGE_SERVER_IP' IDENTIFIED BY '$ESC_PASS';
GRANT SELECT (id, login, email) ON $DB_ACCOUNT_BQ.\`account\` TO '$DB_USER'@'$YOUR_MOBI_BRIDGE_SERVER_IP';
GRANT SELECT (mID, mAuthority) ON $DB_COMMON_BQ.\`gmlist\` TO '$DB_USER'@'$YOUR_MOBI_BRIDGE_SERVER_IP';
GRANT SELECT ON $DB_PLAYER_BQ.* TO '$DB_USER'@'$YOUR_MOBI_BRIDGE_SERVER_IP';
FLUSH PRIVILEGES;
SQL

if [ $? -eq 0 ]; then
  echo "SQL commands executed successfully."
  echo "User '$DB_USER'@'$YOUR_MOBI_BRIDGE_SERVER_IP' created/updated with the generated password."
else
  echo "There was an error running the SQL commands. Check MySQL server and credentials."
  exit 1
fi

# ensure python script is present and executable
if [ ! -f "$SCRIPT" ]; then
  echo "Please create $SCRIPT (put the python code there) and make it executable."
  exit 1
fi
chmod +x "$SCRIPT"

# call python updater
"$PY" "$SCRIPT" --file "$JSON_FILE" --bridge-ip "$YOUR_MOBI_BRIDGE_SERVER_IP" --db-pass "$RANDOM_STRONG_PASS" --db-user "$DB_USER" --db-port "$DB_PORT"
RET=$?
if [ $RET -ne 0 ]; then
  echo "Updater script failed with exit code $RET"
  exit $RET
fi

echo "Done."
