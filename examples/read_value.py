#!/usr/bin/env python
# coding: utf-8

# Python 2.x compatibility
from __future__ import (
    unicode_literals,
    print_function,
    absolute_import,
    division,
)

# This script requires installing the smbus Python package.
import smbus
import time

DEVICE_BUS = 1
DEVICE_ADDR = 0x42
INTERVAL = 1
REF_VOLTAGE = 5

bus = smbus.SMBus(DEVICE_BUS)


def read_byte(index):
    bus.write_byte(DEVICE_ADDR, index)
    return bus.read_byte(DEVICE_ADDR)


def read():
    xff = read_byte(0)
    msb = read_byte(1)
    lsb = read_byte(2)
    chk = read_byte(3)
    xaa = read_byte(4)
    if xff != 0xff:
        raise IOError("received wrong initiator: 0x{:02x}".format(xff))
    if chk != (msb ^ lsb):
        raise IOError("received wrong checksum: 0x{:02x} != 0x{:02x}".format(
            chk,
            msb ^ lsb,
        ))
    if xaa != 0xaa:
        raise IOError("received wrong terminator: 0x{:02x}".format(xaa))
    return (msb << 8) | lsb


while True:
    try:
        val = read()
        print("Raw value: {} -> voltage with {} V reference: {} V".format(
            val,
            REF_VOLTAGE,
            val * REF_VOLTAGE / 1024,
        ))
    except IOError as e:
        print(repr(e))
    time.sleep(INTERVAL)
