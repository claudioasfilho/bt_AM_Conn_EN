#!/bin/sh
make -j11 all
commander flash bt_soc_empty.s37 -s 440179577 -d efr32bg22

