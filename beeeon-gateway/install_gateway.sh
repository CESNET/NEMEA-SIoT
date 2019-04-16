#!/bin/sh

glib2=`find . -regex ".*/glib2[^/]*\.ipk"`
poco=`find . -regex ".*/poco[^/]*\.ipk"`
gateway=`find . -regex ".*/beeeon-gateway[^/]*\.ipk"`

function install_glib2 {
  opkg install $glib2
}

function install_poco {
  opkg install $poco
}

function install_gateway {
  echo "updating opgk"
  opkg update > /dev/null 2>&1 
  opkg install $gateway
}

install_glib2
install_poco
install_gateway
