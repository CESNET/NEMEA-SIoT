#!/bin/sh

function install_siot_modules {
  opkg install `find . -regex ".*/siot-[^/]*\.ipk"`
}

install_siot_modules
