#!/usr/bin/env python3

import os
import sys
import json

arguments = len(sys.argv)
src_directory = "."
dst_directory = "../../annotated-data-sets/ipcam/"


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
                    if fields["dp"] == 139:
                        fields["flow_type"] = "NetBIOS"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 445:
                        fields["flow_type"] = "SMB (Local NAS Connection)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 1900 and fields["sp"] != 1900:
                        fields["flow_type"] = "SSDP (m-search)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 1900 and fields["sp"] == 1900:
                        fields["flow_type"] = "SSDP (notify)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 55443:
                        fields["flow_type"] = "TLS (myedimax.com)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 8760:
                        fields["flow_type"] = "UDP myedimax.com Init"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == 53:
                        fields["flow_type"] = "DNS (google.com/myedimax.com)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 9765 or fields["sp"] == 28844:
                        fields["flow_type"] = "UDP Heartbeat myedimax.com"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == fields["sp"] and fields["da"] == "255.255.255.255":
                        fields["flow_type"] = "Local Init Broadcast"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 80:
                        fields["flow_type"] = "HTTP (Test Connection To google.com)"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == None and fields["dp"] == None:
                        fields["flow_type"] = "Membership Report Group"
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
