#!/bin/sh

DEST_PATH=../../DanmakuFlameMaster/src/main/libs
ANDROID_SOURCE_PATH=AndroidSource
export AOSP_PATH=${PWD}/$ANDROID_SOURCE_PATH/platform
ANDROID_LIBS_PATH=AndroidLibs
export AOSP_LIBS_PATH=${PWD}/$ANDROID_LIBS_PATH

mkdir -p $ANDROID_SOURCE_PATH/platform/external/skia
mkdir -p $ANDROID_SOURCE_PATH/platform/frameworks/base
mkdir -p $ANDROID_SOURCE_PATH/platform/system/core
mkdir -p $ANDROID_SOURCE_PATH/platform/frameworks/native

git clone https://github.com/android/platform_frameworks_base.git $ANDROID_SOURCE_PATH/platform/frameworks/base
git clone https://github.com/ctiao/platform-system-core.git $ANDROID_SOURCE_PATH/platform/system/core
git clone https://github.com/ctiao/platform-frameworks-native.git $ANDROID_SOURCE_PATH/platform/frameworks/native
cd $ANDROID_SOURCE_PATH/platform/external/skia
git pull
cd -
cd $ANDROID_SOURCE_PATH/platform/frameworks/base
git pull
cd -
cd $ANDROID_SOURCE_PATH/platform/system/core
git pull
cd -
cd $ANDROID_SOURCE_PATH/platform/frameworks/native
git pull
cd -



checkout_tags() {
	echo "\n\nexternal/skia: checkout -b $1..."
	cd $ANDROID_SOURCE_PATH/platform/external/skia
	git add .
	git commit -m 'commit'
	git checkout $1 -f
	git branch
	cd -
	
	echo "\n\nframeworks/base: checkout -b $1..."
	cd $ANDROID_SOURCE_PATH/platform/frameworks/base
	git add .
	git commit -m 'commit'
	git checkout $1 -f
	git branch
	cd -
	
	echo "\n\nsystem/core: checkout -b $1"
	cd $ANDROID_SOURCE_PATH/platform/system/core
	git add .
	git commit -m 'commit'
	git checkout $1 -f
	git branch
	cd -
	
	echo "\n\nframeworks/native: checkout -b $1"
	cd $ANDROID_SOURCE_PATH/platform/frameworks/native
	git add .
	git commit -m 'commit'
	git checkout $1 -f
	git branch
	cd -
	
	echo "====checkout tag ready======"	
}

build_so() {
	echo "\n\n-----ndk build libndkbitmap.so-------- "
	ndk-build
}

cp_so() {
	mkdir -p tmp
	cp -rf libs tmp
}

cp_release_so() {
	rm -rf $DEST_PATH/*/libndkbitmap*.so
	mkdir -p $DEST_PATH/armeabi
	yes | cp tmp/libs/armeabi/libndkbitmap*.so $DEST_PATH/armeabi
	mkdir -p $DEST_PATH/armeabi-v7a
	yes | cp -rf tmp/libs/armeabi/libndkbitmap*.so $DEST_PATH/armeabi-v7a
	mkdir -p $DEST_PATH/x86
	yes | cp -rf tmp/libs/armeabi/libndkbitmap*.so $DEST_PATH/x86
	mkdir -p $DEST_PATH/mips
	yes | cp -rf tmp/libs/armeabi/libndkbitmap*.so $DEST_PATH/mips
	
	rm -rf tmp
}

rm -rf tmp

checkout_tags android-cts-4.4_r1
build_so
cp_so

cp_release_so

echo "BUILD SUCCESS"
