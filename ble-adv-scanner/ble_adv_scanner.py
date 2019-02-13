#!/usr/bin/env python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4 fileencoding=utf-8 :

# Copyright (C) 2019 CESNET
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

""" ble_adv_scanner: Scans advertising channels of Bluetooth Low Energy via HCI enabled controllers."""

__author__      = "Ondřej Hujňák"
__copyright__   = "Copyright 2019, CESNET"
__version__     = "1.0"

import PyBT_tools as btool
import pytrap

import argparse
import signal
import sys

from scapy.layers.bluetooth import *


unirecSpec = "macaddr BDADDR, time TIME, int8 RSSI, uint8 ATYPE"

Terminator = False

def terminate(signum, frame):
    global Terminator
    Terminator = True

def trap_init(argv):
    """ Sets up trap interface for output with UniRec """
    trap = pytrap.TrapCtx()
    trap.init(argv, 0, 1)
    trap.ifcctl(0, False, pytrap.CTL_BUFFERSWITCH, 0) # Disable output buffering
    trap.setDataFmt(0, pytrap.FMT_UNIREC, unirecSpec)
    return trap

class ADV_Scanner(object):
    """ Scanner for checking BLE devices available in range on advertising channels. """

    bts = None
    running = False

    def __init__(self):
        self.running = False

        # Set up Bluetooth interface
        self.bts = btool.BTStack()

    def getDeviceReports(self):
        """ Generator function, which yields pakets with Device Reports from BLE scan """

        if not self.isRunning():
            self.start()

        global Terminator
        while not Terminator:
            pkt = self.bts.recv()
            if HCI_LE_Meta_Advertising_Report in pkt:
                yield pkt


    def isRunning(self):
        return self.running

    def setPassiveMode(self):
        """ Sets scanning mode to be passive - i.e. just listen, do not send SEND_REQ packets """
        
        command = HCI_Hdr()/HCI_Command_Hdr()/HCI_Cmd_LE_Set_Scan_Parameters(type=0) # type=0 means passive scanning
        self.bts.send(command)
        while True:
            pkt = self.bts.recv()
            if HCI_Event_Command_Complete in pkt:
                if (pkt.opcode == command.opcode):
                    if (pkt.status is not 0): # Command successfull
                        print("Setting scan parameters failed.", file=sys.stderr)
                        return 1
                    else:
                        print("Passive scanning successfully set.")
                        break

    def start(self):
        """ Starts scanning and keep reporting even duplicite BDADDRs """
    
        command = HCI_Hdr()/HCI_Command_Hdr()/HCI_Cmd_LE_Set_Scan_Enable(filter_dups=0) # Do not filter duplicites
        self.bts.send(command)
        while True:
            pkt = self.bts.recv()
            if HCI_Event_Command_Complete in pkt:
                if (pkt.opcode == command.opcode):
                    if (pkt.status is not 0): # Command successfull
                        print("Could not start scanning.", file=sys.stderr)
                        return 2
                    else:
                        self.running = True
                        print("LE scanning started.")
                        break

    def stop(self):
        """ Stops scanning """

        command = HCI_Hdr()/HCI_Command_Hdr()/HCI_Cmd_LE_Set_Scan_Enable(enable=0)
        self.bts.send(command)
        while True:
            pkt = self.bts.recv()
            if HCI_Event_Command_Complete in pkt:
                if (pkt.opcode == command.opcode):
                    if (pkt.status is not 0): # Command successfull
                        print("Stopping scanning scanning.", file=sys.stderr)
                        return 2
                    else:
                        self.running = False
                        print("LE scanning stopped.")
                        break

# -----------------------------------------------------------------------------


def main(argv):
    parser = argparse.ArgumentParser(description=""" BLE Advertisements Scanner (NEMEA module)
    Inputs: 0
    Outputs: 1 (UniRec: macaddr BDADDR, time TIME, int8 RSSI, uint8 ATYPE)

    This module sets up HCI connected Bluetooth controller for passive scanning
    and reports every device which appears on advertising channels. """)
    parser.add_argument('-i', metavar='IFC_SPEC',
        required=True, help='TRAP interface specifier.',)
    parser.formatter_class = argparse.RawTextHelpFormatter
    args = parser.parse_args()
    
    # Set up SIGINT handler for graceful halting
    signal.signal(signal.SIGINT, terminate)

    trap = trap_init(argv)

    scanner = ADV_Scanner()
    scanner.setPassiveMode()
    scanner.start()

    for pkt in scanner.getDeviceReports():
        report = pkt[HCI_LE_Meta_Advertising_Report]
        
        rec = pytrap.UnirecTemplate(unirecSpec)
        rec.createMessage()
        
        rec.BDADDR = pytrap.UnirecMACAddr(report.addr)
        rec.TIME   = pytrap.UnirecTime(pkt.time)
        rec.RSSI   = report.rssi
        rec.ATYPE  = report.atype

        trap.send(rec.getData())

    scanner.stop()

    # Clean up NEMEA
    trap.finalize()

if __name__ == "__main__":
    main(sys.argv)
