#!/bin/sh
# FreeBSD için C++ geliştirme ortamını hazırlama scripti
NEED_VCPKG=false

# Temel geliştirme araçlarını yükle #llvm15
pkg install -y cmake git bash autoconf automake libtool gmake

# Ninja'nın yeni sürümünü portlardan kuruyoruz (pkg versiyonu yerine)
pkg install -y pkgconf
cd /tmp
fetch https://github.com/ninja-build/ninja/archive/refs/tags/v1.12.1.tar.gz
tar -xf v1.12.1.tar.gz
cd ninja-1.12.1
cmake -B build
cmake --build build
cmake --install build

# Ninja'nın sistem PATH'inde olduğundan emin ol
which ninja || ln -sf /usr/local/bin/ninja /usr/bin/ninja

# gmake'in sistem PATH'inde olduğundan emin ol
which gmake || ln -sf /usr/local/bin/gmake /usr/bin/gmake

# vcpkg kurmak ve hazırlamak için
if [ "$NEED_VCPKG" = true ]; then
	# vcpkg için gerekli araçlar
	pkg install -y zip unzip curl
	echo "vcpkg kuruluyor..."
	git clone https://github.com/Microsoft/vcpkg.git "$HOME/vcpkg"
	cd "$HOME/vcpkg"
	./bootstrap-vcpkg.sh -disableMetrics

	#echo "${EXPORT_CMD} CMAKE_MAKE_PROGRAM=\"/usr/local/bin/ninja\"" >> "$PROFILE_FILE"
	#echo "${EXPORT_CMD} MAKE=\"/usr/local/bin/gmake\"" >> "$PROFILE_FILE"
else #sistemi kullanalim.
	pkg install -y boost-all nlohmann-json stb
fi

# rsync kur (Visual Studio için gerekli)
pkg install -y rsync

echo "FreeBSD geliştirme ortamı kurulumu tamamlandı!"
