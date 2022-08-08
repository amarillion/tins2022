#!/usr/bin/bash -e

# BASE=/mingw32/bin
# ALLEGRO_BASE=/z/prg/allegro5-git/Build/Release/lib
# WINDOWS_BASE=/c/Windows/System32

BASE2=/usr/i686-w64-mingw32/lib
BASE=/usr/i686-w64-mingw32/bin
WINDOWS_BASE=/usr/lib/gcc/i686-w64-mingw32/10-posix

arr=("$BASE" "$BASE2" "$WINDOWS_BASE")

find_dll() {
	find "${arr[@]}" -maxdepth 1 -type f -iname $1 | head -n 1
}

copy_dll() {
	DLL=$(find_dll $1)
	DEST=$2
	if [ -z $DLL ] 
	then
		echo "Could not find $1"
		return
	fi
	cp $DLL $DEST
}

mkdir -p build/release_win
mkdir -p build/debug_win

copy_dll allegro_monolith-5.2.dll "build/release_win"
copy_dll allegro_monolith-debug-5.2.dll "build/debug_win"

for DEST in "build/release_win" "build/debug_win"
do
	copy_dll libgcc_s_dw2-1.dll $DEST
	copy_dll libstdc++-6.dll $DEST
	copy_dll libfreetype-6.dll $DEST
	# copy_dll libbz2-1.dll $DEST
	# copy_dll libharfbuzz-0.dll $DEST
	# copy_dll libglib-2.0-0.dll $DEST
	copy_dll libogg-0.dll $DEST
	copy_dll libpng16-16.dll $DEST
	# copy_dll libjpeg-8.dll $DEST
	copy_dll libFLAC-8.dll $DEST
	copy_dll libphysfs.dll $DEST
	# copy_dll libintl-8.dll $DEST
	# copy_dll libtheoradec-1.dll $DEST
	copy_dll libvorbis-0.dll $DEST
	copy_dll libvorbisfile-3.dll $DEST
	# copy_dll libdumb.dll $DEST
	# copy_dll zlib1.dll $DEST
	copy_dll libwinpthread-1.dll $DEST
	# copy_dll libiconv-2.dll $DEST
	# copy_dll libidn2-0.dll $DEST
	# copy_dll libgraphite2.dll $DEST
	# copy_dll libpcre-1.dll $DEST
	# copy_dll libunistring-2.dll $DEST
	# copy_dll libbrotlidec.dll $DEST
	# copy_dll libbrotlicommon.dll $DEST
	# cp $WINDOWS_BASE/xinput1_3.dll $DEST
	# cp $WINDOWS_BASE/dsound.dll $DEST
	# cp $WINDOWS_BASE/msvcrt.dll $DEST
	copy_dll SSLEAY32.dll $DEST
	copy_dll LIBEAY32.dll $DEST

	echo Checking $DEST/*.exe
	pushd $DEST
	for i in $(objdump -p *.exe *.dll | grep 'DLL Name:' | sort | uniq | sed "s/\s*DLL Name: //")
	do
		# Check if file exists, case insensitive matching...
		if [ -z $(find . -iname "$i") ]
		then
			echo "MISSING: $i"
		else
			echo "FOUND: $i"
		fi
	done
	popd

done
