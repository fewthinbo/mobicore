#NEED_VCPKG=true

pkg install -y cmake git bash autoconf automake libtool gmake patchelf python3

pkg install -y pkgconf
cd /tmp
fetch https://github.com/ninja-build/ninja/archive/refs/tags/v1.12.1.tar.gz
tar -xf v1.12.1.tar.gz
cd ninja-1.12.1
cmake -B build
cmake --build build
cmake --install build

which ninja || ln -sf /usr/local/bin/ninja /usr/bin/ninja

which gmake || ln -sf /usr/local/bin/gmake /usr/bin/gmake


#if [ "$NEED_VCPKG" = true ]; then
echo "installing vcpkg dependencies..."
pkg install -y zip unzip curl

echo "vcpkg installing..."
git clone https://github.com/Microsoft/vcpkg.git "$HOME/vcpkg"
cd "$HOME/vcpkg"
./bootstrap-vcpkg.sh -disableMetrics

export VCPKG_ROOT="$HOME/vcpkg"
export PATH="$PATH:$HOME/vcpkg"
export CMAKE_MAKE_PROGRAM="/usr/local/bin/ninja"
export MAKE="/usr/local/bin/gmake"
export CMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake"

PROFILE_FILE="$HOME/.profile"
EXPORT_CMD="export"
echo "${EXPORT_CMD} VCPKG_ROOT=\"$HOME/vcpkg\"" >> "$PROFILE_FILE"
echo "${EXPORT_CMD} PATH=\"\$PATH:$HOME/vcpkg\"" >> "$PROFILE_FILE"
echo "${EXPORT_CMD} CMAKE_TOOLCHAIN_FILE=\"$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake\"" >> "$PROFILE_FILE"
echo "${EXPORT_CMD} CMAKE_MAKE_PROGRAM=\"/usr/local/bin/ninja\"" >> "$PROFILE_FILE"
echo "${EXPORT_CMD} MAKE=\"/usr/local/bin/gmake\"" >> "$PROFILE_FILE"

echo "VCPKG_ROOT: $VCPKG_ROOT"
echo "PATH: $PATH"
echo "CMAKE_MAKE_PROGRAM: $CMAKE_MAKE_PROGRAM"
echo "MAKE: $MAKE"
echo "CMAKE_TOOLCHAIN_FILE: $CMAKE_TOOLCHAIN_FILE"
#fi

echo "installing rsync for vs"
pkg install -y rsync
echo "FreeBSD env setup complated."

pkg install -y rsync

echo "Running cron script..."
secondary/setup_cron.sh || { echo "Script failed!"; exit 1; }

echo "2. echo \$VCPKG_ROOT (value should be $HOME/vcpkg)" 
