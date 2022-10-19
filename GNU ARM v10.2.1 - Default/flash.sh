#!/bin/sh
make -j11 all
commander flash bt_AM_Conn_EN.s37 -s 440179577 -d efr32bg22

