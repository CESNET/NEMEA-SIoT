#!/usr/bin/env python3

import os
import sys
import json

arguments = len(sys.argv)
src_directory = "."
dst_directory = "/home/start/joy-analysis/annotated-data-sets/ikea-homekit/"


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
                    if fields["dp"] == 5353 and fields["sp"] == 5353:
                        fields["flow_type"] = "MDNS"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == None and fields["sp"] == None and fields["ip"]["out"]["ttl"] == 1:
                        fields["flow_type"] = "Memebership Report Group"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 80 and fields["ip"]["in"]["ttl"] == 128:
                        fields["flow_type"] = "Homekit Data"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == None and fields["sp"] == None and fields["ip"]["out"]["ttl"] != 1:
                        fields["flow_type"] = "Port Unreachable"
                        json.dump(fields,dst_file)
                        print(file=dst_file)

                    elif fields["dp"] == 5684 and fields["ip"]["in"]["ttl"] == 255:
                        fields["flow_type"] = "Ikea Application Data"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == 5684 and fields["ip"]["out"]["ttl"] == 255:
                        fields["flow_type"] = "Ikea Port Test"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["ip"]["out"]["ttl"] == 128 and fields["dp"] == 443:
                        fields["flow_type"] = "TLS Webhook Ikea Cloud"
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
