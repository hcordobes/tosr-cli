tosr-cli
========

TOSR-0n USB Relay CLI application

Intro
-----

The TOSR-0n family of gadgets are a relay array that is controllable via
serial-over-USB, WiFi, Bluetooth and Zigbee. This command line interface
application abstracts the protocol used, by giving an easy access over the CLI, and
makes it easy to include the relay in scripts, by using some shell
return values for the status of the relays.

This implementation only covers the serial-over-USB interface, although
it could be ported to support other interfaces.

Also an optional udev rule is included if more than one ttyUSB is present.

This code can be distributed under the terms of the GPLv3 license. See
LICENSE.
