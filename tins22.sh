#!/bin/sh

#accessing through symlink?
if [ -h $0 ]
then
	# change to directory of target of symlink
	TARGET=`readlink $0`
	cd $(dirname $TARGET)
else
	# change to directory of this script
	cd $(dirname $0)
fi

LD_LIBRARY_PATH=lib bin/tins22
