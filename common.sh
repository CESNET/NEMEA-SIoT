#!/bin/sh

function install {
  pkg=`find . -regex ".*/$1[^/]*\.ipk"`
  if [ -z "$pkg" ]; then
    echo "WARNING: $pkg not found!"
    return
  fi
  opkg install $pkg
}
