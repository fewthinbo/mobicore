#!/bin/bash

set -eu

echo "=== Debian/Ubuntu build environment setup starting ==="
NEED_NINJA=0
NEED_VCPKG=0

# ----------------------------
# 1. Base packages
# ----------------------------
echo "Updating package lists..."
apt-get update

echo "Installing essential packages..."
apt-get install -y cmake git bash autoconf automake libtool make patchelf python3 pkg-config zip unzip curl build-essential

# Make sure make is available
which make || ln -sf /usr/bin/make /usr/local/bin/make

if [ "$NEED_NINJA" -gt 0 ]; then
	# ----------------------------
	# 2. Ninja installation (skip if exists)
	# ----------------------------
	if command -v ninja >/dev/null 2>&1; then
	  echo "ninja already installed: $(which ninja)"
	else
	  echo "Installing ninja..."
	  apt-get install -y ninja-build
	  echo "ninja installed at: $(which ninja)"
	fi
fi

if [ "$NEED_VCPKG" -gt 0 ]; then
	# ----------------------------
	# 3. vcpkg installation
	# ----------------------------
	VCPKG_DIR="$HOME/vcpkg"
	if [ -d "$VCPKG_DIR" ]; then
	  echo "vcpkg already exists at $VCPKG_DIR"
	  cd "$VCPKG_DIR"
	  git pull --quiet || true
	else
	  echo "Cloning vcpkg..."
	  git clone https://github.com/Microsoft/vcpkg.git "$VCPKG_DIR"
	  cd "$VCPKG_DIR"
	  ./bootstrap-vcpkg.sh -disableMetrics
	fi
fi

# ----------------------------
# 4. Environment setup
# ----------------------------
PROFILE_FILE="$HOME/.profile"

echo "Configuring environment variables..."

ensure_export() {
  VAR_NAME="$1"
  VAR_VALUE="$2"
  if ! grep -q "export $VAR_NAME=" "$PROFILE_FILE" 2>/dev/null; then
    echo "export $VAR_NAME=\"$VAR_VALUE\"" >> "$PROFILE_FILE"
  else
    # güncelle
    sed "s|^export $VAR_NAME=.*$|export $VAR_NAME=\"$VAR_VALUE\"|" "$PROFILE_FILE" > "$PROFILE_FILE.tmp" && mv "$PROFILE_FILE.tmp" "$PROFILE_FILE"
  fi
}

ensure_export "MAKE" "/usr/bin/make"

if [ "$NEED_NINJA" -gt 0 ]; then
	ensure_export "CMAKE_MAKE_PROGRAM" "/usr/bin/ninja"
fi

if [ "$NEED_VCPKG" -gt 0 ]; then
	CMAKE_TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake"
	ensure_export "CMAKE_TOOLCHAIN_FILE" "$CMAKE_TOOLCHAIN_FILE"
	ensure_export "VCPKG_ROOT" "$VCPKG_DIR"
	# PATH satırını tekrarlamadan ekle
	if ! grep -q "$VCPKG_DIR" "$PROFILE_FILE" 2>/dev/null; then
		echo "export PATH=\"\$PATH:$VCPKG_DIR\"" >> "$PROFILE_FILE"
	fi
fi

# Yükle (ignore unbound variables during source)
set +u
source "$PROFILE_FILE"
set -u

echo "Environment configured:"

if [ "$NEED_VCPKG" -gt 0 ]; then
	echo "  VCPKG_ROOT: $VCPKG_ROOT"
	echo "  PATH includes: $VCPKG_ROOT"
	echo "  CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
	echo "  CMAKE_MAKE_PROGRAM: $CMAKE_MAKE_PROGRAM"
fi

echo "  MAKE: $MAKE"
echo "=== Debian/Ubuntu environment setup completed successfully ==="
echo

if [ "$NEED_VCPKG" -gt 0 ]; then
	echo "To verify:"
	echo "  echo \$VCPKG_ROOT   # should be $HOME/vcpkg"
	echo "  vcpkg version"
fi