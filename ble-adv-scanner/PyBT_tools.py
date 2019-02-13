# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4 fileencoding=utf-8 :

# Copyright (C) 2013-2016 CESNET
#
# LICENSE TERMS
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name of the Company nor the names of its contributors
#    may be used to endorse or promote products derived from this
#    software without specific prior written permission.
#
# ALTERNATIVELY, provided that this notice is retained in full, this
# product may be distributed under the terms of the GNU General Public
# License (GPL) version 2 or later, in which case the provisions
# of the GPL apply INSTEAD OF those given above.
#
# This software is provided ``as is'', and any express or implied
# warranties, including, but not limited to, the implied warranties of
# merchantability and fitness for a particular purpose are disclaimed.
# In no event shall the company or contributors be liable for any
# direct, indirect, incidental, special, exemplary, or consequential
# damages (including, but not limited to, procurement of substitute
# goods or services; loss of use, data, or profits; or business
# interruption) however caused and on any theory of liability, whether
# in contract, strict liability, or tort (including negligence or
# otherwise) arising in any way out of the use of this software, even
# if advised of the possibility of such damage.

# This library was inspired by Mike Ryan (https://github.com/mikeryan/PyBT) and others

""" PyBT_tools: Simple library for interfacing with Bluetooth controllers. Uses Scapy project for packet construction."""

__author__      = "Ondřej Hujňák"
__copyright__   = "Copyright 2019, CESNET"
__version__     = "0.1"

import os
import socket
import sys

from fcntl import ioctl
from scapy.layers.bluetooth import *

# ioctl HCI Constants: https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/lib/hci.h
IOCTL_HCIDEVUP       = 0x400448c9  # _IOW('H', 201, int)
IOCTL_HCIDEVDOWN     = 0x400448ca  # _IOW('H', 202, int)
IOCTL_HCIDEVRESET    = 0x400448cb  # _IOW('H', 203, int)

class HCIConfig(object):
    """
    HCI Config class is used for configuration of Bluetooth adapters.
    """

    @staticmethod
    def up(iface):
        sock = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_RAW, socket.BTPROTO_HCI)
        ioctl(sock.fileno(), IOCTL_HCIDEVUP, iface)
        sock.close()
        return True


    @staticmethod
    def down(iface):
        sock = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_RAW, socket.BTPROTO_HCI)
        ioctl(sock.fileno(), IOCTL_HCIDEVDOWN, iface)
        sock.close()
        return True


    @staticmethod
    def reset(iface):
        sock = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_RAW, socket.BTPROTO_HCI)
        ioctl(sock.fileno(), IOCTL_HCIDEVRESET, iface)
        sock.close()
        return True


class BTStack:
    sock    = None
    bdaddr  = None
    bdaddr_rand = None
    interval = (None, None)  # interval (min, max)
    connected = False


    def __init__(self, adapter=0):
        self.sock = self.get_socket(adapter)

        # set up the device
        self.command(HCI_Cmd_Reset())

        # get BDADDR
        res = self.command(HCI_Cmd_Read_BD_Addr())
        self.bdaddr = res[HCI_Cmd_Complete_Read_BD_Addr].addr


    def __del__(self):  # TODO: Clean up properly
        if self.sock is not None and not self.sock.closed:
            #close the socket
            self.sock.close()
            # Still opened thanks to the bug in Bluetooth sockets (https://github.com/secdev/scapy/issues/191)
            # TODO: Try it in C, if socket properly closes, then use libc functions accordingly


    def command(self, command):
        return self.sock.send_command(HCI_Hdr()/HCI_Command_Hdr()/command)


    def connect(self, addr, type):
        """ Create connection with address addr. The type can be 0 for public address or 1 for random one. """

        if self.connected:
            print("[+] Already connected", file=sys.stderr)
            return

        connection_cmd = HCI_Hdr()/HCI_Command_Hdr()/HCI_Cmd_LE_Create_Connection(paddr=addr, patype=type)
        if self.interval[0] is not None and self.interval[1] is not None:
            connection_cmd[HCI_Cmd_LE_Create_Connection].min_interval = self.interval[0]
            connection_cmd[HCI_Cmd_LE_Create_Connection].max_interval = self.interval[1]
            #TODO: unverified code!! Run some tests!
        
        # can't use send_command() because reply is a Command Status (0x0f) and not Command Complete (0x0e)
        self.sock.send(connection_cmd)
        while True:
            pkt = self.sock.recv()
            if HCI_Event_Command_Status in pkt:
                if (pkt.opcode == connection_cmd.opcode):
                    if p.status == 0:
                        self.connected = True
                        break
                    else:
                        raise BluetoothSocketError("Problem establishing connection")

        time.sleep(.5) # TODO: Ugly hack, remove when found why necessary


    def disconnect(self):  # TODO: Fix disconnection, now it seems dirty, cannot reconnect afterwards
        if self.connected:
            self.sock.send(HCI_Hdr()/HCI_Command_Hdr()/HCI_Cmd_Disconnect())
            self.connected = False


    def get_socket(self, adapter):
        try:
            return BluetoothUserSocket(adapter)
        except BluetoothSocketError as e:
            print("[!] Creating socket failed: " + str(e), file=sys.stderr)
            if os.getuid() > 0:
                print("[!] Are you definitely root? detected uid: " + str(os.getuid()), file=sys.stderr)
            else:
                print("[+] Attempting to take iface down", file=sys.stderr)
                try:
                    HCIConfig.down(adapter)
                    try:
                        return BluetoothUserSocket(adapter)
                    except BluetoothSocketError as e:
                        print("[!] Creating socket failed: " + str(e), file=sys.stderr)
                        print("[!] Giving up.", file=sys.stderr)
                except OSError as e:
                    print("[!] Taking iface down failed: " + str(e), file=sys.stderr)
                    print("[!] Giving up.", file=sys.stderr)
        sys.exit(1)
 
    def send(self, packet):
        return self.sock.send(packet)

    def send_att(self, data, gatt_handle):
        if isinstance(data, str):
            data = bytes.fromhex(data)
        elif isinstance(data, int):
            data = bytes.fromhex(str(data))
        
        returnvalue = self.sock.send(HCI_Hdr()/HCI_ACL_Hdr(handle=64)/L2CAP_Hdr()/ATT_Hdr()/ATT_Write_Request(gatt_handle=gatt_handle)/data)
        while True:
            p = self.sock.recv()
            if p.type == 0x4:
                if p.code == 0x13:
                    break                    
       
        return returnvalue

    def send_raw(self, data):
        returnvalue = self.sock.send(data);
        while True:
            p = self.sock.recv()
            if p.type == 0x4:
                if p.code == 0x13:
                    break                    
       
        return returnvalue

    def recv(self):
        return self.sock.recv()
