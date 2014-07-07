#!/bin/sh

DEST_PATH=../../DanmakuFlameMaster/src/main/libs
ANDROID_SOURCE_PATH=AndroidSource
export AOSP_PATH=${PWD}/$ANDROID_SOURCE_PATH/platform

mkdir -p $ANDROID_SOURCE_PATH/platform/external/skia
mkdir -p $ANDROID_SOURCE_PATH/platform/frameworks/base
mkdir -p $ANDROID_SOURCE_PATH/platform/system/core
mkdir -p $ANDROID_SOURCE_PATH/platform/frameworks/native

git clone https://github.com/ctiao/platform-external-skia.git $ANDROID_SOURCE_PATH/platform/external/skia
git clone https://github.com/ctiao/platform-frameworks-base.git $ANDROID_SOURCE_PATH/platform/frameworks/base
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
	git checkout -b $1 $1
	git checkout $1 -f
	git branch
	cd -
	
	echo "\n\nframeworks/base: checkout -b $1..."
	cd $ANDROID_SOURCE_PATH/platform/frameworks/base
	git add .
	git commit -m 'commit'
	git checkout -b $1 $1
	git checkout $1 -f
	git branch
	cd -
	
	echo "\n\nsystem/core: checkout -b $1"
	cd $ANDROID_SOURCE_PATH/platform/system/core
	git add .
	git commit -m 'commit'
	git checkout -b $1 $1
	git checkout $1 -f
	git branch
	cd -
	
	echo "\n\nframeworks/native: checkout -b $1"
	cd $ANDROID_SOURCE_PATH/platform/frameworks/native
	git add .
	git commit -m 'commit'
	git checkout -b $1 $1
	git checkout $1 -f
	git branch
	cd -
	
	echo "====checkout tag ready======"
}

build_so() {
	echo "\n\n-----ndk build libskia.so-------- "
	export API=$1
	ndk-build
}

cp_so() {
	mkdir tmp
	cp libs tmp -rf
}

cp_release_so() {
	rm -rf $DEST_PATH/*/libndkbitmap*.so
	mkdir -p $DEST_PATH/armeabi
	yes | cp tmp/libs/armeabi/libndkbitmap*.so $DEST_PATH/armeabi
	mkdir -p $DEST_PATH/armeabi-v7a
	yes | cp -rf tmp/libs/armeabi/libndkbitmap*.so $DEST_PATH/armeabi-v7a
	mkdir -p $DEST_PATH/x86
	yes | cp -rf tmp/libs/x86/libndkbitmap*.so $DEST_PATH/x86
	mkdir -p $DEST_PATH/mips
	yes | cp -rf tmp/libs/mips/libndkbitmap*.so $DEST_PATH/mips
	
	rm -rf tmp
}

rm -rf tmp

checkout_tags android-sdk-support_r11
build_so 11
cp_so

checkout_tags android-cts-4.0_r1
build_so 14
cp_so

checkout_tags android-4.0.3_r1
build_so 15
cp_so

checkout_tags android-4.1.2_r1
build_so 16
cp_so

checkout_tags android-4.2.2_r1
build_so 17
cp_so

checkout_tags android-4.3_r1
build_so 18
cp_so

checkout_tags android-4.4.2_r1
cp $ANDROID_SOURCE_PATH/platform/external/skia/include/lazy/SkBitmapFactory.h $ANDROID_SOURCE_PATH/platform/external/skia/include/core/SkBitmapFactory.h #file missing
build_so 19
cp_so

cp_release_so

echo "BUILD SUCCESS"
