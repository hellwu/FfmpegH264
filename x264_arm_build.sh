#!/bin/bash

NDK=/home/hellw/android-ndk-r10e

PLATFORM=$NDK/platforms/android-21/arch-arm
TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86
PREFIX=./android/arm

function build_one
{
./configure \
--prefix=$PREFIX \
--enable-shared \
--enable-static \
--enable-pic \
--enable-strip \
--enable-thread \
--disable-asm \
--host=arm-linux-androideabi \
--cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
--sysroot=$PLATFORM \
--extra-cflags="-Os -fpic" \
--extra-ldflags="" \

$ADDITIONAL_CONFIGURE_FLAG
make clean
make -j4
make install
}
build_one

