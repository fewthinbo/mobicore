#!/bin/sh
set -eu

INPUT_FILE="/usr/mobile/config.json"

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

#take password
while :; do
  printf "Enter your config.json password for encrypt(Sent you with e-mail): "
  read PASSWD
  if ! validate_ip "$PASSWD"; then
    echo "Invalid input, try again."
    continue
  fi
  if ask_yesno "You entered '$PASSWD'. Confirm?"; then
    CFG_PWD="$PASSWD"
    break
  fi
done

echo "=== Config file encrypting ==="
#run encrypter for the first setup
./usr/mobile/App_Encrypter INPUT_FILE CFG_PWD
EXIT_CODE=$?
if [ $EXIT_CODE -eq 0 ]; then
  echo "Encryption succeeded!" 
  echo "You can copy $INPUT_FILE to your desktop and remove from vds."
else
  echo "Encryption failed! Exit code: $EXIT_CODE"
fi