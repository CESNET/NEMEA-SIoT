IoT Datasets
===============
This folder includes IoT datasets for treat detection and ML trainig purposes. Extended flow data was created by [Joy tool](https://github.com/cisco/joy). Used initial parameters you can see in bash scripts.

Datasets for each device/protocol are stored in seperate folders. You can see their description [below](#dataset-description). Folders [data-sets](data-sets) and [annotated-data-sets](annotated-data-sets) are almost the same. The only difference is that flows in [annotated-data-sets](annotated-data-sets) folder have field "flow type" that was added by script "annotate-data-flow.py". This script is part of each dataset group.

# Dataset Description
## IKEA APP
The IKEA gateway itself doesn’t allow remote communication with clients. All control commands from IKEA mobile have to be sent from the same broadcast domain. 

After plugging the gateway in, it establish one HTTP connection for firmware version check (fw.ota.homesmart.ikea.net) and one TLS connection with IKEA cloud service (webhook.logentries.com). Before TLS and HTTP connection, there is of course DNS queries for host names. After that phase, the gateway is ready to process user commands. If the Internet connection is not available IKEA gateway will be trying to resolve DNS name fw.ota.homestart.ikea.net, however, user commands will work. Once several dozens of seconds, the gateway sends multicast membership report group message and MDNS queries.

Each command in IKEA application generate the specific sequence of packet length. Playloads of these packet is encrypted, however, the sequence of packet lengths is still the same for one command. Also, the packet lenghts and times are very similar during IKEA gateway and application initialization. All application data are send to port 5684. If this port is used as source in a new flow, IKEA gateway is testing availability of a client. This usually happens during application re/initialization. 


## IKEA Homekit
In this case, IKEA gateway is still the same however different communication protocol is used. Because of the same gateway, the booting process after power on is also the same. 
The main difference is on application layer. For communication is used port 80 instead of 5684. Homekit communication is also fully encrypted.

## Normal User Traffic
This type of data set was created for testing purposes. It includes valid HTTP/S, SSH and DNS traffic that was captured from local PC client communication.

## IP Cam
For this dataset Edimax IC-9110W was used. This camera has two modes: cloud or local. In the cloud mode, you are able to connect to the video stream from mobile app from remote and local sites. In the local mode, you are unable to connect to the camera video stream from remote sites without VPN. 

In the cloud mode, the IP camera establishes a TLS connection to myedimax.com on port 55443. For this connection is then sent UDP heartbeat to myedimax.com every 20 seconds to port 9765. Also, every 60 seconds is tested HTTP connection to google.com. Except cloud communication, there is the local one. The video stream from the IP camera is store to local NAS via NetBIOS on port 139 or via SMB on port 445. Simultaneously, once several dozens of seconds the IP camera sends multicast membership report group message. Several local flows are represented by SSDP protocol. There are two types SSDP notify and SSDP m-search.

The local IP camera mode is the same as the cloud mode but the local mode missing the external communication with myedimax.com, however, HTTP connection test for google.com is still here.
In the data set, there are also examples of anomaly traffic for both cloud and local modes. In both cases, the local NAS is down or video recording is stopped, so the flows has different behaviour. The IP camera is still trying to connect to the NAS server or no SMB/NetBIOS traffic is available.

## Voice Assistant
For this category Google Mini assistant was used. After powering on, Google Mini establishes connections with the following services:
- clents1.google.com
- time.google.com (ntp)
- connectivitycheck.gstatic.com
- tools.google.com
- www.gstatic.com
- mtalk.google.com
- clients3.google.com
- android.googleapis.com
- www.google.com
- clients4.google.com

For mtalk.google.com is used destination port 5228 and for time.google.com (NTP) is used port 123. For the rest services is used TLS port 443 or HTTP port 80 for connectivity check. From local point of view, there are just a few types of flows like multicast membership report group, MDNS and ICMP  messages. If Google Mini is in stand by mode, during which it waits for "Hey Google!" command, no external communication is sent.
