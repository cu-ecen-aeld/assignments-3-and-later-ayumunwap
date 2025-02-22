#!/bin/sh
set -e
set -u

# the first argument is a path to a directory on the filesystem
filesdir=$1
# the second argument is a text string which will be searched within these files
searchstr=$2

if [ $# -lt 2 ]; then
	echo "One or more of the specified arguments are not specified"
	exit 1
else
	if [ ! -d $filesdir ]; then
		echo "The second argument does not represent a directory on the filesystem"
		exit 1
	else
		# X is the number of files in the directory and all subdirectories
		X=`find $filesdir -type f | wc -l`
		# Y is the number of matching lines found in respective files
		Y=`grep -r $searchstr $filesdir | wc -l`	
		echo "The number of files are ${X} and the number of matching lines are ${Y}"
	fi
fi
