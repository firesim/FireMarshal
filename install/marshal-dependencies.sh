#!/bin/bash
# set -x

# thanks to Giacomo Rizzi: some ideas have been taken from: https://github.com/gufoe/vuos-tutorial

bare_metal=0

while getopts p: flag
do
	case "${flag}" in
		p) prefix=${OPTARG};;
		b) bare_metal=1;;
	esac
done

INITWD=$(pwd)

# switch to firemarshal/install dir
cd "$(dirname "$0")"
BASE=$(pwd)
echo "BASE: $BASE"

if [ -z ${RISCV} ]
then
	echo "RISCV environment variable NOT FOUND"
	exit 1
fi

# Determine Ubuntu vs Centos
_=$(which apt)
if [ $? -eq 0 ]
then
	os="deb/ubuntu"
else
	_=$(which yum)
	if [ $? -eq 0 ]
	then
		os="centos"
	else
		echo "yum/apt NOT FOUND"
		exit 1
	fi
fi


echo -e "\nInstalling standard packages"
if [ "$os" = "deb/ubuntu" ]
then
	cat ubuntu-requirements.txt | sudo xargs apt-get install -y
elif [ "$os" = "centos" ]
then
	cat centos-requirements.txt | sudo xargs yum install -y
fi

echo -e "\nInstalling python dependencies"
pip3 install --user -r python-requirements.txt

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
	NAME=$2
	PREWD=$(pwd)
	echo installing $1
	cd  "$BASE"/dependencies
	mkdir $NAME
	wget $REPO -O $NAME.tar.gz
	tar -xvzf $NAME.tar.gz -C $NAME --strip-components=1
	cd $NAME

	echo CMAKE
	mkdir -p build
	cd build
	if [ -z "$prefix" ]
	then
		cmake .. $2
		make
	else
		if [ "$os" = "deb/ubuntu" ]
		then
			MARSHAL_PREFIX=$prefix CC=$BASE/gccwrap-ubuntu $prefix/bin/cmake .. -DCMAKE_PREFIX_PATH=$prefix -DCMAKE_INSTALL_PREFIX=$prefix
			C_INCLUDE_PATH="$prefix/include" LIBRARY_PATH="$prefix/lib" make
		else
			MARSHAL_PREFIX=$prefix CC=$BASE/gccwrap-centos $prefix/bin/cmake .. -DCMAKE_PREFIX_PATH=$prefix -DCMAKE_INSTALL_PREFIX=$prefix
			C_INCLUDE_PATH="$prefix/include" LIBRARY_PATH="$prefix/lib64" make
		fi
	fi
	make install


	cd $PREWD
}

function install_cmake {
	PREWD=$(pwd)
	version=3.23
	build=0
	echo installing cmake-$version.$build
	cd "$BASE"/dependencies
	wget https://github.com/Kitware/CMake/releases/download/v$version.$build/cmake-$version.$build-linux-x86_64.sh
	if [ -z "$prefix" ]
	then
		bash cmake-$version.$build-linux-x86_64.sh --prefix="/usr/local/bin" --exclude-subdir --skip-license 
	else
		bash cmake-$version.$build-linux-x86_64.sh --prefix=$prefix --exclude-subdir --skip-license 
	fi
	cd $PREWD
}

function install_meson {
	PREWD=$(pwd)
	release=0.61.4
	echo installing meson-$release
	cd "$BASE"/dependencies
	mkdir meson
	wget https://github.com/mesonbuild/meson/releases/download/$release/meson-$release.tar.gz -O meson.tar.gz
	tar -xzvf meson.tar.gz -C meson --strip-components=1
	cd $PREWD
}

function install_ninja {
	PREWD=$(pwd)
	release=1.10.2
	echo installing ninja-$release
	cd "$BASE"/dependencies
	wget https://github.com/ninja-build/ninja/releases/download/v$release/ninja-linux.zip
	if [ -z "$prefix" ]
	then
		unzip -d /usr/local/bin/ ninja-linux.zip
	else
		unzip -d $prefix/bin/ ninja-linux.zip
	fi
	cd $PREWD
}

function install_libslirp {
	PREWD=$(pwd)
	release=4.6.1
	echo installing libslirp
	cd "$BASE"/dependencies
	mkdir libslirp
	wget https://gitlab.freedesktop.org/slirp/libslirp/-/archive/v$release/libslirp-v$release.tar.gz -O libslirp.tar.gz
	tar -xvzf libslirp.tar.gz -C libslirp --strip-components=1
	cd libslirp
	if [ -z "$prefix" ]
	then
		python3.8 ../meson/meson.py build
		ninja -C build install
	else
		NINJA=$prefix/bin/ninja python3.8 ../meson/meson.py build -Dprefix=$prefix
		$prefix/bin/ninja -C build install
	fi
	cd $PREWD
}

function install_qemu {
	PREWD=$(pwd)
	release=5.0.0
	echo installing qemu-$release
	cd  "$BASE"/dependencies
	mkdir qemu
	wget https://download.qemu.org/qemu-$release.tar.xz -O qemu.tar.xz
	tar -xvJf qemu.tar.xz -C qemu --strip-components=1
	cd qemu

	mkdir -p build
	cd build
	if [ -z "$prefix" ]
	then
		../configure --target-list=riscv64-softmu --enable-vde --disable-werror
		make
	else
		if [ "$os" = "deb/ubuntu" ]
		then
			MARSHAL_PREFIX=$prefix ../configure --prefix=$prefix --interp-prefix=$prefix --cc=$BASE/gccwrap-ubuntu --target-list=riscv64-softmmu --enable-vde --disable-werror --extra-ldflags="-Wl,-rpath=$prefix/lib"
			C_INCLUDE_PATH="$prefix/include" LIBRARY_PATH="$prefix/lib" make
		else
			MARSHAL_PREFIX=$prefix ../configure --prefix=$prefix --interp-prefix=$prefix --cc=$BASE/gccwrap-centos --target-list=riscv64-softmmu --enable-vde --disable-werror --extra-ldflags="-Wl,-rpath=$prefix/lib -Wl,-rpath=$prefix/lib64"
			C_INCLUDE_PATH="$prefix/include" LIBRARY_PATH="$prefix/lib64" make
		fi
	fi
	make install


	cd $PREWD
}

# Start installation

if [ $bare_metal -eq 0 ]
then
	
	rm -rf dependencies
	mkdir dependencies

	if [ -z "$prefix" ]
	then
		prefix=${RISCV}
	fi
	
	# vde packages need cmake >3.13
	install_cmake

	# meson needs Python 3.8, not available on CentOS 7 w/o scl
	if [ "$os" = "centos" ]
	then
		sudo yum install -y centos-release-scl
		sudo yum install -y rh-python38
        source /opt/rh/rh-python38/enable
	fi

	install_ninja
	install_meson
	install_libslirp
	
	# we're done with meson
	if [ "$os" = "centos" ]
	then
		sudo yum remove -y rh-python38-python
	fi

	install_repo https://github.com/virtualsquare/s2argv-execs/archive/refs/tags/1.3.tar.gz s2argv-execs
	install_repo https://github.com/rd235/vdeplug4/archive/refs/tags/v4.0.1.tar.gz vdeplug4
	install_repo https://github.com/virtualsquare/libvdeslirp/archive/refs/tags/0.1.1.tar.gz libvdeslirp
	install_repo https://github.com/virtualsquare/vdeplug_slirp/archive/refs/tags/0.1.0.tar.gz vdeplug_slirp

	install_qemu

	cd .. && ./init-submodules.sh
fi

echo 'Installation completed'
cd $INITWD
