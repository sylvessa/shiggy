#!/bin/bash

cyan="\033[0;36m"
green="\033[0;32m"
yellow="\033[1;33m"
reset="\033[0m"

binv=2.40
gccv=13.2.0
target=i386-elf
prefix="$(pwd)/bin/bin"

# put to path temporary
export PATH="$prefix:$PATH"

# check if already exists
if command -v $target-gcc >/dev/null 2>&1 && command -v $target-ld >/dev/null 2>&1; then
	echo -e "${green}toolchain already built${reset}"
	echo -e "${yellow}path is set, run make in shiggy${reset}"
	exit 0
fi

echo -e "${cyan}make dirs${reset}"
mkdir -p toolchain
mkdir -p bin
cd toolchain

echo -e "${cyan}install deps${reset}"
sudo apt install wget build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo -y

echo -e "${cyan}download src${reset}"
wget https://ftp.wayne.edu/gnu/binutils/binutils-$binv.tar.gz
wget https://ftp.wayne.edu/gnu/gcc/gcc-$gccv/gcc-$gccv.tar.gz

echo -e "${cyan}extract src${reset}"
echo -e "${green}extracting binutils${reset}"
tar -xvzf binutils-$binv.tar.gz
echo -e "${green}extracting gcc${reset}"
tar -xvzf gcc-$gccv.tar.gz

echo -e "${cyan}build binutils${reset}"
mkdir -p build-binutils
cd build-binutils
../binutils-$binv/configure --target=$target --prefix=$prefix --with-sysroot --disable-nls --disable-werror
make -j$(nproc) V=1
make install
cd ..

echo -e "${cyan}fetch gcc deps${reset}"
cd gcc-$gccv
./contrib/download_prerequisites
cd ..

echo -e "${cyan}build gcc (c only)${reset}"
mkdir -p build-gcc
cd build-gcc
../gcc-$gccv/configure --target=$target --prefix=$prefix --disable-nls --enable-languages=c --without-headers
make all-gcc -j$(nproc) V=1
make install-gcc
cd ..
rm -rf toolchain


echo
echo -e "${green}toolchain built${reset}"
echo -e "${yellow}path is set now, run make in shiggy${reset}"
