#!/bin/sh

set -eu

echo "=== FreeBSD build environment setup starting ==="

# ----------------------------
# 1. Base packages
# ----------------------------
echo "Installing essential packages..."
pkg install -y cmake git bash autoconf automake libtool gmake patchelf python3 pkgconf zip unzip curl 


# ----------------------------
# 2. Ninja installation (skip if exists)
# ----------------------------
if command -v ninja >/dev/null 2>&1; then
  echo "ninja already installed: $(which ninja)"
else
  echo "Installing ninja..."
  cd /tmp
  fetch -q https://github.com/ninja-build/ninja/archive/refs/tags/v1.12.1.tar.gz
  tar -xf v1.12.1.tar.gz
  cd ninja-1.12.1
  cmake -B build -G "Unix Makefiles"
  cmake --build build
  cmake --install build
  echo "ninja installed at: $(which ninja || echo /usr/local/bin/ninja)"
fi

# symlinks
which ninja || ln -sf /usr/local/bin/ninja /usr/bin/ninja
which gmake || ln -sf /usr/local/bin/gmake /usr/bin/gmake


# ----------------------------
# 4. vcpkg installation
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

# ----------------------------
# 5. Environment setup
# ----------------------------
PROFILE_FILE="$HOME/.profile"
CMAKE_TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake"

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

ensure_export "VCPKG_ROOT" "$VCPKG_DIR"
ensure_export "CMAKE_TOOLCHAIN_FILE" "$CMAKE_TOOLCHAIN_FILE"
ensure_export "CMAKE_MAKE_PROGRAM" "/usr/local/bin/ninja"
ensure_export "MAKE" "/usr/local/bin/gmake"

# PATH satırını tekrarlamadan ekle
if ! grep -q "$VCPKG_DIR" "$PROFILE_FILE" 2>/dev/null; then
  echo "export PATH=\"\$PATH:$VCPKG_DIR\"" >> "$PROFILE_FILE"
fi

# Yükle
. "$PROFILE_FILE"

echo "Environment configured:"
echo "  VCPKG_ROOT: $VCPKG_ROOT"
echo "  PATH includes: $VCPKG_ROOT"
echo "  CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
echo "  CMAKE_MAKE_PROGRAM: $CMAKE_MAKE_PROGRAM"
echo "  MAKE: $MAKE"

echo "=== FreeBSD environment setup completed successfully ==="
echo
echo "To verify:"
echo "  echo \$VCPKG_ROOT   # should be $HOME/vcpkg"
echo "  vcpkg version"
