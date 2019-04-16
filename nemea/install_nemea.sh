#!/bin/sh

framework=`find . -regex ".*/nemea-framework[^/]*\.ipk"`
supervisorl=`find . -regex ".*/nemea-supervisorl[^/]*\.ipk"`
logreplay=`find . -regex ".*/nemea-logreplay[^/]*\.ipk"`
logger=`find . -regex ".*/nemea-logger[^/]*\.ipk"`

function install_nemea_framework {
  opkg install $framework
}

function install_nemea_supervisorl {
  opkg install $supervisorl
}

function install_nemea_logreplay {
  opkg install $logreplay
}

function install_nemea_logger {
  opkg install $logger
}

install_nemea_framework
install_nemea_supervisorl
install_nemea_logreplay
install_nemea_logger
