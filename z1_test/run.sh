#!/bin/bash
set -e
make -B

for x in test-*; do
	echo -n $x:
	./$x
done
