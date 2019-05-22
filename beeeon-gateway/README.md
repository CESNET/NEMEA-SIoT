## TODO

## BeeeOn Gateway on TurrisOS

To install and run BeeeOn Gateway it is necessary to install poco and glib packages first.
```
opkg install poco-all_1.9.0-all-1_mvebu.ipk
opkg install glib2_2.58.3-1_mvebu.ipk
```
>At the time of writing (BGW package v2019.6.1-rc1-1) glib is missing as a BGW dependency, so the BGW can be installed without it, yet an attempt to run it will result in a segmentation fault.

Than install current BGW package.

```
opkg update
opkg install beeeon-gateway_v2019.6.1-rc1-1_mvebu.ipk
```

To get Turris Omnia to recognize ACM devices, which examples include Z-Wave dongles, it is necessary to upgrade kmod* packages, install kmod-usb-acm and reboot the router.
```
opkg update
opkg install $(opkg list-installed | grep kmod | cut -f1 -d" ")
opkg install kmod-usb-acm
reboot
```
> Probably not all kmod* packages need to be upgraded, but it is the best way to ensure it will work and what bad can it bring, right?
