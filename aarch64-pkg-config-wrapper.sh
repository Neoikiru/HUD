#!/bin/bash
export PKG_CONFIG_DIR=""
export PKG_CONFIG_LIBDIR=/home/neo/rpi_sysroot/usr/lib/aarch64-linux-gnu/pkgconfig:/home/neo/rpi_sysroot/usr/share/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=/home/neo/rpi_sysroot
exec pkg-config "$@"
