#!/bin/bash

if [ "$1" = "" ]; then
	echo "SYNTAX: compare binary_file"
	echo "	compare result of PFG and CFG"
fi

./test_pfg $1 2> log.out | sort | uniq > pfg.out || exit 1
./out_cfg $1 2>> log.out | sort | uniq > cfg.out || exit 1

diff pfg.out cfg.out || (echo "RESULT: failure."; exit 1)
echo "RESULT: equality !"

