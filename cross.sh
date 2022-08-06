#!/bin/bash -e

IMAGE=amarillion/alleg5-plus-buildenv:latest-mingw-w64-i686
SRCDIR=$(dirname $(readlink -f $0))
docker run -v $SRCDIR:/data $IMAGE \
 	make TARGET=CROSSCOMPILE WINDOWS=1 BUILD=DEBUG "$@"
docker run -v $SRCDIR:/data $IMAGE \
	./gather-dlls.sh

# go inside cross-compile container
# docker run -v $SRCDIR:/data -ti $IMAGE bash
