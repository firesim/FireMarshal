#!/bin/bash
#set -x

# thanks to Giacomo Rizzi: some ideas have been taken from: https://github.com/gufoe/vuos-tutorial

BASE="$(dirname "$0")"
cd $BASE
BASE=$(pwd)
echo "BASE: $BASE"

# User friendly messages on error
set -E
set -o functrace
function handle_error {
    local retval=$?
    local line=${last_lineno:-$1}
    echo "Failed at $line: $BASH_COMMAND"
    echo "Trace: " "$@"
    exit $retval
}
if (( ${BASH_VERSION%%.*} <= 3 )) || [[ ${BASH_VERSION%.*} = 4.0 ]]; then
	trap '[[ $FUNCNAME = handle_error ]] || { last_lineno=$real_lineno; real_lineno=$LINENO; }' DEBUG
fi
trap 'handle_error $LINENO ${BASH_LINENO[@]}' ERR

function install_repo {
	REPO=$1
	REPOBASE=${REPO##*/}
	REPOBASE=${REPOBASE%%.*}
	PREWD=$(pwd)
	echo installing $1
	cd  "$BASE"/gits
	git clone --recurse-submodules $1
	cd $REPOBASE
	if [ -f configure.ac ]
	then
		echo AUTOCONF
		autoreconf -vif
		./configure $2
	elif [ -f CMakeLists.txt ] 
	then
		echo CMAKE
		mkdir -p build
		cd build
		cmake .. $2
	else
		echo UNKNOWN
		exit 1
	fi
	make
	make install
	ldconfig
	cd $PREWD
}

function install_cmake {
	PREWD=$(pwd)
	version=3.15
	build=0
	echo installing cmake-$version.$build
	cd "$BASE"/gits
	wget https://cmake.org/files/v$version/cmake-$version.$build.tar.gz
	tar -xzvf cmake-$version.$build.tar.gz

	cd cmake-$version.$build/
	./bootstrap
	make -j$(nproc)
	make install
	cd $PREWD
}

function pip_install {
	package=$1
	PREWD=$(pwd)
	echo installing $package
	python3 -m pip install $1
	#pip3 install $1
	#cd ~/.local/bin
	#mv $1 /usr/local/bin
	#cd $PREWD
}

function install_libslirp {
	echo installing libslirp
	PREWD=$(pwd)
	cd "$BASE"/gits
	git clone https://gitlab.freedesktop.org/slirp/libslirp.git
	cd libslirp
	meson build
	sudo ninja -C build install
	cd $PREWD
}


# Start installation

sudo apt-get --yes install git python3 python3-pip build-essential make autogen autoconf libtool netcat udhcpc vde2 libexecs-dev libcap-dev libpcap-dev libvdeplug-dev

install_cmake
pip_install ninja
pip_install meson
install_libslirp

install_repo https://github.com/virtualsquare/vde-2.git -enable-experimental
install_repo https://github.com/rd235/vdeplug4.git
install_repo https://github.com/rd235/libvdestack.git
install_repo https://github.com/virtualsquare/vdeplug_vlan.git
install_repo https://github.com/virtualsquare/libvdeslirp.git
install_repo https://github.com/virtualsquare/vdeplug_slirp.git
install_repo https://github.com/virtualsquare/vdeplug_pcap.git
#randmac has additional deps
install_repo https://github.com/rd235/strcase.git
install_repo https://github.com/virtualsquare/randmac.git

echo 'Installation completed'
