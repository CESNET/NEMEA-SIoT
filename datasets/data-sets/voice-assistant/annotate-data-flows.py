#!/usr/bin/env python3

import os
import sys
import json

arguments = len(sys.argv)
src_directory = "."
dst_directory = "../../annotated-data-sets/voice-assistant/"


# read all files in subdirectories
for root, dirs, files in os.walk(src_directory):
    # Skip parent directories
    if dirs:
        continue
    for file in files:
        # open files in src and dst directories
        with open(root+"/"+file,encoding="utf-8", mode="r") as flows, open(dst_directory+root[2:]+"/"+file,encoding="utf-8",mode="w+") as dst_file:
            for flow in flows:
                # anotate specific flows
                fields = json.loads(flow)
                try:
                    if fields["dp"] == 8009 and fields["da"] == "192.168.3.110":
                        fields["flow_type"] = "Local Client TCP Scan"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sa"] == "192.168.3.109" or fields["sa"] == "192.168.3.164":
                        fields["flow_type"] = "Local Client Communication"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == 67 and fields["dp"] == 68:
                        fields["flow_type"] = "DHCP"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == 68 and fields["dp"] == 67:
                        fields["flow_type"] = "DHCP (Partial)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == 53:
                        fields["flow_type"] = "DNS (Google Services)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == None and fields["dp"] == None and fields["da"].startswith("2"):
                        fields["flow_type"] = "Membership Report Group"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == None and fields["dp"] == None and not fields["da"].startswith("2"):
                        fields["flow_type"] = "ICMP"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == 5353 and fields["dp"] == 5353:
                        fields["flow_type"] = "MDNS"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 5228:
                        fields["flow_type"] = "TLS (Mtalk Service)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == 5228:
                        fields["flow_type"] = "TLS (Mtalk Service - Missing TLS Connectivity Establishment)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 443:
                        fields["flow_type"] = "TLS (Google Services)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == 443:
                        fields["flow_type"] = "TLS (Google Services - Missing TLS Connectivity Establishment)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 123:
                        fields["flow_type"] = "NTP"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 80:
                        fields["flow_type"] = "HTTP (Connectivity Check)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == 80:
                        fields["flow_type"] = "HTTP (Connectivity Check - Missing Flow Beginning)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)

                    else:
                        fields["flow_type"] = "Unknown"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                except Exception as e:
                        fields["flow_type"] = "Flow Error"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
