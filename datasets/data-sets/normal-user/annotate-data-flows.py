#!/usr/bin/env python3

import os
import sys
import json

arguments = len(sys.argv)
src_directory = "."
dst_directory = "/home/start/joy-analysis/annotated-data-sets/anomaly-traffic/"


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
                    if fields["dp"] == 443:
                        fields["flow_type"] = "TLS"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == 53:
                        fields["flow_type"] = "DNS"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 22:
                        fields["flow_type"] = "SSH"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["dp"] == 80:
                        fields["flow_type"] = "HTTP"
                        json.dump(fields,dst_file)
                        print(file=dst_file)
                    elif fields["sp"] == 443:
                        fields["flow_type"] = "Incoming TLS"
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
