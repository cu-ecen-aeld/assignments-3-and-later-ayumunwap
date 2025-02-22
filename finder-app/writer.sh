#!/bin/bash
set -e
set -u

# the first argument is a full path to a file (including filename) on the filesystem
writefile=$1
# the second argument is a text string which will be written within this file
writestr=$2

if [ $# -lt 2 ]; then
	echo "One or more of the specified arguments are not specified"
	exit 1
else
	mkdir -p "$(dirname $writefile)" && echo $writestr > $writefile
	if [ $? -ne 0 ]; then
		echo "The file could not be created"
		exit 1
	fi
fi
