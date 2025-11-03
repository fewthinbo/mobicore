#!/bin/sh

echo "Running prerequisite script..."
secondary/setup_env.sh || { echo "Script failed!"; exit 1; }

echo "Updating /root/.shrc with mobi aliases..."
if ! grep -q "mobi_clean_rel" /root/.shrc; then
cat << 'EOF' >> /root/.shrc
#release
alias mobi_preset_rel="cd /usr/mobile/src/ && cmake --preset bsd-ninja-release"
alias mobi_clean_rel="mobi_preset_rel && cd /usr/mobile/src/ && cmake --build --preset bsd-ninja-release --target clean"
alias mobi_install_rel="mobi_preset_rel && mobi_clean_rel && cd /usr/mobile/src/ && cmake --build --preset bsd-ninja-release --target install"

#debug
alias mobi_preset_debug="cd /usr/mobile/src/ && cmake --preset bsd-ninja-debug"
alias mobi_clean_debug="mobi_preset_debug && cd /usr/mobile/src/ && cmake --build --preset bsd-ninja-debug --target clean"
alias mobi_install_debug="mobi_preset_debug && mobi_clean_debug cd /usr/mobile/src/ && cmake --build --preset bsd-ninja-debug --target install"
EOF
else
	echo "Aliases already exist in /root/.shrc, skipping append."
fi

echo "Reloading /root/.shrc..."
. /root/.shrc

echo "Mobicore installing..."
mobi_install_rel

echo "Running script..."
secondary/setup_sql.sh || { echo "Script failed!"; exit 1; }